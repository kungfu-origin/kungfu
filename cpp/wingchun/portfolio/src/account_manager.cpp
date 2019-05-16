//
// Created by PolarAir on 2019-04-17.
//

#include "account_manager.hxx"
#include "account_storage.h"
#include "storage_common.h"

namespace kungfu
{
    AccountManager::AccountManager(const char *account_id, AccountType type, const char *db)
    : impl_(new impl(account_id, type, db)), db_file_(db), storage_(new AccountStorage(account_id))
    {
        create_acc_tables(db);
        create_pos_tables(db);
        SQLite::Database acc_db(db, SQLite::OPEN_READONLY);
        storage_->load(acc_db, this);
    }

    AccountManager::~AccountManager()
    {
        if (nullptr != storage_)
        {
            delete storage_;
            storage_ = nullptr;
        }
        if (nullptr != impl_)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

    Position AccountManager::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_pos(instrument_id, exchange_id);
    }

    Position AccountManager::get_short_pos(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_pos(instrument_id, exchange_id);
    }

    double AccountManager::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_last_price(instrument_id, exchange_id);
    }

    std::vector<Instrument> AccountManager::get_all_pos_instruments() const
    {
        return impl_->get_all_pos_instruments();
    }

    void AccountManager::on_quote(const kungfu::Quote *quote)
    {
        impl_->on_quote(quote);
    }

    void AccountManager::on_order(const kungfu::Order *order)
    {
        impl_->on_order(order);
    }

    void AccountManager::on_trade(const kungfu::Trade *trade)
    {
        impl_->on_trade(trade);
    }

    void AccountManager::on_positions(const vector<kungfu::Position> &positions)
    {
        impl_->on_positions(positions);
    }

    void AccountManager::on_position_details(const vector<kungfu::Position> &details)
    {
        impl_->on_position_details(details);
    }

    void AccountManager::on_account(const kungfu::AccountInfo &account)
    {
        impl_->on_account(account);
    }

    void AccountManager::insert_order(const kungfu::OrderInput *input)
    {
        impl_->insert_order(input);
    }

    bool AccountManager::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        return impl_->freeze_algo_order(algo_id, frozen);
    }

    void AccountManager::release_algo_order(uint64_t algo_id)
    {
        impl_->release_algo_order(algo_id);
    }

    void AccountManager::switch_day(const std::string &trading_day)
    {
        impl_->switch_day(trading_day);
    }

    int64_t AccountManager::get_last_update() const
    {
        return impl_->get_last_update();
    }

    std::string AccountManager::get_current_trading_day() const
    {
        return impl_->get_current_trading_day();
    }

    void AccountManager::set_current_trading_day(const std::string &trading_day)
    {
        impl_->set_current_trading_day(trading_day);
    }

    void AccountManager::register_pos_callback(kungfu::PositionCallback cb)
    {
        impl_->register_pos_callback(std::move(cb));
    }

    void AccountManager::register_acc_callback(kungfu::AccountCallback cb)
    {
        impl_->register_acc_callback(std::move(cb));
    }

    void AccountManager::register_pnl_callback(kungfu::PnLCallback cb)
    {
        impl_->register_pnl_callback(std::move(cb));
    }

    void AccountManager::set_initial_equity(double equity)
    {
        impl_->set_initial_equity(equity);
    }

    void AccountManager::set_static_equity(double equity)
    {
        impl_->set_static_equity(equity);
    }

    double AccountManager::calc_commission(const kungfu::Trade *trade) const
    {
        return impl_->calc_commission(trade);
    }

    double AccountManager::calc_tax(const kungfu::Trade *trade) const
    {
        return impl_->calc_tax(trade);
    }

    AccountInfo AccountManager::get_account_info() const
    {
        return impl_->get_account_info();
    }

    void AccountManager::dump_to_db(SQLite::Database *db, bool save_meta) const
    {
        if (nullptr != storage_)
        {
            bool new_db = nullptr == db;
            if (new_db)
            {
                db = new SQLite::Database(db_file_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
                db->exec("BEGIN");
            }
            bool ret = storage_->save(*db, this, save_meta);
            if (new_db)
            {
                db->exec(ret ? "COMMIT" : "ROLLBACK");
                delete db;
                db = nullptr;
            }
        }
    }
}