//
// Created by PolarAir on 2019-04-18.
//

#include "portfolio_manager.hxx"
#include "portfolio_storage.h"
#include "storage_common.h"

namespace kungfu
{
    PortfolioManager::PortfolioManager(const char* name, const char *db)
    : impl_(new impl(db)), db_file_(db), storage_(new PortfolioStorage(name))
    {
        create_pnl_tables(db);
        create_acc_tables(db);
        create_pos_tables(db);
        SQLite::Database pnl_db(db, SQLite::OPEN_READONLY);
        storage_->load(pnl_db, this);
    }

    PortfolioManager::~PortfolioManager()
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

    Position PortfolioManager::get_long_pos(const char *account_id, const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_pos(account_id, instrument_id, exchange_id);
    }

    Position PortfolioManager::get_short_pos(const char *account_id, const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_pos(account_id, instrument_id, exchange_id);
    }

    double PortfolioManager::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_last_price(instrument_id, exchange_id);
    }

    std::vector<Instrument> PortfolioManager::get_all_pos_instruments(const char *account_id) const
    {
        return impl_->get_all_pos_instruments(account_id);
    }

    SubPortfolioInfo PortfolioManager::get_sub_portfolio(const char *account_id) const
    {
        return impl_->get_sub_portfolio(account_id);
    }

    PortfolioInfo PortfolioManager::get_portfolio() const
    {
        return impl_->get_portfolio();
    }

    const AccountManagerPtr PortfolioManager::get_account(const char *account_id) const
    {
        return impl_->get_account(account_id);
    }

    void PortfolioManager::on_quote(const kungfu::Quote *quote)
    {
        impl_->on_quote(quote);
    }

    void PortfolioManager::on_order(const kungfu::Order *order)
    {
        impl_->on_order(order);
    }

    void PortfolioManager::on_trade(const kungfu::Trade *trade)
    {
        impl_->on_trade(trade);
    }

    void PortfolioManager::on_positions(const std::vector<kungfu::Position> &positions)
    {
        impl_->on_positions(positions);
    }

    void PortfolioManager::on_position_details(const std::vector<kungfu::Position> &details)
    {
        impl_->on_position_details(details);
    }

    void PortfolioManager::on_account(const kungfu::AccountInfo &account)
    {
        impl_->on_account(account);
    }

    void PortfolioManager::insert_order(const kungfu::OrderInput *input)
    {
        impl_->insert_order(input);
    }

    bool PortfolioManager::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        return impl_->freeze_algo_order(algo_id, frozen);
    }

    void PortfolioManager::release_algo_order(uint64_t algo_id)
    {
        impl_->release_algo_order(algo_id);
    }

    void PortfolioManager::switch_day(const std::string &trading_day)
    {
        impl_->switch_day(trading_day);
    }

    int64_t PortfolioManager::get_last_update() const
    {
        return impl_->get_last_update();
    }

    std::string PortfolioManager::get_current_trading_day() const
    {
        return impl_->get_current_trading_day();
    }

    void PortfolioManager::set_current_trading_day(const std::string &trading_day)
    {
        impl_->set_current_trading_day(trading_day);
    }

    void PortfolioManager::register_pos_callback(kungfu::PositionCallback cb)
    {
        impl_->register_pos_callback(std::move(cb));
    }

    void PortfolioManager::register_acc_callback(kungfu::AccountCallback cb)
    {
        impl_->register_acc_callback(std::move(cb));
    }

    void PortfolioManager::register_pnl_callback(kungfu::PnLCallback cb)
    {
        impl_->register_pnl_callback(std::move(cb));
    }

    void PortfolioManager::set_initial_equity(double equity)
    {
        impl_->set_initial_equity(equity);
    }

    void PortfolioManager::set_static_equity(double equity)
    {
        impl_->set_static_equity(equity);
    }

    void PortfolioManager::dump_to_db() const
    {
        SQLite::Database db(db_file_, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("BEGIN");
        db.exec(storage_->save(db, this) ? "COMMIT" : "ROLLBACK");
    }
}