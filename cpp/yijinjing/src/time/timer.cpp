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
 * Timer for kungfu system.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide basic nano time and time transformation
 */

#include <nlohmann/json.hpp>

#include <chrono>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>
#include <kungfu/yijinjing/journal/journal.h>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

std::shared_ptr<NanoTimer> NanoTimer::m_ptr = std::shared_ptr<NanoTimer>(nullptr);

NanoTimer* NanoTimer::getInstance()
{
    if (m_ptr.get() == nullptr)
    {
        if(spdlog::default_logger()->name().empty())
        {
            kungfu::log::LogConfig::setup_log("timer");
        }
        m_ptr = std::shared_ptr<NanoTimer>(new NanoTimer());
    }
    return m_ptr.get();
}

inline int64_t now_in_nano()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

NanoTimer::NanoTimer()
{
    int64_t system_nanotime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
    int64_t local_nanotime = now_in_nano();
    basetime_in_nano = system_nanotime - local_nanotime;
    SPDLOG_TRACE("Initialized timer from local: now {}, diff {}", now_in_nano(), basetime_in_nano);
}

int64_t NanoTimer::getNano() const
{
    return now_in_nano() + basetime_in_nano;
}
