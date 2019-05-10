/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by cjiang on 17/3/24.
//

#include "nlohmann/json.hpp"

#include "PageEngine.h"
#include "Journal.h"
#include "Page.h"
#include "Timer.h"
#include "Hash.hpp"
#include "PageUtil.h"
#include "Log.h"

#include <sstream>
#include <mutex>
#include <signal.h>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

std::mutex paged_mtx;

#define COMM_FILE KUNGFU_JOURNAL_FOLDER + "PAGE_ENGINE_COMM"
const int INTERVAL_IN_MILLISEC = 1000000;

void signal_callback(int signum)
{
    SPDLOG_INFO("PageEngine Caught signal {}", signum);
    exit(signum);
}

bool PageEngine::write(string content, byte msg_type, bool is_last, short source)
{
    if (writer.get() == nullptr)
        return false;
    writer->write_frame(content.c_str(), content.length() + 1, source, msg_type, is_last, -1);
    return true;
}

void PageEngine::acquire_mutex() const
{
    paged_mtx.lock();
}

void PageEngine::release_mutex() const
{
    paged_mtx.unlock();
}

PageEngine::PageEngine(const string& _base_dir) : base_dir(_base_dir), commBuffer(nullptr), commFile(COMM_FILE), maxIdx(0),
                                          microsecFreq(INTERVAL_IN_MILLISEC),
                                          task_running(false), last_switch_nano(0), comm_running(false) {
    for (int s = 1; s < 32; s++)
        signal(s, signal_callback);

    KungfuLog::setup_log("paged");
    KungfuLog::set_log_level(spdlog::level::info);

    // setup basic tasks
    tasks.clear();
    add_task(PstBasePtr(new PstPidCheck(this))); // pid check is a necessary task.

    SPDLOG_INFO("page engine base dir {}", get_kungfu_home());
}

PageEngine::~PageEngine()
{
    Py_Finalize();
}

void PageEngine::start()
{
    getNanoTime(); // init NanoTimer before ReqClient, avoid deadlock
    SPDLOG_INFO("reset socket: {}", PAGED_SOCKET_FILE);
    remove(string(PAGED_SOCKET_FILE).c_str());
    // step 0: init commBuffer
    SPDLOG_INFO("loading page buffer: {}", commFile);
    commBuffer = PageUtil::LoadPageBuffer(commFile, COMM_SIZE, true, true);
    memset(commBuffer, 0, COMM_SIZE);
    // step 1: start commBuffer checking thread
    comm_running = false;
    commThread = ThreadPtr(new std::thread(boost::bind(&PageEngine::start_comm, this)));
    // step 2: start socket listening
    socketThread = ThreadPtr(new std::thread(boost::bind(&PageEngine::start_socket, this)));
    // make sure buffer / socket are running
    while (!(PageSocketHandler::getInstance()->is_running() && comm_running))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(INTERVAL_IN_MILLISEC / 10));
    }
    SPDLOG_INFO("creating writer: {} {}", PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME);
    writer = JournalWriter::create(PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME, "paged");

    if (microsecFreq <= 0)
        throw std::runtime_error("unaccepted task time interval");
    task_running = true;
    taskThread = ThreadPtr(new std::thread(boost::bind(&PageEngine::start_task, this)));
    write("", MSG_TYPE_PAGED_START);
}

void PageEngine::set_freq(double secondFreq)
{
    microsecFreq = (int)(secondFreq * MICROSECONDS_PER_SECOND);
    SPDLOG_INFO("microsecond frequency updated to {}", microsecFreq);
}

void PageEngine::stop()
{
    SPDLOG_INFO("(stop) try...");

    /* write paged end in system journal */
    write("", MSG_TYPE_PAGED_END);
    writer.reset();

    /* stop task thread first */
    task_running = false;
    if (taskThread.get() != nullptr)
    {
        taskThread->join();
        taskThread.reset();
    }
    SPDLOG_INFO("(stopTask) done");

    /* stop comm thread */
    comm_running = false;
    if (commThread.get() != nullptr)
    {
        commThread->join();
        commThread.reset();
    }
    SPDLOG_INFO("(stopComm) done");

    /* stop socket io thread */
    PageSocketHandler::getInstance()->stop();
    if (socketThread.get() != nullptr)
    {
        socketThread->join();
        socketThread.reset();
    }
    SPDLOG_INFO("(stopSocket) done");

    /* finish up */
    SPDLOG_INFO("(stop) done");
}

