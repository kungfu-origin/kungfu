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

/**
 * Page Service Tasks.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Page engine needs several tasks to be done in schedule.
 * here we define tasks which can be implemented in page engine
 */

#include "PageServiceTask.h"
#include "PageService.h"
#include "PageUtil.h"
#include "Timer.h"
#include "Journal.h"
#ifdef ENABLE_ACTIVATION_CODE
#include "ActivationCode.hpp"
#endif

#ifdef _WINDOWS
#include <windows.h>
#include <Processthreadsapi.h>
#elif defined __APPLE__
#include <libproc.h>
#elif defined __linux__
#include <sstream>
#endif

USING_YJJ_NAMESPACE

PstPidCheck::PstPidCheck(PageService *pe): engine(pe) {}

void PstPidCheck::go()
{
    int pid;
#ifdef __APPLE__
    struct proc_taskallinfo ti;
    int nb;
#endif
    vector<string> clientsToRemove;
    {
        for (auto const &item: engine->pidClient)
        {
            pid = item.first;
#ifdef _WINDOWS
            HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            DWORD exitCode = 0;
            if (GetExitCodeProcess(process, &exitCode) == FALSE)
            {
                SPDLOG_CRITICAL("pid check failed {}", pid);
            }
            if (exitCode != STILL_ACTIVE)
#elif defined __APPLE__
            nb = proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &ti, sizeof(ti));
            if (nb == 0)
#elif defined __linux__
            struct stat sts;
            std::stringstream ss;
            ss << "/proc/" << pid;
            if (stat(ss.str().c_str(), &sts) == -1 && errno == ENOENT)
#endif
            {
                for (auto const &name: item.second)
                {
                    SPDLOG_WARN("process {} with pid {} exited", name, pid);
                    clientsToRemove.push_back(name);
                }
            }
        }
    }
    for (auto const &name: clientsToRemove)
    {
        engine->exit_client(name, 0, false);
    }
}

PstTimeTick::PstTimeTick(PageService *pe): engine(pe) {}

void PstTimeTick::go()
{
    engine->write("", MSG_TYPE_TIME_TICK);
}

PstTempPage::PstTempPage(PageService *pe): engine(pe) {}

void PstTempPage::go()
{
    auto &fileAddrs = engine->fileAddrs;
    if (fileAddrs.find(TEMP_PAGE) == fileAddrs.end())
    {
        SPDLOG_INFO("NEW TEMP PAGE: {}", TEMP_PAGE);
        void *buffer = PageUtil::LoadPageBuffer(TEMP_PAGE, JOURNAL_PAGE_SIZE, true, true);
        if (buffer != nullptr)
        {
            fileAddrs[TEMP_PAGE] = buffer;
        }
    }
}

