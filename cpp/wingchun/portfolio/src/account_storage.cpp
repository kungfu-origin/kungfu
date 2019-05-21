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
    AccountStorage::AccountStorage(const char *account_id) : account_id_(account_id) {}

    bool AccountStorage::save(SQLite::Database &db, const kungfu::AccountManager *acc_manager, bool save_meta)
    {
        if (nullptr == acc_manager || account_id_ != acc_manager->impl_->account_.account_id)
            return false;

        try
        {
            const auto& account = acc_manager->impl_->account_;

            SQLite::Statement delete_account(db, "DELETE FROM account WHERE account_id = ? and source_id = ?");
            delete_account.bind(1, account_id_);
            delete_account.bind(2, std::string(account.source_id));
            delete_account.exec();

            SQLite::Statement delete_bond_expired(db, "DELETE FROM bond_expire where account_id = ?");
            delete_bond_expired.bind(1, account_id_);
            delete_bond_expired.exec();

            SQLite::Statement delete_acc_frozen(db, "DELETE FROM acc_frozen WHERE account_id = ?");
            delete_acc_frozen.bind(1, account_id_);

            SQLite::Statement delete_meta(db, "DELETE FROM meta");
            delete_meta.exec();

            SQLite::Statement save_account_info(db, "INSERT INTO account("
                                                        "rcv_time, update_time, trading_day, account_id, type, broker_id, source_id, "
                                                        "initial_equity, static_equity, dynamic_equity, accumulated_pnl, accumulated_pnl_ratio, "
                                                        "intraday_pnl, intraday_pnl_ratio, avail, market_value, margin, accumulated_fee, "
                                                        "intraday_fee, frozen_cash, frozen_margin, frozen_fee, position_pnl, close_pnl"
                                                        ") "
                                                        "VALUES(?, ?, ?, ?, ?,?, ?, ?,?, ?, ?, ?, ?,?,?,?,?,?,?,?,?, ?, ?, ?)");
            save_account_info.bind(1, account.rcv_time);
            save_account_info.bind(2, account.update_time);
            save_account_info.bind(3, account.trading_day);
            save_account_info.bind(4, account.account_id);
            save_account_info.bind(5, std::string(1, account.type));
            save_account_info.bind(6, account.broker_id);
            save_account_info.bind(7, account.source_id);
            save_account_info.bind(8, account.initial_equity);
            save_account_info.bind(9, account.static_equity);
            save_account_info.bind(10, account.dynamic_equity);
            save_account_info.bind(11, account.accumulated_pnl);
            save_account_info.bind(12, account.accumulated_pnl_ratio);
            save_account_info.bind(13, account.intraday_pnl);
            save_account_info.bind(14, account.intraday_pnl_ratio);
            save_account_info.bind(15, account.avail);
            save_account_info.bind(16, account.market_value);
            save_account_info.bind(17, account.margin);
            save_account_info.bind(18, account.accumulated_fee);
            save_account_info.bind(19, account.intraday_fee);
            save_account_info.bind(20, account.frozen_cash);
            save_account_info.bind(21, account.frozen_margin);
            save_account_info.bind(22, account.frozen_fee);
            save_account_info.bind(23, account.position_pnl);
            save_account_info.bind(24, account.close_pnl);
            save_account_info.exec();

            for (const auto& iter : acc_manager->impl_->bond_map_)
            {
                SQLite::Statement insert_bond_expired(db, "INSERT INTO bond_expire(date, account_id, amount) VALUES(?, ?, ?)");
                insert_bond_expired.bind(1, iter.first);
                insert_bond_expired.bind(2, account_id_);
                insert_bond_expired.bind(3, iter.second);
                insert_bond_expired.exec();
            }

            for (const auto& iter : acc_manager->impl_->frozen_map_)
            {
                SQLite::Statement insert_acc_frozen(db, "INSERT INTO acc_frozen(order_id, account_id, amount) VALUES(?, ?, ?)");
                insert_acc_frozen.bind(1, iter.first);
                insert_acc_frozen.bind(2, account_id_);
                insert_acc_frozen.bind(3, iter.second);
                insert_acc_frozen.exec();
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