void PageEngine::start_task()
{
    SPDLOG_INFO("(startTasks) (microseconds) {}", microsecFreq);
    while (task_running)
    {
        acquire_mutex();
        for (auto item: tasks)
        {
            item.second->go();
        }
        release_mutex();
        std::this_thread::sleep_for(std::chrono::microseconds(microsecFreq));
    }
}

bool PageEngine::add_task(PstBasePtr task)
{
    acquire_mutex();
    string name = task->getName();
    bool exist = (tasks.find(name) != tasks.end());
    tasks[name] = task;
    if (exist)
    {
        SPDLOG_INFO("task {} replaced", name);
    } else
    {
        SPDLOG_INFO("task {} added", name);
    }
    release_mutex();
    return !exist;
}

bool PageEngine::remove_task(PstBasePtr task)
{
    string name = task->getName();
    acquire_mutex();
    bool ret = remove_task_by_name(name);
    release_mutex();
    return ret;
}

bool PageEngine::remove_task_by_name(string taskName)
{
    auto task_iter = tasks.find(taskName);
    if (task_iter == tasks.end())
        return false;
    tasks.erase(task_iter);
    SPDLOG_INFO("(rmTask) (name) {}", taskName);
    return true;
}

void PageEngine::start_socket()
{
    PageSocketHandler::getInstance()->run(this);
}

int PageEngine::reg_journal(const string& clientName)
{
    size_t idx = 0;
    for (; idx < MAX_COMM_USER_NUMBER; idx++)
        if (GET_COMM_MSG(commBuffer, idx)->status == PAGED_COMM_RAW)
            break;

    if (idx >= MAX_COMM_USER_NUMBER)
    {
        SPDLOG_ERROR("idx {} exceeds limit {}", idx, MAX_COMM_USER_NUMBER);
        return -1;
    }
    if (idx > maxIdx)
        maxIdx = idx;

    PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
    msg->status = PAGED_COMM_OCCUPIED;
    msg->last_page_num = 0;
    auto it = clientJournals.find(clientName);
    if (it == clientJournals.end())
    {
        SPDLOG_ERROR("cannot find the client in reg_journal");
        return -1;
    }
    it->second.user_index_vec.push_back(idx);
    SPDLOG_INFO("[RegJournal] (client) {} (idx) {}", clientName, idx);
    return idx;
}

bool PageEngine::reg_client(string& _commFile, int& fileSize, int& hashCode, const string& clientName, int pid, bool isWriter)
{
    SPDLOG_INFO("[RegClient] (name) {} (writer?) {}", clientName, isWriter);
    if (clientJournals.find(clientName) != clientJournals.end())
        return false;

    map<int, vector<string> >::iterator it = pidClient.find(pid);
    if (it == pidClient.end())
        pidClient[pid] = {clientName};
    else
        pidClient[pid].push_back(clientName);

    std::stringstream ss;
    ss << clientName << getNanoTime() << pid;
    hashCode = MurmurHash2(ss.str().c_str(), ss.str().length(), HASH_SEED);
    PageClientInfo& clientInfo = clientJournals[clientName];
    clientInfo.user_index_vec.clear();
    clientInfo.reg_nano = getNanoTime();
    clientInfo.is_writing = isWriter;
    clientInfo.is_strategy = false;
    clientInfo.rid_start = -1;
    clientInfo.rid_end = -1;
    clientInfo.pid = pid;
    clientInfo.hash_code = hashCode;
    _commFile = this->commFile;
    fileSize = COMM_SIZE;
    return true;
}

