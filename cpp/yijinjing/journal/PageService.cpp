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

#include "PageService.h"
#include "Journal.h"
#include "Page.h"
#include "Timer.h"
#include "Hash.hpp"
#include "PageUtil.h"
#include "Log.h"

#include <sstream>
#include <signal.h>
#include <nlohmann/json.hpp>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

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

PageService::PageService(const string& _base_dir) : base_dir(_base_dir), memory_message_buffer(nullptr), memory_message_limit(0), memory_msg_file(MEMORY_MSG_FILE) {
    KungfuLog::setup_log("paged");
    KungfuLog::set_log_level(spdlog::level::info);

    SPDLOG_INFO("Page engine base dir {}", get_kungfu_home());

    for (int s = 1; s < 32; s++)
        signal(s, signal_callback);
}

void PageService::start()
{
    getNanoTime(); // init NanoTimer before ReqClient, avoid deadlock

    SPDLOG_INFO("Loading page buffer {}", memory_msg_file);
    memory_message_buffer = PageUtil::LoadPageBuffer(memory_msg_file, MEMORY_MSG_FILE_SIZE, true, true);
    memset(memory_message_buffer, 0, MEMORY_MSG_FILE_SIZE);

    SPDLOG_INFO("Creating writer for {}/{}", PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME);
    writer = JournalWriter::create(PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME, "paged", false);
    write("", MSG_TYPE_PAGED_START);

    SPDLOG_INFO("PageService started");
}

void PageService::stop()
{
    /* write paged end in system journal */
    write("", MSG_TYPE_PAGED_END);
    writer.reset();
    SPDLOG_INFO("PageService stopped");
}

void PageService::process_memory_message()
{
    for (int idx = 0; idx < memory_message_limit; idx++)
    {
        PageServiceMsg* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
        if (msg->status == PAGED_COMM_REQUESTING)
        {
            SPDLOG_INFO("Request page for id {}/{}", idx, memory_message_limit);
            if (msg->last_page_num > 0 && msg->last_page_num != msg->page_num)
            {
                short curPage = msg->page_num;
                msg->page_num = msg->last_page_num;
                release_page(*msg);
                msg->page_num = curPage;
            }
            msg->status = initiate_page(*msg);
            msg->last_page_num = msg->page_num;
        }
    }
}

std::string PageService::register_journal(const string& clientName)
{
    int idx = 0;
    for (; idx < MAX_COMM_USER_NUMBER; idx++)
        if (GET_MEMORY_MSG(memory_message_buffer, idx)->status == PAGED_COMM_RAW)
            break;

    if (idx >= MAX_COMM_USER_NUMBER)
    {
        SPDLOG_ERROR("idx {} exceeds limit {}", idx, MAX_COMM_USER_NUMBER);
        return json{
            {"type", PAGED_SOCKET_JOURNAL_REGISTER},
            {"success", false},
            {"error_msg", "idx exceeds limit"},
            {"memory_msg_idx", idx}
        }.dump();
    }
    if (idx >= memory_message_limit)
    {
        memory_message_limit = idx + 1;
    }

    PageServiceMsg* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
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
            {"memory_msg_idx", idx}
        }.dump();
    }
    it->second.user_index_vec.push_back(idx);
    SPDLOG_INFO("Register journal for {} with id {}", clientName, idx);
    return json{
        {"type", PAGED_SOCKET_JOURNAL_REGISTER},
        {"success", true},
        {"error_msg", ""},
        {"memory_msg_idx", idx}
    }.dump();
}

std::string PageService::register_client(const string& clientName, int pid, bool isWriter)
{
    SPDLOG_INFO("Register client {} with isWriter {}", clientName, isWriter);
    if (clientJournals.find(clientName) != clientJournals.end())
        return json{
            {"success", false},
            {"error_msg", "client already exists"},
            {"memory_msg_file", this->memory_msg_file},
            {"file_size", MEMORY_MSG_FILE_SIZE},
            {"hash_code", -1}
        }.dump();

    map<int, vector<string> >::iterator it = pidClient.find(pid);
    if (it == pidClient.end())
        pidClient[pid] = {clientName};
    else
        pidClient[pid].push_back(clientName);

    std::stringstream ss;
    ss << clientName << getNanoTime() << pid;
    int hashCode = MurmurHash2(ss.str().c_str(), ss.str().length(), HASH_SEED);
    PageClientInfo& clientInfo = clientJournals[clientName];
    clientInfo.user_index_vec.clear();
    clientInfo.reg_nano = getNanoTime();
    clientInfo.is_writing = isWriter;
    clientInfo.is_strategy = false;
    clientInfo.rid_start = -1;
    clientInfo.rid_end = -1;
    clientInfo.pid = pid;
    clientInfo.hash_code = hashCode;
    return json{
        {"type", isWriter ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER},
        {"success", true},
        {"error_msg", ""},
        {"memory_msg_file", this->memory_msg_file},
        {"file_size", MEMORY_MSG_FILE_SIZE},
        {"hash_code", hashCode}
    }.dump();
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
        PageServiceMsg* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
        json j_request;
        j_request["name"] = clientName;
        j_request["folder"] = msg->folder;
        j_request["rid_s"] = info.rid_start;
        j_request["rid_e"] = info.rid_end;
        j_request["pid"] = info.pid;
        write(j_request.dump(), MSG_TYPE_STRATEGY_END);
    }

    for (auto idx: info.user_index_vec)
    {
        PageServiceMsg* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
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

byte PageService::initiate_page(const PageServiceMsg& msg)
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

void PageService::release_page(const PageServiceMsg& msg)
{
    SPDLOG_INFO("Release page in {} for {}", msg.folder, msg.name);

    map<PageServiceMsg, int>::iterator count_it;
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
