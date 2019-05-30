//
// Created by qlu on 2019/3/14.
//

#include <spdlog/spdlog.h>

#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/storage/trade_storage.h>

namespace kungfu
{
    namespace storage
    {
        TradeStorage::TradeStorage(const std::string &file_path) : db_(file_path.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        TradeStorage::~TradeStorage() {}
        
        void TradeStorage::create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS trade(\n"
                        "                    id              INTEGER PRIMARY KEY NOT NULL,\n"
                        "                    order_id        CHAR(50),\n"
                        "                    trade_time      INTEGER,\n"
                        "                    instrument_id   CHAR(50),\n"
                        "                    exchange_id     CHAR(50),\n"
                        "                    account_id      CHAR(50),\n"
                        "                    client_id       CHAR(50),\n"
                        "                    instrument_type CHAR(1),\n"
                        "                    side            CHAR(1),\n"
                        "                    offset          CHAR(1),\n"
                        "                    price           DOUBLE,\n"
                        "                    volume          INTEGER,\n"
                        "                    tax             DOUBLE,\n"
                        "                    commission      DOUBLE);";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to create table trades, exception: {}", e.what());
            }
        }

        long long TradeStorage::add_trade(const Trade &trade)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT INTO trade (order_id, trade_time, instrument_id, exchange_id, account_id, client_id, instrument_type, side, offset, price, volume, tax, commission)"
                        " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                insert.bind(1, std::to_string(trade.order_id));
                insert.bind(2, trade.trade_time);
                insert.bind(3, trade.instrument_id);
                insert.bind(4, trade.exchange_id);
                insert.bind(5, trade.account_id);
                insert.bind(6, trade.client_id);
                insert.bind(7, std::string(1, trade.instrument_type));
                insert.bind(8, std::string(1, trade.side));
                insert.bind(9, std::string(1, trade.offset));
                insert.bind(10, trade.price);
                insert.bind(11, trade.volume);
                insert.bind(12, trade.tax);
                insert.bind(13, trade.commission);
                insert.exec();
                return  db_.getLastInsertRowid();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to insert trade[{}], exception: {}\n", to_string(trade) ,e.what());
                return 0;
            }
        }
    }
}
