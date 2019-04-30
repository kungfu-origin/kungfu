//
// Created by PolarAir on 2019-04-17.
//

#include "account_manager.hpp"

namespace kungfu
{
    AccountManager::AccountManager(const char *account_id, AccountType type, const char *db) : impl_(new impl(account_id, type, db))
    {

    }

    AccountManager::~AccountManager()
    {
        if (nullptr != impl_)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

    int64_t AccountManager::get_long_tot(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_tot(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_long_tot_avail(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_tot_avail(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_long_tot_fro(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_tot_fro(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_long_yd(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_yd(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_long_yd_avail(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_yd_avail(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_long_yd_fro(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_yd_fro(instrument_id, exchange_id);
    }

    double AccountManager::get_long_realized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_realized_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_long_unrealized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_unrealized_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_long_open_price(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_open_price(instrument_id, exchange_id);
    }

    double AccountManager::get_long_cost_price(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_cost_price(instrument_id, exchange_id);
    }

    double AccountManager::get_long_margin(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_margin(instrument_id, exchange_id);
    }

    double AccountManager::get_long_position_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_position_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_long_close_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_close_pnl(instrument_id, exchange_id);
    }

    Position AccountManager::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_long_pos(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_tot(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_tot(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_tot_avail(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_tot_avail(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_tot_fro(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_tot_fro(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_yd(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_yd(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_yd_avail(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_yd_avail(instrument_id, exchange_id);
    }

    int64_t AccountManager::get_short_yd_fro(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_yd_fro(instrument_id, exchange_id);
    }

    double AccountManager::get_short_realized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_realized_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_short_unrealized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_unrealized_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_short_open_price(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_open_price(instrument_id, exchange_id);
    }

    double AccountManager::get_short_cost_price(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_cost_price(instrument_id, exchange_id);
    }

    double AccountManager::get_short_margin(const char *instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_margin(instrument_id, exchange_id);
    }

    double AccountManager::get_short_position_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_position_pnl(instrument_id, exchange_id);
    }

    double AccountManager::get_short_close_pnl(const char* instrument_id, const char *exchange_id) const
    {
        return impl_->get_short_close_pnl(instrument_id, exchange_id);
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

    double AccountManager::get_initial_equity() const
    {
        return impl_->get_initial_equity();
    }

    double AccountManager::get_static_equity() const
    {
        return impl_->get_static_equity();
    }

    double AccountManager::get_dynamic_equity() const
    {
        return impl_->get_dynamic_equity();
    }

    double AccountManager::get_accumulated_pnl() const
    {
        return impl_->get_accumulated_pnl();
    }

    double AccountManager::get_accumulated_pnl_ratio() const
    {
        return impl_->get_accumulated_pnl_ratio();
    }

    double AccountManager::get_intraday_pnl() const
    {
        return impl_->get_intraday_pnl();
    }

    double AccountManager::get_intraday_pnl_ratio() const
    {
        return impl_->get_intraday_pnl_ratio();
    }

    double AccountManager::get_avail() const
    {
        return impl_->get_avail();
    }

    double AccountManager::get_market_value() const
    {
        return impl_->get_market_value();
    }

    double AccountManager::get_margin() const
    {
        return impl_->get_margin();
    }

    double AccountManager::get_accumulated_fee() const
    {
        return impl_->get_accumulated_fee();
    }

    double AccountManager::get_intraday_fee() const
    {
        return impl_->get_intraday_fee();
    }

    double AccountManager::get_frozen_cash() const
    {
        return impl_->get_frozen_cash();
    }

    double AccountManager::get_frozen_margin() const
    {
        return impl_->get_frozen_margin();
    }

    double AccountManager::get_frozen_fee() const
    {
        return impl_->get_frozen_fee();
    }

    double AccountManager::get_position_pnl() const
    {
        return impl_->get_position_pnl();
    }

    double AccountManager::get_close_pnl() const
    {
        return impl_->get_close_pnl();
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
}