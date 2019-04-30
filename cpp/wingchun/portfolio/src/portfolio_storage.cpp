//
// Created by PolarAir on 2019-04-18.
//

#include "portfolio_storage.h"
#include "storage_common.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    PortfolioStorage::PortfolioStorage(const char *db_file) : db_file_(db_file)
    {
        create_pos_tables(db_file);
        create_acc_tables(db_file);
        create_pnl_tables(db_file);
    }

    void PortfolioStorage::save(int64_t last_update, const std::string &trading_day, const kungfu::PortfolioInfo &pnl)
    {
        SQLite::Database db(db_file_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        try
        {
            db.exec("DELETE FROM portfolio");
            db.exec("DELETE FROM meta");

            db.exec(fmt::format("INSERT INTO portfolio("
                                "trading_day, update_time, initial_equity, static_equity, dynamic_equity, "
                                "accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio"
                                ") VALUES('{}',{},{},{},{},{},{},{},{})",
                                pnl.trading_day, pnl.update_time, pnl.initial_equity, pnl.static_equity, pnl.dynamic_equity,
                                pnl.accumulated_pnl, pnl.accumulated_pnl_ratio, pnl.intraday_pnl, pnl.intraday_pnl_ratio));

            db.exec(fmt::format("INSERT INTO meta(update_time, trading_day) VALUES({}, '{}')", last_update, trading_day));

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    void PortfolioStorage::save_meta(int64_t last_update, const std::string &trading_day)
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

    void PortfolioStorage::load(int64_t &last_update, std::string &trading_day, kungfu::PortfolioInfo &pnl,
            std::map<std::string, AccountManagerPtr> &stock_accounts)
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

            pnl = {};
            SQLite::Statement query_pnl(db, "SELECT * FROM portfolio");
            if (query_pnl.executeStep())
            {
                strcpy(pnl.trading_day, query_pnl.getColumn(0));
                pnl.update_time = query_pnl.getColumn(1);
                pnl.initial_equity = query_pnl.getColumn(2);
                pnl.static_equity = query_pnl.getColumn(3);
                pnl.dynamic_equity = query_pnl.getColumn(4);
                pnl.accumulated_pnl = query_pnl.getColumn(5);
                pnl.accumulated_pnl_ratio = query_pnl.getColumn(6);
                pnl.intraday_pnl = query_pnl.getColumn(7);
                pnl.intraday_pnl_ratio = query_pnl.getColumn(8);
            }

            stock_accounts.clear();
            SQLite::Statement query_count(db, "SELECT count(type) FROM sqlite_master where type = 'table' and name = 'account'");
            if (query_count.executeStep())
            {
                if (query_count.getColumn(0).getInt() == 1)
                {
                    SQLite::Statement query_acc(db, "SELECT account_id FROM account where type = ?");
                    query_acc.bind(1, std::string(1, AccountTypeStock));
                    while (query_acc.executeStep())
                    {
                        std::string account_id = query_acc.getColumn(0).getString();
                        stock_accounts[account_id] = std::make_shared<AccountManager>(account_id.c_str(), AccountTypeStock, db_file_.c_str());
                    }
                }
            }

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

}
