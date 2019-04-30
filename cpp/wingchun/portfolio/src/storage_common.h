//
// Created by PolarAir on 2019-04-24.
//

#ifndef KUNGFU_STORAGE_COMMON_H
#define KUNGFU_STORAGE_COMMON_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    inline void create_pos_tables(const char *db_file)
    {
        static std::string pos_columns = "rcv_time INTEGER, "
                                         "update_time INTEGER, "
                                         "instrument_id CHAR(50), "
                                         "instrument_type CHAR(1), "
                                         "exchange_id CHAR(50), "
                                         "account_id CHAR(50), "
                                         "client_id CHAR(50), "
                                         "direction CHAR(1), "
                                         "volume INTEGER, "
                                         "yesterday_volume INTEGER, "
                                         "frozen_total INTEGER, "
                                         "frozen_yesterday INTEGER, "
                                         "last_price DOUBLE, "
                                         "open_price DOUBLE, "
                                         "cost_price DOUBLE, "
                                         "close_price DOUBLE, "
                                         "pre_close_price DOUBLE, "
                                         "settlement_price DOUBLE, "
                                         "pre_settlement_price DOUBLE, "
                                         "margin DOUBLE, "
                                         "position_pnl DOUBLE, "
                                         "close_pnl DOUBLE, "
                                         "realized_pnl DOUBLE, "
                                         "unrealized_pnl DOUBLE, "
                                         "open_date CHAR(50), "
                                         "expire_date CHAR(50)";
        SQLite::Database db(db_file, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec("CREATE TABLE IF NOT EXISTS position(" + pos_columns + ")");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS position_uid ON position("
                    "instrument_id, exchange_id, instrument_type, account_id, client_id, direction"
                    ")");
            db.exec("CREATE TABLE IF NOT EXISTS bond_position(" + pos_columns + ")"); // bond不需要key
            db.exec("CREATE TABLE IF NOT EXISTS pos_detail(" + pos_columns + ")"); // detail不需要key
            db.exec("CREATE TABLE IF NOT EXISTS pos_frozen(order_id INTEGER PRIMARY KEY, account_id CHAR(50), frozen_vol INTEGER)");
            db.exec("CREATE TABLE IF NOT EXISTS meta(update_time INTEGER, trading_day CHAR(50))");

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    inline void create_acc_tables(const char *db_file)
    {
        static std::string acc_columns = "rcv_time int, "
                                         "update_time int, "
                                         "trading_day char(50), "
                                         "account_id char(50), "
                                         "type char(1), "
                                         "broker_id char(50), "
                                         "source_id char(50), "
                                         "initial_equity double, "
                                         "static_equity double, "
                                         "dynamic_equity double, "
                                         "accumulated_pnl double, "
                                         "accumulated_pnl_ratio double, "
                                         "intraday_pnl double, "
                                         "intraday_pnl_ratio double, "
                                         "avail double, "
                                         "market_value double, "
                                         "margin double, "
                                         "accumulated_fee double, "
                                         "intraday_fee double, "
                                         "frozen_cash char(50), "
                                         "frozen_margin char(50), "
                                         "frozen_fee char(50), "
                                         "position_pnl char(50), "
                                         "close_pnl char(50)";
        SQLite::Database db(db_file, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec("CREATE TABLE IF NOT EXISTS account(" + acc_columns + ")");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS account_uid ON account(account_id, type, source_id)");
            db.exec("CREATE TABLE IF NOT EXISTS bond_expire(date char(50), account_id char(50), amount double)");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS bond_expire_uid ON bond_expire(date, account_id)");
            db.exec("CREATE TABLE IF NOT EXISTS acc_frozen(order_id int PRIMARY KEY, account_id char(50), amount double)");
            db.exec("CREATE TABLE IF NOT EXISTS meta(update_time int, trading_day char(50))");

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    inline void create_pnl_tables(const char *db_file)
    {
        SQLite::Database db(db_file, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec("CREATE TABLE IF NOT EXISTS portfolio("
                    "trading_day char(50), "
                    "update_time int, "
                    "initial_equity double, "
                    "static_equity double, "
                    "dynamic_equity double, "
                    "accumulated_pnl double, "
                    "accumulated_pnl_ratio double, "
                    "intraday_pnl double, "
                    "intraday_pnl_ratio double "
                    ")");
            db.exec("CREATE TABLE IF NOT EXISTS meta(update_time int, trading_day char(50))");

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }
}

#endif //KUNGFU_STORAGE_COMMON_H
