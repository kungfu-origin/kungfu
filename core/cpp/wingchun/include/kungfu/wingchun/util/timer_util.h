//
// Created by qlu on 2019/3/25.
//

#ifndef KUNGFU_TIMER_UTIL_H
#define KUNGFU_TIMER_UTIL_H

#include <kungfu/yijinjing/time.h>

namespace kungfu
{
    #define SWITCH_HOUR 17

    inline std::string get_trading_day_from_nano(int64_t nano)
    {
        return yijinjing::time::strftime(nano + (yijinjing::time_unit::NANOSECONDS_PER_DAY - SWITCH_HOUR * yijinjing::time_unit::NANOSECONDS_PER_HOUR),
                                         "%Y%m%d");
    }

    inline int64_t get_nano_from_trading_day(const std::string& trading_day, const std::string& time)
    {
        int hour, min, sec;
        sscanf(time.c_str(), "%d:%d:%d", &hour, &min, &sec);
        int n_hour = (hour+yijinjing::time_unit::HOURS_PER_DAY-SWITCH_HOUR)%yijinjing::time_unit::HOURS_PER_DAY;
        int64_t nano = yijinjing::time::strptime(
                (trading_day + std::string("-") + std::to_string(n_hour) + std::string(":") + std::to_string(min) + std::string(":") +
                 std::to_string(sec)).c_str(), "%Y%m%d-%H:%M:%S");
        nano = nano - yijinjing::time_unit::NANOSECONDS_PER_DAY + SWITCH_HOUR * yijinjing::time_unit::NANOSECONDS_PER_HOUR;
        return nano;
    }
}

#endif //KUNGFU_TIMER_UTIL_H
