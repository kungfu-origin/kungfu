//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_ACCOUNT_MANAGER_H
#define KUNGFU_ACCOUNT_MANAGER_H

#include "pnl_def.h"
#include <memory>

namespace kungfu
{
    class AccountManager final : public IPnLDataHandler, public IPosDataFetcher, public IAccDataFetcher
    {
    public:
        explicit AccountManager(const char* account_id, AccountType type, const char* db);
        virtual ~AccountManager();

        // IPosDataFetcher
        int64_t get_long_tot(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_long_tot_avail(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_long_tot_fro(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_long_yd(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_long_yd_avail(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_long_yd_fro(const char* instrument_id, const char* exchange_id) const override;
        double get_long_realized_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_long_unrealized_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_long_open_price(const char* instrument_id, const char* exchange_id) const override;
        double get_long_cost_price(const char* instrument_id, const char* exchange_id) const override;
        double get_long_margin(const char* instrument_id, const char* exchange_id) const override;
        double get_long_position_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_long_close_pnl(const char* instrument_id, const char* exchange_id) const override;
        Position get_long_pos(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_tot(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_tot_avail(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_tot_fro(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_yd(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_yd_avail(const char* instrument_id, const char* exchange_id) const override;
        int64_t get_short_yd_fro(const char* instrument_id, const char* exchange_id) const override;
        double get_short_realized_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_short_unrealized_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_short_open_price(const char* instrument_id, const char* exchange_id) const override;
        double get_short_cost_price(const char* instrument_id, const char* exchange_id) const override;
        double get_short_margin(const char* instrument_id, const char* exchange_id) const override;
        double get_short_position_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_short_close_pnl(const char* instrument_id, const char* exchange_id) const override;
        Position get_short_pos(const char* instrument_id, const char* exchange_id) const override;
        double get_last_price(const char* instrument_id, const char* exchange_id) const override;
        std::vector<Instrument> get_all_pos_instruments() const override;
        // IPosDataFetcher

        // IAccDataFetcher
        double get_initial_equity() const override;
        double get_static_equity() const override;
        double get_dynamic_equity() const override;
        double get_accumulated_pnl() const override;
        double get_accumulated_pnl_ratio() const override;
        double get_intraday_pnl() const override;
        double get_intraday_pnl_ratio() const override;
        double get_avail() const override;
        double get_market_value() const override;
        double get_margin() const override;
        double get_accumulated_fee() const override;
        double get_intraday_fee() const override;
        double get_frozen_cash() const override;
        double get_frozen_margin() const override;
        double get_frozen_fee() const override;
        double get_position_pnl() const override;
        double get_close_pnl() const override;
        // IAccDataFetcher

        // IPnLDataHandler
        void on_quote(const Quote* quote) override;
        void on_order(const Order* order) override;
        void on_trade(const Trade* trade) override;
        void on_positions(const std::vector<Position>& positions) override;
        void on_position_details(const std::vector<Position>& details) override;
        void on_account(const AccountInfo& account) override;
        void insert_order(const OrderInput* input) override;
        bool freeze_algo_order(uint64_t algo_id, const AssetsFrozen& frozen) override;
        void release_algo_order(uint64_t algo_id) override;
        void switch_day(const std::string& trading_day) override;
        int64_t get_last_update() const override;
        std::string get_current_trading_day() const override;
        void set_current_trading_day(const std::string& trading_day) override;
        void register_pos_callback(PositionCallback cb) override;
        void register_acc_callback(AccountCallback cb) override;
        void register_pnl_callback(PnLCallback cb) override;
        void set_initial_equity(double equity) override;
        void set_static_equity(double equity) override;
        // IPnLDataHandler

        double calc_commission(const Trade* trade) const;
        double calc_tax(const Trade* trade) const;
        AccountInfo get_account_info() const;

    private:
        class impl;
        impl* impl_;
    };
    typedef std::shared_ptr<AccountManager> AccountManagerPtr;
}

#endif //KUNGFU_ACCOUNT_MANAGER_H
