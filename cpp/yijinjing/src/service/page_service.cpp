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

#include <sstream>
#include <signal.h>
#include <nlohmann/json.hpp>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/service/page_service.h>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

void page_service_signal_callback(int signum)
{
    switch(signum){
#ifdef _WINDOWS
        case  SIGINT:          // interrupt
        case  SIGBREAK:        // Ctrl-Break sequence
            SPDLOG_INFO("KungFu server interrupted");
            exit(signum);
            break;
        case  SIGTERM:         // Software termination signal from kill
            SPDLOG_INFO("KungFu server killed");
            exit(signum);
            break;
        case  SIGILL:          // illegal instruction - invalid function image
        case  SIGFPE:          // floating point exception
        case  SIGSEGV:         // segment violation
        case  SIGABRT:         // abnormal termination triggered by abort call
        case  SIGABRT_COMPAT:  // SIGABRT compatible with other platforms, same as SIGABRT
            SPDLOG_CRITICAL("KungFu server stopped by signal {}", signum);
            exit(signum);
            break;
#else
        case SIGURG:       // discard signal       urgent condition present on socket
        case SIGCONT:      // discard signal       continue after stop
        case SIGCHLD:      // discard signal       child status has changed
        case SIGIO:        // discard signal       I/O is possible on a descriptor (see fcntl(2))
        case SIGWINCH:     // discard signal       Window size change
        case SIGINFO:      // discard signal       status request from keyboard
            SPDLOG_INFO("KungFu server discard signal {}", signum);
            break;
        case SIGSTOP:      // stop process         stop (cannot be caught or ignored)
        case SIGTSTP:      // stop process         stop signal generated from keyboard
        case SIGTTIN:      // stop process         background read attempted from control terminal
        case SIGTTOU:      // stop process         background write attempted to control terminal
            SPDLOG_CRITICAL("KungFu server stopped by signal {}", signum);
            exit(signum);
            break;
        case SIGINT:       // terminate process    interrupt program
            SPDLOG_INFO("KungFu server interrupted");
            exit(signum);
            break;
        case SIGKILL:      // terminate process    kill program
        case SIGTERM:      // terminate process    software termination signal
            SPDLOG_INFO("KungFu server killed");
            exit(signum);
            break;
        case SIGHUP:       // terminate process    terminal line hangup
        case SIGPIPE:      // terminate process    write on a pipe with no reader
        case SIGALRM:      // terminate process    real-time timer expired
        case SIGXCPU:      // terminate process    cpu time limit exceeded (see setrlimit(2))
        case SIGXFSZ:      // terminate process    file size limit exceeded (see setrlimit(2))
        case SIGVTALRM:    // terminate process    virtual time alarm (see setitimer(2))
        case SIGPROF:      // terminate process    profiling timer alarm (see setitimer(2))
            SPDLOG_CRITICAL("KungFu server terminated by signal {}", signum);
            exit(signum);
            break;
        case SIGUSR1:      // terminate process    User defined signal 1
        case SIGUSR2:      // terminate process    User defined signal 2
            SPDLOG_CRITICAL("KungFu server caught user defined signal {}", signum);
            exit(signum);
            break;
        case SIGQUIT:      // create core image    quit program
        case SIGILL:       // create core image    illegal instruction
        case SIGTRAP:      // create core image    trace trap
        case SIGABRT:      // create core image    abort program (formerly SIGIOT)
        case SIGEMT:       // create core image    emulate instruction executed
        case SIGFPE:       // create core image    floating-point exception
        case SIGBUS:       // create core image    bus error
            SPDLOG_CRITICAL("bus error");
        case SIGSEGV:      // create core image    segmentation violation
            SPDLOG_CRITICAL("segmentation violation");
        case SIGSYS:       // create core image    non-existent system call invoked
            SPDLOG_CRITICAL("KungFu server caught unexpected signal {}", signum);
            exit(signum);
            break;
#endif // _WINDOWS
        default:
            SPDLOG_INFO("KungFu server caught unknown signal {}, signal ignored", signum);
    }
}

bool PageService::write(string content, byte msg_type, bool is_last, short source)
{
    if (writer.get() == nullptr)
        return false;
    writer->write_frame(content.c_str(), content.length() + 1, source, msg_type, is_last, -1);
    return true;
}