void PageEngine::release_page(const PageCommMsg& msg)
{
    SPDLOG_INFO("[RmPage] (folder) {} (jname) {} (pNum) {} (lpNum) {}", msg.folder, msg.name, msg.page_num, msg.last_page_num);

    map<PageCommMsg, int>::iterator count_it;
    if (msg.is_writer)
    {
        count_it = fileWriterCounts.find(msg);
        if (count_it == fileWriterCounts.end())
        {
            SPDLOG_ERROR("cannot find key at fileWriterCounts in exit_client");
            return;
        }
    }
    else
    {
        count_it = fileReaderCounts.find(msg);
        if (count_it == fileReaderCounts.end())
        {
            SPDLOG_ERROR("cannot find key at fileReaderCounts in exit_client");
            return;
        }
    }
    count_it->second --;
    if (count_it->second == 0)
    {
        bool otherSideEmpty = false;
        if (msg.is_writer)
        {
            fileWriterCounts.erase(count_it);
            otherSideEmpty = fileReaderCounts.find(msg) == fileReaderCounts.end();
        }
        else
        {
            fileReaderCounts.erase(count_it);
            otherSideEmpty = fileWriterCounts.find(msg) == fileWriterCounts.end();
        }
        if (otherSideEmpty)
        {
            string path = PageUtil::GenPageFullPath(msg.folder, msg.name, msg.page_num);
            auto file_it = fileAddrs.find(path);
            if (file_it != fileAddrs.end())
            {
                void* addr = file_it->second;
                SPDLOG_INFO("[AddrRm] (path) {} (addr) {}", path, addr);
                PageUtil::ReleasePageBuffer(addr, JOURNAL_PAGE_SIZE, true);
                fileAddrs.erase(file_it);
            }
        }
    }
}

byte PageEngine::initiate_page(const PageCommMsg& msg)
{
    SPDLOG_INFO("[InPage] (folder) {} (jname) {} (pNum) {} (lpNum) {} (writer?) {}", msg.folder, msg.name, msg.page_num, msg.last_page_num, msg.is_writer);

    string path = PageUtil::GenPageFullPath(msg.folder, msg.name, msg.page_num);
    if (fileAddrs.find(path) == fileAddrs.end())
    {
        void* buffer = nullptr;
        if (!PageUtil::FileExists(path))
        {   // this file is not exist....
            if (!msg.is_writer)
                return PAGED_COMM_NON_EXIST;
            else
            {
                auto tempPageIter = fileAddrs.find(TEMP_PAGE);
                if (tempPageIter != fileAddrs.end())
                {
                    int ret = rename((TEMP_PAGE).c_str(), path.c_str());
                    if (ret < 0)
                    {
                        SPDLOG_ERROR("[InPage] ERROR: Cannot rename from {} to {}", TEMP_PAGE, path);
                        return PAGED_COMM_CANNOT_RENAME_FROM_TEMP;
                    }
                    else
                    {
                        SPDLOG_INFO("[InPage] TEMP_POOL: {} to {}", TEMP_PAGE, path);
                        buffer = tempPageIter->second;
                        fileAddrs.erase(tempPageIter);
                    }
                }
                if (buffer == nullptr)
                    buffer = PageUtil::LoadPageBuffer(path, JOURNAL_PAGE_SIZE, true, true);
            }
        }
        else
        {   // exist file but not loaded, map and lock immediately.
            buffer = PageUtil::LoadPageBuffer(path, JOURNAL_PAGE_SIZE, false, true);
        }

        SPDLOG_INFO("[AddrAdd] (path) {} (addr) {}", path, buffer);
        fileAddrs[path] = buffer;
    }

    if (msg.is_writer)
    {
        auto count_it = fileWriterCounts.find(msg);
        if (count_it == fileWriterCounts.end())
            fileWriterCounts[msg] = 1;
        else
            return PAGED_COMM_MORE_THAN_ONE_WRITE;
    }
    else
    {
        auto count_it = fileReaderCounts.find(msg);
        if (count_it == fileReaderCounts.end())
            fileReaderCounts[msg] = 1;
        else
            count_it->second ++;
    }
    return PAGED_COMM_ALLOCATED;
}

bool PageEngine::login_td(const string& clientName, short source)
{
    SPDLOG_INFO("[TELogin] (name) {} (source) {}", clientName, source);

    map<string, PageClientInfo>::iterator it = clientJournals.find(clientName);
    if (it == clientJournals.end())
    {
        SPDLOG_ERROR("[ERROR][TELogin] client {} does not exist!", clientName);
        return false;
    }
    PageClientInfo& info = it->second;
    if (info.user_index_vec.size() != 1)
    {
        SPDLOG_ERROR("[ERROR][TELogin] this client suppose to have only one journal! (name) {}", clientName);
        return false;
    }
    PageCommMsg* msg = GET_COMM_MSG(commBuffer, info.user_index_vec[0]);
    json j_request;
    j_request["name"] = clientName;
    j_request["folder"] = msg->folder;
    j_request["rid_s"] = info.rid_start;
    j_request["rid_e"] = info.rid_end;
    j_request["pid"] = info.pid;
    j_request["last_switch_nano"] = last_switch_nano;
    write(j_request.dump(), MSG_TYPE_TRADE_ENGINE_LOGIN, true, source);
    info.trade_engine_vec.push_back(source);
    return true;
}

