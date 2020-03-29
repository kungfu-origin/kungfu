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

#ifndef YIJINJING_TIME_H
#define YIJINJING_TIME_H

#include <string>

#define KUNGFU_DATETIME_FORMAT_DEFAULT "%F %T.%N"

namespace kungfu::yijinjing {
struct time_unit {
  static constexpr int64_t MILLISECONDS_PER_SECOND = 1000;
  static constexpr int64_t MICROSECONDS_PER_MILLISECOND = 1000;
  static constexpr int64_t NANOSECONDS_PER_MICROSECOND = 1000;

  static constexpr int64_t MICROSECONDS_PER_SECOND = MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
  static constexpr int64_t NANOSECONDS_PER_MILLISECOND = NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
  static constexpr int64_t NANOSECONDS_PER_SECOND = NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;

  static constexpr int64_t SECONDS_PER_MINUTE = 60;
  static constexpr int64_t MINUTES_PER_HOUR = 60;
  static constexpr int64_t HOURS_PER_DAY = 24;
  static constexpr int64_t SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

  static constexpr int64_t MILLISECONDS_PER_MINUTE = MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE;
  static constexpr int64_t NANOSECONDS_PER_MINUTE = NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
  static constexpr int64_t NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
  static constexpr int64_t NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;

  static constexpr int64_t UTC_OFFSET = NANOSECONDS_PER_HOUR * 8;
};

class time {
public:
  /**
   * Get timestamp in nano seconds.
   * @return current nano time in int64_t (unix-timestamp * 1e9 + nano-part)
   */
  static int64_t now_in_nano();

  /**
   * Given a timestamp, returns the start point of next minute.
   * @param nanotime timestamp in nano seconds
   * @return start time point of next minute in nano seconds
   */
  static int64_t next_minute(int64_t nanotime);

  /**
   * Given a timestamp, returns the next end of trading time, i.e. 15:30 of today if the argument is before that,
   * or 15:30 or tomorrow.
   * @param nanotime timestamp in nano seconds
   * @return the next trading session end time point in nano seconds
   */
  static int64_t next_trading_day_end(int64_t nanotime);

  /**
   * Start time of today (00:00:00) in nano seconds.
   * @return start time of today in nano seconds
   */
  static int64_t today_start();

  /**
   * Parse string time to nano time.
   * @param timestr string-formatted time
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return nano time in int64_t
   */
  static int64_t strptime(const std::string &timestr, const std::string &format = KUNGFU_DATETIME_FORMAT_DEFAULT);

  /**
   * Format nano seconds to string.
   * @param nanotime nano time in int64_t
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return string-formatted time
   */
  static std::string strftime(int64_t nanotime, const std::string &format = KUNGFU_DATETIME_FORMAT_DEFAULT);

  /**
   * Format now to string.
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return string-formatted time
   */
  static std::string strfnow(const std::string &format = KUNGFU_DATETIME_FORMAT_DEFAULT);

private:
  time();

  static const time &get_instance();

  int64_t start_time_system_ = 0;
  int64_t start_time_steady_ = 0;
};
} // namespace kungfu::yijinjing

#endif // YIJINJING_TIME_H