PageService::PageService(const string& _base_dir) : base_dir(_base_dir), memory_message_buffer(nullptr), memory_message_limit(0), memory_msg_file(MEMORY_MSG_FILE) {
    kungfu::log::LogConfig::setup_log("paged");

    SPDLOG_INFO("Page engine base dir {}", get_kungfu_home());

    for (int s = 1; s < NSIG; s++)
    {
        signal(s, page_service_signal_callback);
    }
}

string PageService::get_memory_msg_file()
{
    return memory_msg_file;
}

size_t PageService::get_memory_msg_file_size()
{
    return MEMORY_MSG_FILE_SIZE;
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
    for (int32_t idx = 0; idx < memory_message_limit; idx++)
    {
        PageServiceMessage* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
        if (msg->status == PAGE_REQUESTING)
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

int32_t PageService::register_journal(const string& client_name)
{
    int32_t idx = 0;
    for (; idx < MAX_MEMORY_MSG_NUMBER; idx++)
        if (GET_MEMORY_MSG(memory_message_buffer, idx)->status == PAGE_RAW)
            break;
    SPDLOG_INFO("Register journal for {} with id {}", client_name, idx);

    if (idx >= MAX_MEMORY_MSG_NUMBER)
    {
        SPDLOG_ERROR("idx {} exceeds limit {}", idx, MAX_MEMORY_MSG_NUMBER);
        return idx;
    }
    if (idx >= memory_message_limit)
    {
        memory_message_limit = idx + 1;
    }

    PageServiceMessage* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
    msg->status = PAGE_OCCUPIED;
    msg->last_page_num = 0;
    return idx;
}

uint32_t PageService::register_client(const string& client_name, int pid, bool is_writer)
{
    SPDLOG_INFO("Register client {} for pid {} is_writer {}", client_name, pid, is_writer);
    std::stringstream ss;
    ss << client_name << getNanoTime() << pid;
    uint32_t hashCode = MurmurHash2(ss.str().c_str(), ss.str().length(), HASH_SEED);
    return hashCode;
}

void PageService::release_page_at(int idx)
{
    PageServiceMessage* msg = GET_MEMORY_MSG(memory_message_buffer, idx);
    if (msg->status == PAGE_ALLOCATED)
        release_page(*msg);
    msg->status = PAGE_RAW;
}

byte PageService::initiate_page(const PageServiceMessage& msg)
{
    SPDLOG_INFO("Initiate page {}/{}", msg.folder, msg.name);

    string path = PageUtil::GenPageFullPath(msg.folder, msg.name, msg.page_num);
    if (fileAddrs.find(path) == fileAddrs.end())
    {
        void* buffer = nullptr;
        if (!PageUtil::FileExists(path))
        {   // this file is not exist....
            if (!msg.is_writer)
                return PAGE_NON_EXIST;
            else
            {
                auto tempPageIter = fileAddrs.find(TEMP_PAGE);
                if (tempPageIter != fileAddrs.end())
                {
                    int ret = rename((TEMP_PAGE).c_str(), path.c_str());
                    if (ret < 0)
                    {
                        SPDLOG_ERROR("Cannot rename from {} to {}", TEMP_PAGE, path);
                        return PAGE_CANNOT_RENAME_FROM_TEMP;
                    }
                    else
                    {
                        SPDLOG_INFO("Renamed {} to {}", TEMP_PAGE, path);
                        buffer = tempPageIter->second;
                        fileAddrs.erase(tempPageIter);
                    }
                }
                if (buffer == nullptr)
                    buffer = PageUtil::LoadPageBuffer(path, JOURNAL_PAGE_SIZE, msg.is_writer, false);
            }
        }
        else
        {   // exist file but not loaded, map and lock immediately.
            buffer = PageUtil::LoadPageBuffer(path, JOURNAL_PAGE_SIZE, msg.is_writer, false);
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
            return PAGE_MORE_THAN_ONE_WRITE;
    }
    else
    {
        auto count_it = fileReaderCounts.find(msg);
        if (count_it == fileReaderCounts.end())
            fileReaderCounts[msg] = 1;
        else
            count_it->second ++;
    }
    return PAGE_ALLOCATED;
}

void PageService::release_page(const PageServiceMessage& msg)
{
    SPDLOG_INFO("Release page {}/{}", msg.folder, msg.name);

    map<PageServiceMessage, int>::iterator count_it;
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
