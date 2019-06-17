//
// Created by qlu on 2019/6/17.
//

#ifndef KUNGFU_CRON_H
#define KUNGFU_CRON_H

#include <chrono>
#include <string>
#include <sstream>
#include <vector>

namespace kungfu
{
    class CronException : public std::exception
    {
    public:
        CronException(const std::string &message) : message_(message)
        {}

        virtual const char *what() const throw()
        { return message_.c_str(); };

    private:
        const std::string message_;
    };

    inline void add(std::tm &tm, std::chrono::system_clock::duration time)
    {
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        auto tp_adjusted = tp + time;
        auto tm_adjusted = std::chrono::system_clock::to_time_t(tp_adjusted);
        tm = *std::localtime(&tm_adjusted);
    }

    inline void verify_and_set(const std::string &token, const std::string &expression, int &field, const int lower_bound,
                   const int upper_bound, const bool adjust = false)
    {
        if (token == "*")
        {
            field = -1;
        }
        else
        {
            try
            {
                field = std::stoi(token);
            }
            catch (const std::invalid_argument &)
            {
                throw CronException("cron string (`" + token + "` not an integer or *): " + expression);
            }
            catch (const std::out_of_range &)
            {
                throw CronException("cron string (`" + token + "` not convertable to int): " + expression);
            }
            if (field < lower_bound || field > upper_bound)
            {
                std::ostringstream oss;
                oss << "cron string ('" << token << "' must be <= " << upper_bound << " and >= " << lower_bound
                    << "): " << expression;
                throw CronException(oss.str());
            }
            if (adjust)
            {
                field--;
            }
        }
    }

    class Cron
    {
    public:
        Cron(const std::string &expression)
        {
            std::istringstream iss(expression);
            std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                            std::istream_iterator<std::string>{}};

            if (tokens.size() != 5)
            {
                throw CronException("cron string (must be 5 fields): " + expression);
            }
            verify_and_set(tokens[0], expression, minute_, 0, 59);
            verify_and_set(tokens[1], expression, hour_, 0, 23);
            verify_and_set(tokens[2], expression, day_, 1, 31);
            verify_and_set(tokens[3], expression, month_, 1, 12, true);
            verify_and_set(tokens[4], expression, day_of_week_, 0, 6);
        }

        std::chrono::system_clock::time_point cron_to_next(const std::chrono::system_clock::time_point from) const
        {
            auto now = std::chrono::system_clock::to_time_t(from);
            std::tm next(*std::localtime(&now));
            next.tm_sec = 0;
            add(next, std::chrono::minutes(1));
            while (true)
            {
                if (month_ != -1 && next.tm_mon != month_)
                {
                    if (next.tm_mon + 1 > 11)
                    {
                        next.tm_mon = 0;
                        next.tm_year++;
                    } else
                        next.tm_mon++;

                    next.tm_mday = 1;
                    next.tm_hour = 0;
                    next.tm_min = 0;
                    continue;
                }
                if (day_ != -1 && next.tm_mday != day_)
                {
                    add(next, std::chrono::hours(24));
                    next.tm_hour = 0;
                    next.tm_min = 0;
                    continue;
                }
                if (day_of_week_ != -1 && next.tm_wday != day_of_week_)
                {
                    add(next, std::chrono::hours(24));
                    next.tm_hour = 0;
                    next.tm_min = 0;
                    continue;
                }
                if (hour_ != -1 && next.tm_hour != hour_)
                {
                    add(next, std::chrono::hours(1));
                    next.tm_min = 0;
                    continue;
                }
                if (minute_ != -1 && next.tm_min != minute_)
                {
                    add(next, std::chrono::minutes(1));
                    continue;
                }
                break;
            }

            next.tm_isdst = -1;
            return std::chrono::system_clock::from_time_t(std::mktime(&next));
        }
    private:
        int minute_;
        int hour_;
        int day_;
        int month_;
        int day_of_week_;
    };
}
#endif //KUNGFU_CRON_H
