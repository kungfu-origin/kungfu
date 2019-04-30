//
// Created by PolarAir on 2019-04-18.
//

#include "portfolio_manager.hpp"

namespace kungfu
{
    PortfolioManager::PortfolioManager(const char *db) : impl_(new impl(db))
    {

    }

    PortfolioManager::~PortfolioManager()
    {
        if (nullptr != impl_)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

#define IMPLEMENT_IMPL_POS_FUNC(ret_type, func_name) \
    ret_type PortfolioManager::func_name(const char *instrument_id, const char *exchange_id) const \
    { \
        return impl_->func_name(instrument_id, exchange_id); \
    }

    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_tot)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_tot_avail)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_tot_fro)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_yd)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_yd_avail)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_long_yd_fro)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_realized_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_unrealized_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_open_price)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_cost_price)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_margin)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_position_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_long_close_pnl)
    IMPLEMENT_IMPL_POS_FUNC(Position, get_long_pos)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_tot)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_tot_avail)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_tot_fro)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_yd)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_yd_avail)
    IMPLEMENT_IMPL_POS_FUNC(int64_t, get_short_yd_fro)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_realized_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_unrealized_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_open_price)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_cost_price)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_margin)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_position_pnl)
    IMPLEMENT_IMPL_POS_FUNC(double, get_short_close_pnl)
    IMPLEMENT_IMPL_POS_FUNC(Position, get_short_pos)
    IMPLEMENT_IMPL_POS_FUNC(double, get_last_price)

#define IMPLEMENT_IMPL_ACC_FUNC(ret_type, func_name) \
    ret_type PortfolioManager::func_name() const \
    { \
       return impl_->func_name(); \
    }

    IMPLEMENT_IMPL_ACC_FUNC(std::vector<Instrument>, get_all_pos_instruments)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_initial_equity)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_static_equity)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_dynamic_equity)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_accumulated_pnl)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_accumulated_pnl_ratio)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_intraday_pnl)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_intraday_pnl_ratio)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_avail)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_market_value)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_margin)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_accumulated_fee)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_intraday_fee)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_frozen_cash)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_frozen_margin)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_frozen_fee)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_position_pnl)
    IMPLEMENT_IMPL_ACC_FUNC(double, get_close_pnl)

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

    const PortfolioInfo* PortfolioManager::get_pnl() const
    {
        return impl_->get_pnl();
    }

    const AccountManagerPtr PortfolioManager::get_account(const char *account_id) const
    {
        return impl_->get_account(account_id);
    }
}