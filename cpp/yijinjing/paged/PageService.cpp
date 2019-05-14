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

#include "PageService.h"
#include "Journal.h"
#include "Page.h"
#include "Timer.h"
#include "Hash.hpp"
#include "PageUtil.h"
#include "Log.h"

#include <sstream>
#include <mutex>
#include <signal.h>
#include <nlohmann/json.hpp>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

std::mutex paged_mtx;

#define COMM_FILE KUNGFU_JOURNAL_FOLDER + "PAGE_ENGINE_COMM"
const int INTERVAL_IN_MILLISEC = 1000000;

void signal_callback(int signum)
{
    SPDLOG_INFO("PageService Caught signal {}", signum);
    exit(signum);
}

bool PageService::write(string content, byte msg_type, bool is_last, short source)
{
    if (writer.get() == nullptr)
        return false;
    writer->write_frame(content.c_str(), content.length() + 1, source, msg_type, is_last, -1);
    return true;
}

void PageService::acquire_mutex() const
{
    paged_mtx.lock();
}

void PageService::release_mutex() const
{
    paged_mtx.unlock();
}

PageService::PageService(const string& _base_dir) : base_dir(_base_dir),
                                                msg_buffer(nullptr), msg_buffer_idx(0), msg_buffer_idx_limit(0), commFile(COMM_FILE),
                                                microsecFreq(INTERVAL_IN_MILLISEC),
                                                task_running(false), last_switch_nano(0), comm_running(false) {
    KungfuLog::setup_log("paged");
    KungfuLog::set_log_level(spdlog::level::info);

    SPDLOG_INFO("Page engine base dir {}", get_kungfu_home());

    for (int s = 1; s < 32; s++)
        signal(s, signal_callback);
}

PageService::~PageService()
{
    Py_Finalize();
}

void PageService::start()
{
    // setup basic tasks
    tasks.clear();
    add_task(PstBasePtr(new PstPidCheck(this))); // pid check is a necessary task.

    getNanoTime(); // init NanoTimer before ReqClient, avoid deadlock

    SPDLOG_INFO("Loading page buffer {}", commFile);
    msg_buffer = PageUtil::LoadPageBuffer(commFile, COMM_SIZE, true, true);
    memset(msg_buffer, 0, COMM_SIZE);

    SPDLOG_INFO("Creating writer for {}/{}", PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME);
    writer = JournalWriter::create(PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME, "paged", false);
    write("", MSG_TYPE_PAGED_START);

    SPDLOG_INFO("PageService started");

    if (microsecFreq <= 0)
        throw std::runtime_error("unaccepted task time interval");
    task_running = true;
    taskThread = ThreadPtr(new std::thread(boost::bind(&PageService::start_task, this)));
}

void PageService::set_freq(double secondFreq)
{
    microsecFreq = (int)(secondFreq * MICROSECONDS_PER_SECOND);
    SPDLOG_INFO("Microsecond frequency updated to {}", microsecFreq);
}

void PageService::stop()
{
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

    SPDLOG_INFO("PageService stopped");
}

void PageService::start_task()
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

bool PageService::add_task(PstBasePtr task)
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

bool PageService::remove_task(PstBasePtr task)
{
    string name = task->getName();
    acquire_mutex();
    bool ret = remove_task_by_name(name);
    release_mutex();
    return ret;
}

bool PageService::remove_task_by_name(string taskName)
{
    auto task_iter = tasks.find(taskName);
    if (task_iter == tasks.end())
        return false;
    tasks.erase(task_iter);
    SPDLOG_INFO("(rmTask) (name) {}", taskName);
    return true;
}

std::string PageService::reg_journal(const string& clientName)
{
    size_t idx = 0;
    for (; idx < MAX_COMM_USER_NUMBER; idx++)
        if (GET_COMM_MSG(msg_buffer, idx)->status == PAGED_COMM_RAW)
            break;

    if (idx >= MAX_COMM_USER_NUMBER)
    {
        SPDLOG_ERROR("idx {} exceeds limit {}", idx, MAX_COMM_USER_NUMBER);
        return json{
            {"type", PAGED_SOCKET_JOURNAL_REGISTER},
            {"success", false},
            {"error_msg", "idx exceeds limit"},
            {"comm_idx", idx}
        }.dump();
    }
    if (idx > msg_buffer_idx_limit)
        msg_buffer_idx_limit = idx;

    PageCommMsg* msg = GET_COMM_MSG(msg_buffer, idx);
    msg->status = PAGED_COMM_OCCUPIED;
    msg->last_page_num = 0;
    auto it = clientJournals.find(clientName);
    if (it == clientJournals.end())
    {
        SPDLOG_ERROR("cannot find the client in reg_journal");
        return json{
            {"type", PAGED_SOCKET_JOURNAL_REGISTER},
            {"success", false},
            {"error_msg", "cannot find the client"},
            {"comm_idx", idx}
        }.dump();
    }
    it->second.user_index_vec.push_back(idx);
    SPDLOG_INFO("Register journal for {} with id {}", clientName, idx);
    return json{
        {"type", PAGED_SOCKET_JOURNAL_REGISTER},
        {"success", true},
        {"error_msg", ""},
        {"comm_idx", idx}
    }.dump();
}