void  PageEngine::exit_client(const string& clientName, int hashCode, bool needHashCheck)
{
    map<string, PageClientInfo>::iterator it = clientJournals.find(clientName);
    if (it == clientJournals.end())
        return;
    PageClientInfo& info = it->second;
    if (needHashCheck && hashCode != info.hash_code)
    {
        SPDLOG_ERROR("[RmClient] HASH FAILED.. (name) {} (serverHash) {} (clientHash) {}", clientName, info.hash_code, hashCode);
        return;
    }
    if (info.is_strategy)
    {
        int idx = info.user_index_vec[0]; // strategy must be a writer, therefore only one user
        PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
        json j_request;
        j_request["name"] = clientName;
        j_request["folder"] = msg->folder;
        j_request["rid_s"] = info.rid_start;
        j_request["rid_e"] = info.rid_end;
        j_request["pid"] = info.pid;
        j_request["last_switch_nano"] = last_switch_nano;
        write(j_request.dump(), MSG_TYPE_STRATEGY_END);
    }

    for (auto idx: info.user_index_vec)
    {
        PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
        if (msg->status == PAGED_COMM_ALLOCATED)
            release_page(*msg);
        msg->status = PAGED_COMM_RAW;
    }
    SPDLOG_INFO("[RmClient] (name) {} (start) {} (end) {}", clientName, info.reg_nano, getNanoTime());
    vector<string>& clients = pidClient[info.pid];
    clients.erase(remove(clients.begin(), clients.end(), clientName), clients.end());
    if (clients.size() == 0)
        pidClient.erase(info.pid);
    clientJournals.erase(it);
}

IntPair PageEngine::register_strategy(const string& strategyName)
{
    map<string, PageClientInfo>::iterator it = clientJournals.find(strategyName);
    if (it == clientJournals.end())
    {
        SPDLOG_ERROR("[ERROR] cannot find client {} ", strategyName);
        return std::make_pair(-1, -1);
    }

    PageClientInfo& info = it->second;
    int idx = info.user_index_vec[0]; // strategy must be a writer, therefore only one user
    info.is_strategy = true;
    // make sure this rid start from REQUEST_ID_RANGE,
    // [0, REQUEST_ID_RANGE - 1] belongs to trade engine themselves.
    info.rid_start = (idx + 1) * REQUEST_ID_RANGE;
    info.rid_end = (idx + 2) * REQUEST_ID_RANGE - 1;
    PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
    json j_request;
    j_request["name"] = strategyName;
    j_request["folder"] = msg->folder;
    j_request["rid_s"] = info.rid_start;
    j_request["rid_e"] = info.rid_end;
    j_request["pid"] = info.pid;
    j_request["last_switch_nano"] = last_switch_nano;
    write(j_request.dump(), MSG_TYPE_STRATEGY_START);
    SPDLOG_INFO("[RegStrategy] (name) {} (rid) {} - {}", strategyName, info.rid_start, info.rid_end);
    return std::make_pair(info.rid_start, info.rid_end);
};

bool PageEngine::sub_md(const vector<string>& tickers, short source, short msg_type, bool isLast)
{
    SPDLOG_INFO("(subscribe) (source) {} (msg) {} (num) {} (last) {}", source, msg_type, tickers.size(), isLast);
    bool written = true;
    for (size_t i = 0; i < tickers.size(); i++)
        written &= write(tickers[i], msg_type, isLast && (i == tickers.size() - 1), source);
    return written;
}

void PageEngine::start_comm()
{
    comm_running = true;
    for (size_t idx = 0; comm_running; idx = (idx + 1) % (maxIdx + 1))
    {
        PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
        if (msg->status == PAGED_COMM_REQUESTING)
        {
            acquire_mutex();
            SPDLOG_INFO("[Demand] (idx) {}", idx);
            if (msg->last_page_num > 0 && msg->last_page_num != msg->page_num)
            {
                short curPage = msg->page_num;
                msg->page_num = msg->last_page_num;
                release_page(*msg);
                msg->page_num = curPage;
            }
            msg->status = initiate_page(*msg);
            msg->last_page_num = msg->page_num;
            release_mutex();
        }
    }
}

