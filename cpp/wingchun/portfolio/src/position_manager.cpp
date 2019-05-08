//
// Created by PolarAir on 2019-04-16.
//

#include "position_manager.hpp"

namespace kungfu
{
    PositionManager::PositionManager(const char* account_id, const char *db) : impl_(new impl(account_id, db))
    {

    }

    PositionManager::~PositionManager()
    {
        if (nullptr != impl_)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

    Position PositionManager::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_pos(instrument_id, exchange_id);
    }

    Position PositionManager::get_short_pos(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_pos(instrument_id, exchange_id);
    }

    double PositionManager::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_last_price(instrument_id, exchange_id);
    }

    std::vector<Instrument> PositionManager::get_all_pos_instruments() const
    {
        return impl_->get_all_pos_instruments();
    }

    double PositionManager::get_market_value() const
    {
        return impl_->get_market_value();
    }

    void PositionManager::on_quote(const kungfu::Quote *quote)
    {
        impl_->on_quote(quote);
    }

    void PositionManager::on_order(const kungfu::Order *order)
    {
        impl_->on_order(order);
    }

    void PositionManager::on_trade(const kungfu::Trade *trade)
    {
        impl_->on_trade(trade);
    }

    void PositionManager::on_positions(const vector<kungfu::Position> &positions)
    {
        impl_->on_positions(positions);
    }

    void PositionManager::on_position_details(const vector<kungfu::Position> &details)
    {
        impl_->on_position_details(details);
    }

    void PositionManager::on_account(const kungfu::AccountInfo &account)
    {
        impl_->on_account(account);
    }

    void PositionManager::insert_order(const kungfu::OrderInput *input)
    {
        impl_->insert_order(input);
    }

    bool PositionManager::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        return impl_->freeze_algo_order(algo_id, frozen);
    }

    void PositionManager::release_algo_order(uint64_t algo_id)
    {
        impl_->release_algo_order(algo_id);
    }

    void PositionManager::switch_day(const std::string &trading_day)
    {
        impl_->switch_day(trading_day);
    }

    int64_t PositionManager::get_last_update() const
    {
        return impl_->get_last_update();
    }

    std::string PositionManager::get_current_trading_day() const
    {
        return impl_->get_current_trading_day();
    }

    void PositionManager::set_current_trading_day(const std::string &trading_day)
    {
        impl_->set_current_trading_day(trading_day);
    }

    void PositionManager::register_pos_callback(kungfu::PositionCallback cb)
    {
        impl_->register_pos_callback(std::move(cb));
    }

    void PositionManager::register_acc_callback(kungfu::AccountCallback cb)
    {
        impl_->register_acc_callback(std::move(cb));
    }

    void PositionManager::register_pnl_callback(kungfu::PnLCallback cb)
    {
        impl_->register_pnl_callback(std::move(cb));
    }

    void PositionManager::set_initial_equity(double equity)
    {
        impl_->set_initial_equity(equity);
    }

    void PositionManager::set_static_equity(double equity)
    {
        impl_->set_static_equity(equity);
    }
}