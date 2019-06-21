//
// Created by qlu on 2019/4/15.
//

#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/msg.h>

#define SWITCH_HOUR 17
#define REMAINING_HOUR (yijinjing::time_unit::NANOSECONDS_PER_DAY - SWITCH_HOUR * yijinjing::time_unit::NANOSECONDS_PER_HOUR)

namespace kungfu
{
    namespace wingchun
    {

        std::string get_trading_day_from_nano(int64_t nano)
        {
            return yijinjing::time::strftime(nano + REMAINING_HOUR, "%Y%m%d");
        }

        int64_t get_nano_from_trading_day(const std::string& trading_day, const std::string& time)
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

        Calendar::Calendar(const std::string& calendar_db_file)
        {
            storage::CalendarStorage storage(calendar_db_file);
            storage.get_trading_days(1990, 2090, trading_days_, REGION_CN);
        }

        Calendar::~Calendar()
        {}

        bool Calendar::is_open(int64_t nano, const std::string &exchange_id)
        {
            std::string trading_day = get_trading_day_from_nano(nano);
            trading_day = calculate_trading_day(std::atoi(trading_day.c_str()), 0);
            if (trading_evening[exchange_id])
            {
                int64_t evening_start = get_open_time(trading_day, exchange_id, EVENING);
                int64_t evening_end = get_close_time(trading_day, exchange_id, EVENING);
                if (nano >= evening_start && nano <= evening_end)
                {
                    return true;
                }
            }
            int64_t morning_start = get_open_time(trading_day, exchange_id, MORNING);
            int64_t morning_end = get_close_time(trading_day, exchange_id, MORNING);
            int64_t afternoon_start = get_open_time(trading_day, exchange_id, AFTERNOON);
            int64_t afternoon_end = get_close_time(trading_day, exchange_id, AFTERNOON);
            return ((nano >= morning_start && nano <= morning_end) || (nano >= afternoon_start && nano <= afternoon_end));
        }

        int64_t Calendar::get_open_time(const std::string &trading_day, const std::string &exchange_id, int slot)
        {
            if (slot == DEFAULT_SLOT)
            {
                slot = trading_evening[exchange_id] ? EVENING : MORNING;
            }
            return get_nano_from_trading_day(trading_day, trading_times[exchange_id][0][slot]);
        }

        int64_t Calendar::get_close_time(const std::string &trading_day, const std::string &exchange_id, int slot)
        {
            if (slot == DEFAULT_SLOT)
            {
                slot = AFTERNOON;
            }
            return get_nano_from_trading_day(trading_day, trading_times[exchange_id][1][slot]);
        }

        int64_t Calendar::next_open(int64_t nano, const std::string &exchange_id, int slot)
        {
            std::string trading_day = get_trading_day_from_nano(nano);
            trading_day = calculate_trading_day(std::atoi(trading_day.c_str()), 0);
            if (slot == DEFAULT_SLOT || (!trading_evening[exchange_id] && slot == EVENING))
            {
                slot = MORNING;
            }
            int64_t start = get_open_time(trading_day, exchange_id, slot);
            if (nano <= start)
            {
                return start;
            }
            trading_day = calculate_trading_day(std::atoi(trading_day.c_str()), 1);
            return get_open_time(trading_day, exchange_id, slot);
        }

        int64_t Calendar::next_close(int64_t nano, const std::string &exchange_id, int slot)
        {
            std::string trading_day = get_trading_day_from_nano(nano);
            trading_day = calculate_trading_day(std::atoi(trading_day.c_str()), 0);
            if (slot == DEFAULT_SLOT || (!trading_evening[exchange_id] && slot == EVENING))
            {
                slot = AFTERNOON;
            }
            int64_t start = get_close_time(trading_day, exchange_id, slot);
            if (nano <= start)
            {
                return start;
            }
            trading_day = calculate_trading_day(std::atoi(trading_day.c_str()), 1);
            return get_close_time(trading_day, exchange_id, slot);
        }

        std::vector<msg::nanomsg::TradingSession> Calendar::get_trading_sessions(int64_t start_nano, int64_t end_nano, const std::string &exchange_id)
        {
            std::vector<msg::nanomsg::TradingSession> result;
            if (end_nano < start_nano)
            {
                return result;
            }
            std::string start_trading_day = get_trading_day_from_nano(start_nano);
            std::string end_trading_day = get_trading_day_from_nano(end_nano);
            int64_t start, end;
            while (true)
            {
                if (trading_evening[exchange_id])
                {
                    int64_t evening_start = get_open_time(start_trading_day, exchange_id, EVENING);
                    int64_t evening_end = get_close_time(start_trading_day, exchange_id, EVENING);
                    start = evening_start < start_nano ? start_nano : evening_start;
                    end = evening_end < end_nano ? evening_end : end_nano;
                    if (start <= end)
                    {
                        result.push_back({exchange_id, start_trading_day, start, end});
                    }
                }
                int64_t morning_start = get_open_time(start_trading_day, exchange_id, MORNING);
                int64_t morning_end = get_close_time(start_trading_day, exchange_id, MORNING);
                start = morning_start < start_nano ? start_nano : morning_start;
                end = morning_end < end_nano ? morning_end : end_nano;
                if (start <= end)
                {
                    result.push_back({exchange_id, start_trading_day, start, end});
                }
                int64_t afternoon_start = get_open_time(start_trading_day, exchange_id, AFTERNOON);
                int64_t afternoon_end = get_close_time(start_trading_day, exchange_id, AFTERNOON);
                start = afternoon_start < start_nano ? start_nano : afternoon_start;
                end = afternoon_end < end_nano ? afternoon_end : end_nano;
                if (start <= end)
                {
                    result.push_back({exchange_id, start_trading_day, start, end});
                }
                if (start_trading_day == end_trading_day)break;
                start_trading_day = calculate_trading_day(std::atoi(start_trading_day.c_str()), 1);
            }
            return result;
        }

        int Calendar::get_current_trading_day()
        {
            auto cur_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int cur_date = std::stoi(yijinjing::time::strftime(cur_nano, "%Y%m%d"));
            int cur_hour = std::stoi(yijinjing::time::strftime(cur_nano, "%H"));
            return calculate_trading_day(cur_date, cur_hour >= SWITCH_HOUR ? 1 : 0);
        }

        int Calendar::get_next_trading_day()
        {
            return calculate_trading_day(get_current_trading_day(), 1);
        }

        int Calendar::calculate_trading_day(int date, int delta) const
        {
            int trading_days_size = trading_days_.size();
            int lo = 0, hi = trading_days_size;
            while (lo < hi)
            {
                int mi = (lo + hi) >> 1;
                (date <= trading_days_[mi]) ? hi = mi : lo = mi + 1;
            }
            int found = lo;
            int ret;
            if (found + delta < trading_days_size && found + delta >= 0)
            {
                ret = trading_days_[found + delta];
            } else if (found + delta < 0)
            {
                ret = trading_days_[0];
            } else
            {
                ret = trading_days_[trading_days_size - 1];
            }
            return ret;
        }
    }
}
