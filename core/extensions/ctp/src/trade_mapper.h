//
// Created by qlu on 2019-08-09.
//

#ifndef CTP_EXT_TRADE_MAPPER_H
#define CTP_EXT_TRADE_MAPPER_H

#include <SQLiteCpp/SQLiteCpp.h>
namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            struct CtpTrade
            {
                uint64_t trade_id;
                int64_t trade_time;

                const char trading_day[9];
                const char exchange_id[9];
                const char ctp_trade_id[21];
            };

            class TradeMapper
            {
            public:
                TradeMapper(const std::string &file_name) : db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
                {
                    create_table_if_not_exist();
                }

                void create_table_if_not_exist()
                {
                    std::string sql = "CREATE TABLE IF NOT EXISTS ctp_trade(\n"
                                      "                    trade_id        INTEGER PRIMARY KEY NOT NULL,\n"
                                      "                    trade_time      INTEGER,\n"
                                      "                    trading_day     CHAR(50),\n"
                                      "                    exchange_id     CHAR(50),\n"
                                      "                    ctp_trade_id    CHAR(50));";
                    try
                    {
                        db_.exec(sql);
                    }
                    catch (std::exception &e)
                    {
                        SPDLOG_ERROR("failed to create table, sql: {}, exception: {}", sql.c_str(), e.what());
                    }
                }

                void save(uint64_t trade_id, int64_t trade_time, const char* trading_day, const char* exchange_id, const char* ctp_trade_id)
                {
                    SQLite::Statement insert(db_, "INSERT INTO ctp_trade VALUES(?,?,?,?,?)");
                    try
                    {
                        insert.bind(1, (long long)trade_id);
                        insert.bind(2, (long long)trade_time);
                        insert.bind(3, trading_day);
                        insert.bind(4, exchange_id);
                        insert.bind(5, ctp_trade_id);
                        insert.exec();
                    }
                    catch (std::exception &e)
                    {
                        SPDLOG_ERROR("exception: {}", e.what());
                    }
                }

                CtpTrade get(const char* trading_day, const char* exchange_id, const char* trade_id)
                {
                    CtpTrade result = {};
                    SQLite::Statement query(db_, "SELECT * FROM ctp_trade WHERE trading_day = ? AND exchange_id = ? AND ctp_trade_id = ?");
                    try
                    {
                        query.bind(1, trading_day);
                        query.bind(2, exchange_id);
                        query.bind(3, trade_id);

                        if (query.executeStep())
                        {
                            result.trade_id = query.getColumn(0).getInt64();
                            result.trade_time = query.getColumn(1).getInt64();
                        }
                    }
                    catch (std::exception &e)
                    {
                        SPDLOG_ERROR("exception: {}", e.what());
                    }
                    return result;
                }
            private:
                SQLite::Database db_;
            };
        }

    }
}

#endif //CTP_EXT_TRADE_MAPPER_H
