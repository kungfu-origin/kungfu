//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_ACCOUNT_MANAGER_HPP
#define KUNGFU_ACCOUNT_MANAGER_HPP

#include "../include/account_manager.h"
#include "../include/position_manager.h"
#include "account_storage.h"
#include "serialize.h"
#include "util/include/business_helper.h"
#include "commission/commission_manager.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>
#include <util/instrument/instrument.h>

namespace kungfu
{
    class AccountManager::impl final : public IPnLDataHandler
    {
    public:
        friend class AccountStorage;
        explicit impl(const char* account_id, AccountType type, const char* db);
        virtual ~impl();

        Position get_long_pos(const char* instrument_id, const char* exchange_id) const;
        Position get_short_pos(const char* instrument_id, const char* exchange_id) const;
        double get_last_price(const char* instrument_id, const char* exchange_id) const;
        std::vector<Instrument> get_all_pos_instruments() const;

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
        bool recalc_acc();
        void callback() const;
        void on_pos_callback(const Position& pos) const;

    private:
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        CommissionManager                               commission_;
        PositionManager                                 pos_manager_;
        AccountInfo                                     account_;
        std::map<std::string, double>                   bond_map_;
        std::map<int64_t, double>                       frozen_map_; // { order_id -> frozen_money }
        std::vector<AccountCallback>                    cbs_;
        std::vector<PositionCallback>                   pos_cbs_;
    };
}

#endif //KUNGFU_ACCOUNT_MANAGER_HPP