bool PageEngine::switch_trading_day()
{
    return write("", MSG_TYPE_SWITCH_TRADING_DAY);
}

py::dict PageEngine::getStatus() const
{
    acquire_mutex();
    py::dict res;
    res["Client"] = getClientInfo();
    res["Pid"] = getPidInfo();
    res["Task"] = getTaskInfo();
    res["User"] = getUserInfo();
    res["File"] = py::dict();
    res["File"]["Read"] = getFileReaderInfo();
    res["File"]["Write"] = getFileWriterInfo();
    res["File"]["Locking"] = getLockingFiles();
    release_mutex();
    return res;
}

py::dict PageEngine::getClientInfo() const
{
    py::dict info;
    for (auto const &item: clientJournals)
    {
        const string& key = item.first;
        const char* c = key.c_str();
        info[c] = py::dict();
        py::list uIndice;
        py::list tds;
        const PageClientInfo& cInfo = item.second;
        for (auto idx: cInfo.user_index_vec)
            uIndice.append(idx);
        for (auto idx: cInfo.trade_engine_vec)
            tds.append(idx);
        info[c]["users"] = uIndice;
        info[c]["trade_engine_vec"] = tds;
        info[c]["registerTime"] = cInfo.reg_nano;
        info[c]["is_writing"] = cInfo.is_writing;
        info[c]["is_strategy"] = cInfo.is_strategy;
        info[c]["rid_start"] = cInfo.rid_start;
        info[c]["pid"] = cInfo.pid;
        info[c]["hash_code"] = cInfo.hash_code;
    }
    return info;
}

py::dict PageEngine::getUserInfo() const
{
    py::dict info;
    for (size_t idx = 0; idx < maxIdx; idx++)
    {
        PageCommMsg* msg = GET_COMM_MSG(commBuffer, idx);
        if (msg->status == PAGED_COMM_ALLOCATED)
        {
            info[py::cast(idx)] = py::make_tuple(msg->folder, msg->name, py::cast(msg->page_num));
        }
    }
    return info;
}

py::tuple PageEngine::getTaskInfo() const
{
    py::dict info;
    for (auto const &item: tasks)
    {
        info[item.first.c_str()] = item.second->getInfo();
    }
    return py::make_tuple(((task_running)? "running": "stopped"), microsecFreq, info);
}

py::dict PageEngine::getPidInfo() const
{
    py::dict info;
    for (auto const &item: pidClient)
    {
        py::list names;
        for (auto const name: item.second)
            names.append(name);
        info[py::cast(item.first)] = names;
    }
    return info;
}

py::dict PageEngine::getFileReaderInfo() const
{
    py::dict info;
    for (auto const &item: fileReaderCounts)
    {
        const PageCommMsg& msg = item.first;
        py::tuple key = py::make_tuple(msg.folder, msg.name, msg.page_num, msg.last_page_num, msg.is_writer);
        info[key] = item.second;
    }
    return info;
}

py::dict PageEngine::getFileWriterInfo() const
{
    py::dict info;
    for (auto const &item: fileWriterCounts)
    {
        const PageCommMsg& msg = item.first;
        py::tuple key = py::make_tuple(msg.folder, msg.name, msg.page_num, msg.last_page_num, msg.is_writer);
        info[key] = item.second;
    }
    return info;
}

py::list PageEngine::getLockingFiles() const
{
    py::list files;
    for (auto const &iter: fileAddrs)
        files.append(iter.first);
    return files;
}

string getJournalFolder()
{
    return PAGED_JOURNAL_FOLDER;
}

string getJournalName()
{
    return PAGED_JOURNAL_NAME;
}

PYBIND11_MODULE(paged, m)
{
    py::class_<PageEngine, boost::shared_ptr<PageEngine> >(m, "PageEngine")
    .def(py::init<const std::string&>())
    .def("start", &PageEngine::start)
    .def("stop", &PageEngine::stop)
    .def("setFreq", &PageEngine::set_freq, py::arg("seconds")=0.1)
    .def("status", &PageEngine::getStatus)
    .def("write", &PageEngine::write, py::arg("content"), py::arg("msg_type"), py::arg("is_last")=true, py::arg("source")=0);

    m.def("jfolder", &getJournalFolder);
    m.def("jname", &getJournalName);
}
