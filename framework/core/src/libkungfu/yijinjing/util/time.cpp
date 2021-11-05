/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
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
#include <ctime>
#include <fmt/format.h>
#include <iomanip>
#include <regex>
#include <sstream>

#include <kungfu/yijinjing/time.h>

using namespace std::chrono;

namespace kungfu::yijinjing {
int64_t time::now_in_nano() {
  auto duration = steady_clock::now().time_since_epoch().count() - get_instance().base_.steady_clock_count;
  return get_instance().base_.system_clock_count + duration;
}

int64_t time::next_minute(int64_t nanotime) {
  return nanotime - nanotime % time_unit::NANOSECONDS_PER_MINUTE + time_unit::NANOSECONDS_PER_MINUTE;
}

int64_t time::next_trading_day_end(int64_t nanotime) {
  int64_t end_offset = 15 * time_unit::NANOSECONDS_PER_HOUR + 30 * time_unit::NANOSECONDS_PER_MINUTE;
  int64_t trading_day = nanotime - nanotime % time_unit::NANOSECONDS_PER_DAY - time_unit::UTC_OFFSET + end_offset;
  if (trading_day < now_in_nano()) {
    trading_day += time_unit::NANOSECONDS_PER_DAY;
  }
  return trading_day;
}

int64_t time::calendar_day_start(int64_t nanotime) {
  return nanotime - (nanotime % time_unit::NANOSECONDS_PER_DAY) - time_unit::UTC_OFFSET;
}

int64_t time::today_start() { return calendar_day_start(time::now_in_nano()); }

int64_t time::strptime(const std::string &time_string, const std::string &format) {
  int64_t nano = 0;
  std::string normal_timestr = time_string;
  std::string normal_format = format;

  std::regex nano_format_regex("%N");
  if (std::regex_search(format, nano_format_regex)) {
    normal_format = std::regex_replace(format, nano_format_regex, "");

    std::regex nano_regex("(\\d{9})");

    auto nano_begin = std::sregex_iterator(normal_timestr.begin(), normal_timestr.end(), nano_regex);
    auto nano_end = std::sregex_iterator();
    for (std::sregex_iterator i = nano_begin; i != nano_end; ++i) {
      std::smatch match = *i;
      nano = stol(match.str());
      normal_timestr = std::regex_replace(time_string, nano_regex, "");
    }
  }

  std::tm result = {};
  std::istringstream iss(time_string);
  iss >> std::get_time(&result, normal_format.c_str());
  std::time_t parsed_time = std::mktime(&result);
  auto tp_system = system_clock::from_time_t(parsed_time);
  return duration_cast<nanoseconds>(tp_system.time_since_epoch()).count() + nano;
}

int64_t time::strptime(const std::string &time_string, std::initializer_list<std::string> formats) {
  for (const auto &format : formats) {
    auto t = strptime(time_string, format);
    if (strftime(t, format) == time_string) {
      return t;
    }
  }
  return -1;
}

std::string time::strftime(int64_t nanotime, const std::string &format) {
  if (nanotime == INT64_MAX) {
    return "end of world";
  }
  time_point<steady_clock> tp_steady((nanoseconds(nanotime)));
  auto tp_epoch_steady = time_point<steady_clock>{};
  auto tp_diff = tp_steady - tp_epoch_steady;

  auto tp_epoch_system = time_point<system_clock>{};
  std::time_t time_since_epoch =
      system_clock::to_time_t(tp_epoch_system + duration_cast<system_clock::duration>(tp_diff));

  std::string normal_format = format;
  std::regex nano_format_regex("%N");
  if (std::regex_search(format, nano_format_regex)) {
    int64_t nano = tp_diff.count() % time_unit::NANOSECONDS_PER_SECOND;
    normal_format = std::regex_replace(format, nano_format_regex, fmt::format("{:09d}", nano));
  }

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&time_since_epoch), normal_format.c_str());
  if (nanotime > 0) {
    return oss.str();
  } else if (nanotime == 0) {
    return std::regex_replace(oss.str(), std::regex("\\d"), "0");
  } else {
    return std::regex_replace(oss.str(), std::regex("\\d"), "#");
  }
}

std::string time::strfnow(const std::string &format) { return strftime(now_in_nano(), format); }

time_point_info time::get_base() { return get_instance().base_; }

void time::reset(int64_t system_clock_count, int64_t steady_clock_count) {
  time_point_info &base = const_cast<time &>(get_instance()).base_;
  base.system_clock_count = system_clock_count;
  base.steady_clock_count = steady_clock_count;
}

/**
 * start_time_system_ sample: 1560144011373015000
 * start_time_steady_ sample: 867884767983511
 */
time::time() : base_() {
  auto system_clock_now = system_clock::now();
  auto steady_clock_now = steady_clock::now();
  base_.system_clock_count = duration_cast<nanoseconds>(system_clock_now.time_since_epoch()).count();
  base_.steady_clock_count = steady_clock_now.time_since_epoch().count();
}

const time &time::get_instance() {
  static time instance = {};
  return instance;
}
} // namespace kungfu::yijinjing
