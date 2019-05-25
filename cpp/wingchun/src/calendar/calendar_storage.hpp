//
// Created by PolarAir on 2019-02-28.
//

#ifndef KUNGFU_CALENDAR_STORAGE_H
#define KUNGFU_CALENDAR_STORAGE_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <spdlog/spdlog.h>
#include <set>
#include <string>
#include <vector>

namespace kungfu
{
    class CalendarStorage
    {
    public:
        CalendarStorage(const std::string& file_name) : db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }
        void create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS holidays(\n"
                                  "                    region       CHAR(50),\n"
                                  "                    holiday      INTEGER)\n";
                db_.exec(sql);
            }
            catch (std::exception& e)
            {
                SPDLOG_ERROR(e.what());
            }
        }
        const void get_trading_days(int from_year, int to_year, std::vector<int>& trading_days,
                                    const char* region)
        {
            try
            {
                std::set<int> holidays;
                SQLite::Statement query(db_, "SELECT holiday FROM holidays WHERE region = ?");
                query.bind(1, region);
                if (query.executeStep())
                {
                    holidays.insert(query.getColumn(0));
                }
                int start = 0;
                for (int year = from_year; year <= to_year; year++)
                {
                    std::vector<int> months = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
                    {
                        months[2] = 29;
                    }
                    for (int month = 1; month <= 12; month++)
                    {
                        for (int day = 1; day <= months[month]; day++)
                        {
                            if ((start + day) % 7 >= 1 && (start + day) % 7 <= 5)
                            {
                                int current_day = year * 10000 + month * 100 + day;
                                auto search = holidays.find(current_day);
                                if (search == holidays.end())
                                {
                                    trading_days.push_back(current_day);
                                }
                            }
                        }
                        start += months[month];
                    }
                }

            } catch (std::exception& e)
            {
                SPDLOG_ERROR(e.what());
            }
        }

    private:
        SQLite::Database db_;
    };
} // namespace kungfu

#endif // KUNGFU_CALENDAR_MSG_H