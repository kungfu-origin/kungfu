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
#include "PageEngine.h"
#include "PageUtil.h"
#include "Timer.h"
#ifdef ENABLE_ACTIVATION_CODE
#include "ActivationCode.hpp"
#endif

USING_YJJ_NAMESPACE

#define TEMP_PAGE KUNGFU_JOURNAL_FOLDER "TEMP_PAGE"
const string PstTempPage::PageFullPath = TEMP_PAGE;

PstPidCheck::PstPidCheck(PageEngine *pe): engine(pe) {}

void PstPidCheck::go()
{
    vector<string> clientsToRemove;
    {
        for (auto const &item: engine->pidClient)
        {
            struct stat sts;
            std::stringstream ss;
            ss << "/proc/" << item.first;
            if (stat(ss.str().c_str(), &sts) == -1 && errno == ENOENT)
            {
                for (auto const &name: item.second)
                {
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

PstTimeTick::PstTimeTick(PageEngine *pe): engine(pe) {}

void PstTimeTick::go()
{
    engine->write("", MSG_TYPE_TIME_TICK);
}

PstTempPage::PstTempPage(PageEngine *pe): engine(pe) {}

void PstTempPage::go()
{
    auto &fileAddrs = engine->fileAddrs;
    if (fileAddrs.find(PageFullPath) == fileAddrs.end())
    {
        KF_LOG_INFO(engine->get_logger(), "NEW TEMP PAGE: " << PageFullPath);
        void *buffer = PageUtil::LoadPageBuffer(PageFullPath, JOURNAL_PAGE_SIZE, true, true);
        if (buffer != nullptr)
        {
            fileAddrs[PageFullPath] = buffer;
        }
    }
}

PstKfController::PstKfController(PageEngine *pe): engine(pe) {}

long getFirstNano(string& formatTime)
{
    long cur_nano = getNanoTime();
    struct tm time_part;
    strptime(formatTime.c_str(), "%H:%M:%S", &time_part);
    struct tm current_part = parseNano(cur_nano);
    current_part.tm_hour = time_part.tm_hour;
    current_part.tm_min = time_part.tm_min;
    current_part.tm_sec = time_part.tm_sec;
    long res = parseTm(current_part);
    while (res < cur_nano)
        res += NANOSECONDS_PER_DAY;
    return res;
}

void PstKfController::setDaySwitch(string formatTime)
{
    long cur_nano = getFirstNano(formatTime);
    engine->set_last_switch_nano(cur_nano - NANOSECONDS_PER_DAY);
    next_nanos.push_back(cur_nano);
    nano_types.push_back(CONTROLLER_SWITCH_DAY);
}

void PstKfController::addEngineStart(string formatTime)
{
    long cur_nano = getFirstNano(formatTime);
    next_nanos.push_back(cur_nano);
    nano_types.push_back(CONTROLLER_ENGINE_STARTS);
}

void PstKfController::addEngineEnd(string formatTime)
{
    long cur_nano = getFirstNano(formatTime);
    next_nanos.push_back(cur_nano);
    nano_types.push_back(CONTROLLER_ENGINE_ENDS);
}

void PstKfController::go()
{
    long nano = getNanoTime();
    for (size_t i = 0; i < next_nanos.size(); i++)
    {
        long& l = next_nanos[i];
        if (l < nano)
        {
            short tp = nano_types[i];
            switch(tp)
            {
                case CONTROLLER_SWITCH_DAY:
                    engine->switch_trading_day();
                    engine->set_last_switch_nano(l);
                    break;
                case CONTROLLER_ENGINE_STARTS:
                    engine->write("", MSG_TYPE_MD_ENGINE_OPEN);
                    engine->write("", MSG_TYPE_TRADE_ENGINE_OPEN);
                    break;
                case CONTROLLER_ENGINE_ENDS:
                    engine->write("", MSG_TYPE_MD_ENGINE_CLOSE);
                    engine->write("", MSG_TYPE_TRADE_ENGINE_CLOSE);
                    break;
            }
            l += NANOSECONDS_PER_DAY;
        }
    }
}

boost::python::dict PstKfController::getInfo() const
{
    boost::python::dict res;
    string switch_day;
    boost::python::list engine_starts;
    boost::python::list engine_ends;
    for (size_t i = 0; i < next_nanos.size(); i++)
    {
        string time_str = parseNano(next_nanos[i], "%Y-%m-%d %H:%M:%S");

        switch (nano_types[i])
        {
            case CONTROLLER_SWITCH_DAY:
                switch_day = time_str;
                break;
            case CONTROLLER_ENGINE_STARTS:
                engine_starts.append(time_str);
                break;
            case CONTROLLER_ENGINE_ENDS:
                engine_ends.append(time_str);
                break;
        }
    }
    res["switch_day"] = switch_day;
    res["engine_starts"] = engine_starts;
    res["engine_ends"] = engine_ends;
    return res;
}
