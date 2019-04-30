//
// Created by qlu on 2019/3/25.
//

#ifndef KUNGFU_TIMER_UTIL_H
#define KUNGFU_TIMER_UTIL_H

#include "Timer.h"

namespace kungfu
{
    #define SWITCH_HOUR 17

    inline long nseconds_next_min(long cur_nano)
    {
        std::string cur_time = yijinjing::parseNano(cur_nano, "%Y%m%d %H:%M:%S");
        return yijinjing::parseTime((cur_time.substr(0, cur_time.size() - 2) + "00").c_str(), "%Y%m%d %H:%M:%S") + yijinjing::NANOSECONDS_PER_MINUTE;
    }

    inline long nseconds_next_day(long cur_nano)
    {
        std::string cur_time = yijinjing::parseNano(cur_nano, "%Y%m%d %H:%M:%S");
        long next_day = yijinjing::parseTime((cur_time.substr(0, 9) + DAILY_STORAGE_TIME).c_str(), "%Y%m%d %H:%M:%S");
        if (next_day < yijinjing::getNanoTime())
        {
            next_day += yijinjing::NANOSECONDS_PER_DAY;
        }
        return next_day;
    }

    inline std::string get_trading_day_from_nano(long nano)
    {
        return yijinjing::parseNano(nano + (yijinjing::NANOSECONDS_PER_DAY - SWITCH_HOUR * yijinjing::NANOSECONDS_PER_HOUR), "%Y%m%d");
    }

    inline long get_nano_from_trading_day(const std::string& trading_day, const std::string& time)
    {
        int hour, min, sec;
        sscanf(time.c_str(), "%d:%d:%d", &hour, &min, &sec);
        int n_hour = (hour+yijinjing::HOURS_PER_DAY-SWITCH_HOUR)%yijinjing::HOURS_PER_DAY;
        long nano = yijinjing::parseTime((trading_day+std::string("-")+std::to_string(n_hour)+std::string(":")+std::to_string(min)+std::string(":")+std::to_string(sec)).c_str(), "%Y%m%d-%H:%M:%S");
        nano = nano - yijinjing::NANOSECONDS_PER_DAY + SWITCH_HOUR * yijinjing::NANOSECONDS_PER_HOUR;
        return nano;
    }
}

#endif //KUNGFU_TIMER_UTIL_H
