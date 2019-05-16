//
// Created by PolarAir on 2019-04-18.
//

#include "portfolio_storage.h"
#include "storage_common.h"
#include "portfolio_manager.hpp"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    PortfolioStorage::PortfolioStorage(const char *name) : name_(name)
    {

    }

    bool PortfolioStorage::save(SQLite::Database &db, const kungfu::PortfolioManager *pnl_manager)
    {
        try
        {
            const auto& pnl = pnl_manager->impl_->pnl_;
            db.exec("DELETE FROM portfolio");
            db.exec("DELETE FROM meta");

            db.exec(fmt::format("INSERT INTO portfolio("
                                "trading_day, update_time, initial_equity, static_equity, dynamic_equity, "
                                "accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio"
                                ") VALUES('{}',{},{},{},{},{},{},{},{})",
                                pnl.trading_day, pnl.update_time, pnl.initial_equity, pnl.static_equity, pnl.dynamic_equity,
                                pnl.accumulated_pnl, pnl.accumulated_pnl_ratio, pnl.intraday_pnl, pnl.intraday_pnl_ratio));

            for (const auto& acc_iter : pnl_manager->impl_->accounts_)
            {
                SPDLOG_WARN("pnl storage iter to {}", acc_iter.first);
                if (nullptr != acc_iter.second)
                {
                    SPDLOG_WARN("pnl storage about to dump {}", acc_iter.first);
                    acc_iter.second->dump_to_db(&db, false);
                }
            }

            SPDLOG_WARN("pnl storage about to save meta {}, {}", pnl_manager->impl_->last_update_, pnl_manager->impl_->trading_day_);
            save_meta_inner(db, pnl_manager->impl_->last_update_, pnl_manager->impl_->trading_day_);
            return true;
        } catch (std::exception& e)
        {
            SPDLOG_ERROR("failed to save pnl to db, {}", e.what());
            return false;
        }
    }

    void PortfolioStorage::load(SQLite::Database &db, kungfu::PortfolioManager *pnl_manager)
    {
        db.exec("BEGIN");
        try
        {
            SQLite::Statement query_meta(db, "SELECT * FROM meta");
            if (query_meta.executeStep())
            {
                pnl_manager->impl_->last_update_ = query_meta.getColumn(0);
                pnl_manager->impl_->trading_day_ = query_meta.getColumn(1).getString();
            }

            auto& pnl = pnl_manager->impl_->pnl_;
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

            pnl_manager->impl_->accounts_.clear();
            SQLite::Statement query_account(db, "SELECT account_id, type FROM account");
            while (query_account.executeStep())
            {
                std::string account_id = query_account.getColumn(0).getString();
                AccountType type = query_account.getColumn(1).getString()[0];
                pnl_manager->impl_->accounts_[account_id] = std::make_shared<AccountManager>(account_id.c_str(), type,
                        db.getFilename().c_str());
            }

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR("failed to load pnl from db, {}", e.what());
            db.exec("ROLLBACK");
        }
    }

}
