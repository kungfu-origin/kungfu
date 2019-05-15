//
// Created by PolarAir on 2019-04-17.
//

#include "account_storage.h"
#include "storage_common.h"
#include "account_manager.hpp"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    AccountStorage::AccountStorage(const char *account_id) : account_id_(account_id)
    {

    }

    bool AccountStorage::save(SQLite::Database &db, const kungfu::AccountManager *acc_manager, bool save_meta)
    {
        if (nullptr == acc_manager || account_id_ != acc_manager->impl_->account_.account_id)
            return false;

        try
        {
            const auto& account = acc_manager->impl_->account_;
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

            for (const auto& iter : acc_manager->impl_->bond_map_)
            {
                db.exec(fmt::format("INSERT INTO bond_expire(date, account_id, amount) VALUES('{}', '{}', {})",
                        iter.first, account_id_, iter.second));
            }

            for (const auto& iter : acc_manager->impl_->frozen_map_)
            {
                db.exec(fmt::format("INSERT INTO acc_frozen(order_id, account_id, amount) VALUES({}, '{}', {})",
                        iter.first, account_id_, iter.second));
            }

            acc_manager->impl_->pos_manager_.dump_to_db(&db, false);

            if (save_meta)
                save_meta_inner(db, acc_manager->impl_->last_update_, acc_manager->impl_->trading_day_);

            return true;
        } catch (std::exception& e)
        {
            SPDLOG_ERROR("failed to save account to db, {}", e.what());

            return false;
        }
    }

    void AccountStorage::load(SQLite::Database &db, kungfu::AccountManager *acc_manager)
    {
        db.exec("BEGIN");
        try
        {
            SQLite::Statement query_meta(db, "SELECT * FROM meta");
            if (query_meta.executeStep())
            {
                acc_manager->impl_->last_update_ = query_meta.getColumn(0);
                acc_manager->impl_->trading_day_ = query_meta.getColumn(1).getString();
            }

            auto& account = acc_manager->impl_->account_;
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

            acc_manager->impl_->bond_map_.clear();
            SQLite::Statement query_bond(db, "SELECT date, amount FROM bond_expire WHERE account_id = ?");
            query_bond.bind(1, account_id_);
            while (query_bond.executeStep())
            {
                acc_manager->impl_->bond_map_[query_bond.getColumn(0).getString()] = query_bond.getColumn(1);
            }

            acc_manager->impl_->frozen_map_.clear();
            SQLite::Statement query_frozen(db, "SELECT order_id, amount FROM acc_frozen WHERE account_id = ?");
            query_frozen.bind(1, account_id_);
            while (query_frozen.executeStep())
            {
                acc_manager->impl_->frozen_map_[query_frozen.getColumn(0)] = query_frozen.getColumn(1);
            }

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            db.exec("ROLLBACK");
            SPDLOG_ERROR("failed to load account from db, {}", e.what());
        }
    }
}