std::string PageService::reg_client(string& _commFile, int& fileSize, int& hashCode, const string& clientName, int pid, bool isWriter)
{
    SPDLOG_INFO("Register client {} with isWriter {}", clientName, isWriter);
    if (clientJournals.find(clientName) != clientJournals.end())
        return json{
            {"success", false},
            {"error_msg", "client already exists"},
            {"comm_file", this->commFile},
            {"file_size", COMM_SIZE},
            {"hash_code", -1}
        }.dump();

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
    return json{
        {"type", isWriter ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER},
        {"success", true},
        {"error_msg", ""},
        {"comm_file", this->commFile},
        {"file_size", COMM_SIZE},
        {"hash_code", hashCode}
    }.dump();
}

void PageService::release_page(const PageCommMsg& msg)
{
    SPDLOG_INFO("Release page at {}/{}.{}", msg.folder, msg.name, msg.page_num);

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
                SPDLOG_INFO("Release page at {} with address {}", path, addr);
                PageUtil::ReleasePageBuffer(addr, JOURNAL_PAGE_SIZE, true);
                fileAddrs.erase(file_it);
            }
        }
    }
}

byte PageService::initiate_page(const PageCommMsg& msg)
{
    SPDLOG_INFO("Initiate page at {} for {}", msg.folder, msg.name);

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
                        SPDLOG_ERROR("Cannot rename from {} to {}", TEMP_PAGE, path);
                        return PAGED_COMM_CANNOT_RENAME_FROM_TEMP;
                    }
                    else
                    {
                        SPDLOG_INFO("Renamed {} to {}", TEMP_PAGE, path);
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

        SPDLOG_INFO("Added buffer {} to {}", buffer, path);
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

std::string  PageService::exit_client(const string& clientName, int hashCode, bool needHashCheck)
{
    map<string, PageClientInfo>::iterator it = clientJournals.find(clientName);
    if (it == clientJournals.end())
        return json{
            {"type", PAGED_SOCKET_CLIENT_EXIT},
            {"success", false},
            {"error_msg", "client does not exist"}
        }.dump();
    PageClientInfo& info = it->second;
    if (needHashCheck && hashCode != info.hash_code)
    {
        SPDLOG_ERROR("Failed to get hash for {}, server {} client {}", clientName, info.hash_code, hashCode);
        return json{
            {"type", PAGED_SOCKET_CLIENT_EXIT},
            {"success", false},
            {"error_msg", "hash code validation failed"}
        }.dump();
    }
    if (info.is_strategy)
    {
        int idx = info.user_index_vec[0]; // strategy must be a writer, therefore only one user
        PageCommMsg* msg = GET_COMM_MSG(msg_buffer, idx);
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
        PageCommMsg* msg = GET_COMM_MSG(msg_buffer, idx);
        if (msg->status == PAGED_COMM_ALLOCATED)
            release_page(*msg);
        msg->status = PAGED_COMM_RAW;
    }
    SPDLOG_INFO("Client {} exited, used from {} to {}", clientName, info.reg_nano, getNanoTime());
    vector<string>& clients = pidClient[info.pid];
    clients.erase(remove(clients.begin(), clients.end(), clientName), clients.end());
    if (clients.size() == 0)
        pidClient.erase(info.pid);
    clientJournals.erase(it);

    return json{
        {"type", PAGED_SOCKET_CLIENT_EXIT},
        {"success", true},
        {"error_msg", ""}
    }.dump();
}

void PageService::process_one_message()
{
    PageCommMsg* msg = GET_COMM_MSG(msg_buffer, msg_buffer_idx);
    if (msg->status == PAGED_COMM_REQUESTING)
    {
        acquire_mutex();
        SPDLOG_INFO("Request page for id {}", msg_buffer_idx);
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
    msg_buffer_idx = (msg_buffer_idx + 1) % (msg_buffer_idx_limit + 1);
}

py::dict PageService::getStatus() const
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

py::dict PageService::getClientInfo() const
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

py::dict PageService::getUserInfo() const
{
    py::dict info;
    for (size_t idx = 0; idx < msg_buffer_idx_limit; idx++)
    {
        PageCommMsg* msg = GET_COMM_MSG(msg_buffer, idx);
        if (msg->status == PAGED_COMM_ALLOCATED)
        {
            info[py::cast(idx)] = py::make_tuple(msg->folder, msg->name, py::cast(msg->page_num));
        }
    }
    return info;
}

py::tuple PageService::getTaskInfo() const
{
    py::dict info;
    for (auto const &item: tasks)
    {
        info[item.first.c_str()] = item.second->getInfo();
    }
    return py::make_tuple(((task_running)? "running": "stopped"), microsecFreq, info);
}

py::dict PageService::getPidInfo() const
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

py::dict PageService::getFileReaderInfo() const
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

py::dict PageService::getFileWriterInfo() const
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

py::list PageService::getLockingFiles() const
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
    py::class_<PageService, boost::shared_ptr<PageService> >(m, "PageService")
    .def(py::init<const std::string&>())
    .def("process_one_message", &PageService::process_one_message)
    .def("start", &PageService::start)
    .def("stop", &PageService::stop)
    .def("setFreq", &PageService::set_freq, py::arg("seconds")=0.1)
    .def("status", &PageService::getStatus)
    .def("write", &PageService::write, py::arg("content"), py::arg("msg_type"), py::arg("is_last")=true, py::arg("source")=0)
    .def("reg_journal", &PageService::reg_journal, py::arg("clientName"))
    .def("reg_client", &PageService::reg_client, py::arg("commFile"), py::arg("fileSize"), py::arg("hashCode"), py::arg("clientName"), py::arg("pid"), py::arg("isWriter"))
    .def("exit_client", &PageService::exit_client, py::arg("clientName"), py::arg("hashCode"), py::arg("needHashCheck"))
    ;

    m.def("jfolder", &getJournalFolder);
    m.def("jname", &getJournalName);
}
