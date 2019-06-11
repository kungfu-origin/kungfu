/*****************************************************************************
 * Copyright [taurus.ai]
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

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <regex>
#include <fmt/format.h>

#include <kungfu/yijinjing/time.h>

using namespace std::chrono;
using namespace kungfu::yijinjing;

int64_t time::now_in_nano()
{
    auto duration = steady_clock::now().time_since_epoch().count() - get_instance().start_time_steady_;
    return get_instance().start_time_since_epoch_ + duration;
}

int64_t time::strptime(const std::string &timestr, const std::string &format)
{
    int64_t nano = 0;
    std::string normal_timestr = timestr;
    std::string normal_format = format;

    std::regex nano_format_regex("%N");
    if (std::regex_search(format, nano_format_regex))
    {
        normal_format = std::regex_replace(format, nano_format_regex, "");

        std::regex nano_regex("(\\d{9})");

        auto nano_begin = std::sregex_iterator(normal_timestr.begin(), normal_timestr.end(), nano_regex);
        auto nano_end = std::sregex_iterator();
        for (std::sregex_iterator i = nano_begin; i != nano_end; ++i)
        {
            std::smatch match = *i;
            nano = stol(match.str());
            normal_timestr = std::regex_replace(timestr, nano_regex, "");
        }
    }

    std::tm result = {};
    std::istringstream iss(timestr);
    iss >> std::get_time(&result, normal_format.c_str());
    std::time_t parsed_time = std::mktime(&result);
    auto tp_system = system_clock::from_time_t(parsed_time);
    return duration_cast<nanoseconds>(tp_system.time_since_epoch()).count() + nano;
}

const std::string time::strftime(const int64_t nanotime, const std::string &format)
{
    time_point<steady_clock> tp_steady((nanoseconds(nanotime)));
    auto tp_epoch_steady = time_point<steady_clock>{};
    auto tp_diff = tp_steady - tp_epoch_steady;

    auto tp_epoch_system = time_point<system_clock>{};
    std::time_t time_since_epoch = system_clock::to_time_t(tp_epoch_system + duration_cast<system_clock::duration>(tp_diff));

    std::string normal_format = format;
    std::regex nano_format_regex("%N");
    if (std::regex_search(format, nano_format_regex))
    {
        int64_t nano = tp_diff.count() % time_unit::NANOSECONDS_PER_SECOND;
        normal_format = std::regex_replace(format, nano_format_regex, fmt::format("{:09d}", nano));
    }

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_since_epoch), normal_format.c_str());
    if (nanotime > 0)
    {
        return oss.str();
    }
    else if (nanotime == 0)
    {
        return std::regex_replace(oss.str(), std::regex("\\d"), "0");
    }
    else
    {
        return std::regex_replace(oss.str(), std::regex("\\d"), "#");
    }
}

/**
 * start_time_steady_ sample:       867884767983511
 * start_time_since_epoch_ sample:  1560144011373015000
 */
time::time()
{
    auto now = system_clock::now();
    start_time_steady_ = steady_clock::now().time_since_epoch().count();
    start_time_since_epoch_ = duration_cast<nanoseconds>(now.time_since_epoch()).count();
}

const kungfu::yijinjing::time &time::get_instance()
{
    static time t;
    return t;
}
