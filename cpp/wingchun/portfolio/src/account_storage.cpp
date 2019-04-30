//
// Created by PolarAir on 2019-04-17.
//

#include "account_storage.h"
#include "storage_common.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    AccountStorage::AccountStorage(const char *account_id, const char *db_file) : account_id_(account_id), db_file_(db_file)
    {
        create_pos_tables(db_file);
        create_acc_tables(db_file);
    }

    void AccountStorage::save(int64_t last_update, const std::string &trading_day, const kungfu::AccountInfo &account,
                              const std::map<std::string, double> &bond_map,
                              const std::map<int64_t, double> &frozen_map)
    {
        SQLite::Database db(db_file_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec(fmt::format("DELETE FROM account WHERE account_id = '{}' and type = '{}' and source_id = '{}'",
                    account_id_, account.type, account.source_id));
            db.exec(fmt::format("DELETE FROM bond_expire where account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM acc_frozen WHERE account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM meta"));

            db.exec(fmt::format("INSERT INTO account("
                                "rcv_time, update_time, trading_day, account_id, type, broker_id, source_id, "
                                "initial_equity, static_equity, dynamic_equity, accumulated_pnl, accumulated_pnl_ratio, "
                                "intraday_pnl, intraday_pnl_ratio, avail, market_value, margin, accumulated_fee, "
                                "intraday_fee, frozen_cash, frozen_margin, frozen_fee, position_pnl, close_pnl"
                                ") "
                                "VALUES({},{},'{}','{}','{}','{}','{}',{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{})",
                                account.rcv_time, account.update_time, account.trading_day, account.account_id, account.type,
                                account.broker_id, account.source_id, account.initial_equity, account.static_equity,
                                account.dynamic_equity, account.accumulated_pnl, account.accumulated_pnl_ratio,
                                account.intraday_pnl, account.intraday_pnl_ratio, account.avail, account.market_value,
                                account.margin, account.accumulated_fee, account.intraday_fee, account.frozen_cash,
                                account.frozen_margin, account.frozen_fee, account.position_pnl, account.close_pnl));

            for (const auto& iter : bond_map)
            {
                db.exec(fmt::format("INSERT INTO bond_expire(date, account_id, amount) VALUES('{}', '{}', {})",
                        iter.first, account_id_, iter.second));
            }

            for (const auto& iter : frozen_map)
            {
                db.exec(fmt::format("INSERT INTO acc_frozen(order_id, account_id, amount) VALUES({}, '{}', {})",
                        iter.first, account_id_, iter.second));
            }

            db.exec(fmt::format("INSERT INTO meta(update_time, trading_day) VALUES({}, '{}')", last_update, trading_day));

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    void AccountStorage::save_meta(int64_t last_update, const std::string &trading_day)
    {
        SQLite::Database db(db_file_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec(fmt::format("DELETE FROM meta"));
            db.exec(fmt::format("INSERT INTO meta(update_time, trading_day) VALUES({}, '{}')", last_update, trading_day));

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    void AccountStorage::load(int64_t &last_update, std::string &trading_day, kungfu::AccountInfo &account,
                              std::map<std::string, double> &bond_map, std::map<int64_t, double> &frozen_map)
    {
        SQLite::Database db(db_file_, SQLite::OPEN_READONLY);
        db.exec("BEGIN");
        try
        {
            SQLite::Statement query_meta(db, "SELECT * FROM meta");
            if (query_meta.executeStep())
            {
                last_update = query_meta.getColumn(0);
                trading_day = query_meta.getColumn(1).getString();
            }
            account = {};
            strcpy(account.account_id, account_id_.c_str());
            SQLite::Statement query_account(db, "SELECT * FROM account WHERE account_id = ?");
            query_account.bind(1, account_id_);
            if (query_account.executeStep())
            {
                account.rcv_time = query_account.getColumn(0);
                account.update_time = query_account.getColumn(1);
                strcpy(account.trading_day, query_account.getColumn(2));
                strcpy(account.account_id, query_account.getColumn(3));
                account.type = query_account.getColumn(4)[0];
                strcpy(account.broker_id, query_account.getColumn(5));
                strcpy(account.source_id, query_account.getColumn(6));
                account.initial_equity = query_account.getColumn(7);
                account.static_equity = query_account.getColumn(8);
                account.dynamic_equity = query_account.getColumn(9);
                account.accumulated_pnl = query_account.getColumn(10);
                account.accumulated_pnl_ratio = query_account.getColumn(11);
                account.intraday_pnl = query_account.getColumn(12);
                account.intraday_pnl_ratio = query_account.getColumn(13);
                account.avail = query_account.getColumn(14);
                account.market_value = query_account.getColumn(15);
                account.margin = query_account.getColumn(16);
                account.accumulated_fee = query_account.getColumn(17);
                account.intraday_fee = query_account.getColumn(18);
                account.frozen_cash = query_account.getColumn(19);
                account.frozen_margin = query_account.getColumn(20);
                account.frozen_fee = query_account.getColumn(21);
                account.position_pnl = query_account.getColumn(22);
                account.close_pnl = query_account.getColumn(23);
            }

            bond_map.clear();
            SQLite::Statement query_bond(db, "SELECT date, amount FROM bond_expire WHERE account_id = ?");
            query_bond.bind(1, account_id_);
            while (query_bond.executeStep())
            {
                bond_map[query_bond.getColumn(0).getString()] = query_bond.getColumn(1);
            }

            frozen_map.clear();
            SQLite::Statement query_frozen(db, "SELECT order_id, amount FROM acc_frozen WHERE account_id = ?");
            query_frozen.bind(1, account_id_);
            while (query_frozen.executeStep())
            {
                frozen_map[query_frozen.getColumn(0)] = query_frozen.getColumn(1);
            }

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }
}
