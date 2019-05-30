//
// Created by qlu on 2019/1/14.
//

#ifndef WC_2_ORDER_MAPPER_H
#define WC_2_ORDER_MAPPER_H

#include <string>
#include <iostream>
#include <spdlog/spdlog.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    namespace ctp
    {
        struct CtpOrder
        {
            uint64_t internal_order_id;

            std::string broker_id;
            std::string investor_id;
            std::string instrument_id;

            std::string exchange_id;
            std::string order_sys_id;

            int front_id;
            int session_id;
            std::string order_ref;

            uint64_t parent_id;
            std::string client_id;

            uint64_t insert_time;
        };

        class OrderMapper
        {
        public:
            OrderMapper(const std::string &file_name) : db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
            {
                create_table_if_not_exist();
            }

            void create_table_if_not_exist()
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS ctp_order(\n"
                        "                    internal_order_id INTEGER PRIMARY KEY NOT NULL,\n"
                        "                    broker_id     CHAR(50), \n"
                        "                    investor_id   CHAR(50),\n"
                        "                    instrument    CHAR(50),\n"
                        "                    exchange_id   CHAR(50),\n"
                        "                    order_sys_id  CHAR(50),\n"
                        "                    front_id      INTEGER,\n"
                        "                    session_id    INTEGER,\n"
                        "                    order_ref     CHAR(50),\n"
                        "                    parent_id     INTEGER,\n"
                        "                    client_id     CHAR(50),\n"
                        "                    insert_time   INTEGER);";
                try
                {
                    db_.exec(sql);
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to create table, sql: {}, exception: {}", sql.c_str(), e.what());
                }
            }

            void add_ctp_order(uint64_t internal_order_id, const CtpOrder &order)
            {
                try
                {
                    SQLite::Statement insert(db_, "INSERT INTO ctp_order VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                    insert.bind(1, (long long)(order.internal_order_id));
                    insert.bind(2, order.broker_id);
                    insert.bind(3, order.investor_id);
                    insert.bind(4, order.instrument_id);
                    insert.bind(5, order.exchange_id);
                    insert.bind(6, order.order_sys_id);
                    insert.bind(7, order.front_id);
                    insert.bind(8, order.session_id);
                    insert.bind(9, order.order_ref);
                    insert.bind(10, (long long)order.parent_id);
                    insert.bind(11, order.client_id);
                    insert.bind(12, (long long)order.insert_time);
                    insert.exec();
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("exception: {}", e.what());
                }
            }

            void update_sys_id(int front_id, int session_id, const char* order_ref, const char* exchange_id, const char* order_sys_id)
            {
                try
                {
                    SQLite::Statement query(db_, "update ctp_order SET exchange_id = ?, order_sys_id = ? WHERE front_id = ? AND session_id = ? AND order_ref = ?");
                    query.bind(1, exchange_id);
                    query.bind(2, order_sys_id);
                    query.bind(3, front_id);
                    query.bind(4, session_id);
                    query.bind(5, order_ref);
                    query.exec();

                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("exception: {}",e.what());
                }
            }

            const CtpOrder get_order_info(uint64_t internal_order_id)
            {
                CtpOrder result = {};
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM ctp_order WHERE internal_order_id = ?");
                    query.bind(1, (long long)internal_order_id);
                    if (query.executeStep())
                    {
                        result.internal_order_id = query.getColumn(0).getInt64();
                        result.broker_id = query.getColumn(1).getString();
                        result.investor_id = query.getColumn(2).getString();
                        result.instrument_id = query.getColumn(3).getString();
                        result.exchange_id = query.getColumn(4).getString();
                        result.order_sys_id = query.getColumn(5).getString();
                        result.front_id = query.getColumn(6).getInt();
                        result.session_id = query.getColumn(7).getInt();
                        result.order_ref = query.getColumn(8).getString();
                        result.parent_id = query.getColumn(9).getInt64();
                        result.client_id = query.getColumn(10).getString();
                        result.insert_time = query.getColumn(11).getInt64();
                    }
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("exception: {}", e.what());
                }
                return result;
            }

            const uint64_t get_internal_id_by_sys_id(const char* exchange_id, const char* order_sys_id)
            {
                uint64_t result = 0;
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM ctp_order WHERE exchange_id = ? and order_sys_id = ?");
                    query.bind(1, exchange_id);
                    query.bind(2, order_sys_id);
                    if (query.executeStep())
                    {
                        result = (long long)(query.getColumn(0));
                    }
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("exception: {}", e.what());
                }
                return result;
            }

            const CtpOrder get_order_info_by_sys_id(const char* exchange_id, const char* order_sys_id)
            {
                CtpOrder result = {};
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM ctp_order WHERE exchange_id = ? and order_sys_id = ?");
                    query.bind(1, exchange_id);
                    query.bind(2, order_sys_id);
                    if (query.executeStep())
                    {
                        result.internal_order_id = query.getColumn(0).getInt64();
                        result.broker_id = query.getColumn(1).getString();
                        result.investor_id = query.getColumn(2).getString();
                        result.instrument_id = query.getColumn(3).getString();
                        result.exchange_id = query.getColumn(4).getString();
                        result.order_sys_id = query.getColumn(5).getString();
                        result.front_id = query.getColumn(6).getInt();
                        result.session_id = query.getColumn(7).getInt();
                        result.order_ref = query.getColumn(8).getString();
                        result.parent_id = query.getColumn(9).getInt64();
                        result.client_id = query.getColumn(10).getString();
                        result.insert_time = query.getColumn(11).getInt64();
                    }
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("exception: {}", e.what());
                }
                return result;
            }

            const CtpOrder get_order_info_by_order_ref(int front_id, int session_id, const char* order_ref)
            {
                CtpOrder result = {};
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM ctp_order WHERE front_id = ? and session_id = ? and order_ref = ?");
                    query.bind(1, front_id);
                    query.bind(2, session_id);
                    query.bind(3, order_ref);
                    if (query.executeStep())
                    {
                        result.internal_order_id = (long long)(query.getColumn(0));
                        result.broker_id = query.getColumn(1).getString();
                        result.investor_id = query.getColumn(2).getString();
                        result.instrument_id = query.getColumn(3).getString();
                        result.exchange_id = query.getColumn(4).getString();
                        result.order_sys_id = query.getColumn(5).getString();
                        result.front_id = query.getColumn(6).getInt();
                        result.session_id = query.getColumn(7).getInt();
                        result.order_ref = query.getColumn(8).getString();
                        result.parent_id = query.getColumn(9).getInt64();
                        result.client_id = query.getColumn(10).getString();
                        result.insert_time = query.getColumn(11).getInt64();
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
#endif //WC_2_ORDER_MAPPER_H
