//
// Created by qlu on 2019/3/14.
//

#include <spdlog/spdlog.h>

#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/storage/order_storage.h>

namespace kungfu
{
    namespace storage
    {
        OrderStorage::OrderStorage(const std::string& file_path) : db_(file_path.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        OrderStorage::~OrderStorage() {}
        
        void OrderStorage::create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS orders(\n"
                        "                    order_id         CHAR(50) PRIMARY KEY NOT NULL,\n"
                        "                    rcv_time         INTEGER,\n"
                        "                    insert_time      INTEGER,\n"
                        "                    update_time      INTEGER,\n"
                        "                    trading_day      CHAR(50),\n"
                        "                    instrument_id    CHAR(50),\n"
                        "                    exchange_id      CHAR(50),\n"
                        "                    account_id       CHAR(50),\n"
                        "                    client_id        CHAR(50),\n"
                        "                    instrument_type  CHAR(1),\n"
                        "                    limit_price      DOUBLE,\n"
                        "                    frozen_price     DOUBLE,\n"
                        "                    volume           INTEGER,\n"
                        "                    volume_traded    INTEGER,\n"
                        "                    volume_left      INTEGER,\n"
                        "                    tax              DOUBLE,\n"
                        "                    commission       DOUBLE,\n"
                        "                    status           CHAR(1),\n"
                        "                    error_id         INTEGER,\n"
                        "                    error_msg        TEXT,\n"
                        "                    side             CHAR(1),\n"
                        "                    offset           CHAR(1),\n"
                        "                    price_type       CHAR(1),\n"
                        "                    volume_condition CHAR(1),\n"
                        "                    time_condition   CHAR(1),\n"
                        "                    parent_id        CHAR(50));";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to create table {}, exception: {}", "orders", e.what());
            }
        }

        void OrderStorage::add_order(uint64_t order_id, const journal::Order &order)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT OR REPLACE INTO orders VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                insert.bind(1, std::to_string(order.order_id));
                insert.bind(2, (long long)order.rcv_time);
                insert.bind(3, order.insert_time);
                insert.bind(4, order.update_time);
                insert.bind(5, order.trading_day);
                insert.bind(6, order.instrument_id);
                insert.bind(7, order.exchange_id);
                insert.bind(8, order.account_id);
                insert.bind(9, order.client_id);
                insert.bind(10, std::string(1, order.instrument_type));
                insert.bind(11, order.limit_price);
                insert.bind(12, order.frozen_price);
                insert.bind(13, order.volume);
                insert.bind(14, order.volume_traded);
                insert.bind(15, order.volume_left);
                insert.bind(16, order.tax);
                insert.bind(17, order.commission);
                insert.bind(18, std::string(1, order.status));
                insert.bind(19, order.error_id);
                insert.bind(20, order.error_msg);
                insert.bind(21, std::string(1, order.side));
                insert.bind(22, std::string(1, order.offset));
                insert.bind(23, std::string(1, order.price_type));
                insert.bind(24, std::string(1, order.volume_condition));
                insert.bind(25, std::string(1, order.time_condition));
                insert.bind(26, std::to_string(order.parent_id));
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to add order[{}], exception: {}", to_string(order), e.what());
            }
        }

        const journal::Order OrderStorage::get_order(uint64_t order_id)
        {
            journal::Order result = {};
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM orders WHERE order_id = ?");
                query.bind(1, std::to_string(order_id));
                if (query.executeStep())
                {
                    result.order_id =  std::stoll(query.getColumn(0).getString());
                    result.rcv_time = query.getColumn(1);
                    result.insert_time = query.getColumn(2);
                    result.update_time = query.getColumn(3);
                    strcpy(result.trading_day, query.getColumn(4));
                    strcpy(result.instrument_id, query.getColumn(5));
                    strcpy(result.exchange_id, query.getColumn(6));
                    strcpy(result.account_id, query.getColumn(7));
                    strcpy(result.client_id, query.getColumn(8));
                    result.instrument_type = query.getColumn(9)[0];
                    result.limit_price = query.getColumn(10);
                    result.frozen_price = query.getColumn(11);
                    result.volume = query.getColumn(12);
                    result.volume_traded = query.getColumn(13);
                    result.volume_left = query.getColumn(14);
                    result.tax = query.getColumn(15);
                    result.commission = query.getColumn(16);
                    result.status = query.getColumn(17)[0];
                    result.error_id = query.getColumn(18);
                    strcpy(result.error_msg, query.getColumn(19));
                    result.side = query.getColumn(20)[0];
                    result.offset = query.getColumn(21)[0];
                    result.price_type = query.getColumn(22)[0];
                    result.volume_condition = query.getColumn(23)[0];
                    result.time_condition = query.getColumn(24)[0];
                    result.parent_id = std::stoll(query.getColumn(25).getString());
                }
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to query order[{}], exception: {}", order_id, e.what());
            }
            return result;
        }
    }
}
