//
// Created by PolarAir on 2019-02-28.
//

#ifndef KUNGFU_CALENDAR_H
#define KUNGFU_CALENDAR_H

#include <string>

namespace kungfu
{
    namespace wingchun
    {
        class Calendar
        {
        public:

            virtual const std::string get_current_trading_day() const = 0;

            virtual const std::string get_next_trading_day(const std::string& trading_day, int32_t delta_days) = 0;

            virtual bool is_open(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) = 0;

            virtual const std::chrono::system_clock::time_point get_open_time(const std::string& trading_day, const std::string& exchange_id) = 0;
            virtual const std::chrono::system_clock::time_point get_close_time(const std::string& trading_day, const std::string& exchange_id) = 0;

            virtual const std::chrono::system_clock::time_point next_open(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) = 0;
            virtual const std::chrono::system_clock::time_point next_close(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) = 0;

        };
        DECLARE_PTR(Calendar)
    }
}

#endif //KUNGFU_CALENDAR_H
