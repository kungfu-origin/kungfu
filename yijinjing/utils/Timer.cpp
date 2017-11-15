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

#include "Timer.h"
#include <chrono>

USING_YJJ_NAMESPACE

boost::shared_ptr<NanoTimer> NanoTimer::m_ptr = boost::shared_ptr<NanoTimer>(nullptr);

NanoTimer* NanoTimer::getInstance()
{
    if (m_ptr.get() == nullptr)
    {
        m_ptr = boost::shared_ptr<NanoTimer>(new NanoTimer());
    }
    return m_ptr.get();
}

NanoTimer::NanoTimer()
{
    int unix_second_num = std::chrono::seconds(std::time(NULL)).count();
    int tick_second_num = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
    secDiff = (unix_second_num - tick_second_num) * NANOSECONDS_PER_SECOND;
}

long NanoTimer::getNano()
{
    long _nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
    return _nano + secDiff;
}
