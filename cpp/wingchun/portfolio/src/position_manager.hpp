//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_POSITION_MANAGER_HPP
#define KUNGFU_POSITION_MANAGER_HPP

#include "../include/position_manager.h"
#include "util/include/business_helper.h"
#include "util/instrument/instrument.h"
#include "serialize.h"
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>

namespace kungfu
{
    class PositionManager::impl final : public IPnLDataHandler
    {
    public:
        friend class PositionStorage;
        impl(const char* account_id, const char* db);
        virtual ~impl();

        Position get_long_pos(const char* instrument_id, const char* exchange_id) const;
        Position get_short_pos(const char* instrument_id, const char* exchange_id) const;
        double get_last_price(const char* instrument_id, const char* exchange_id) const;
        std::vector<Instrument> get_all_pos_instruments() const;
        double get_market_value() const; // for stock only

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

    private:
        double choose_price(const Position& pos) const;
        bool recalc_pos_by_price(Position& pos);
        void callback(const Position& pos) const;
        void init_pos_from_input(const OrderInput* input);
        void init_pos_from_trade(const Trade* trade);
        void deal_bond_from_buy(const Trade* trade);
        void insert_detail_from_trade(const Trade* trade);
        void remove_detail_from_trade(const Trade* trade);
        void on_order_stock(const Order* order);
        void on_order_future(const Order* order);
        void on_trade_stock(const Trade* trade);
        void on_trade_future(const Trade* trade);

    private:
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        std::map<std::string, Position>                 long_pos_map_; // { symbol -> pos }
        std::map<std::string, Position>                 short_pos_map_; // { symbol -> pos }
        std::map<std::string, std::vector<Position>>    bond_map_;  // { expire_date -> [ bond_pos ] }
        std::map<std::string, std::vector<Position>>    long_detail_map_; // { symbol -> [ detail ] }
        std::map<std::string, std::vector<Position>>    short_detail_map_; // { symbol -> [ detail ] }
        std::map<int64_t, int64_t>                      frozen_map_;// { order_id -> frozen_vol }
        std::vector<PositionCallback>                   cbs_;
    };
}

#endif //KUNGFU_POSITION_MANAGER_HPP