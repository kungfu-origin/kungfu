//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_POSITION_MANAGER_HPP
#define KUNGFU_POSITION_MANAGER_HPP

#include "../include/position_manager.h"
#include "position_storage.h"
#include "util/include/business_helper.h"
#include "util/instrument/instrument.h"
#include "serialize.h"
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>

namespace kungfu
{
    class PositionManager::impl final : public IPnLDataHandler, public IPosDataFetcher
    {
    public:
        explicit impl(const char* account_id, const char* db);
        virtual ~impl();

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

        double get_market_value() const; // for stock only

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
        PositionStorage                                 storage_;
        std::map<std::string, Position>                 long_pos_map_; // { symbol -> pos }
        std::map<std::string, Position>                 short_pos_map_; // { symbol -> pos }
        std::map<std::string, std::vector<Position>>    bond_map_;  // { expire_date -> [ bond_pos ] }
        std::map<std::string, std::vector<Position>>    long_detail_map_; // { symbol -> [ detail ] }
        std::map<std::string, std::vector<Position>>    short_detail_map_; // { symbol -> [ detail ] }
        std::map<int64_t, int64_t>                      frozen_map_;// { order_id -> frozen_vol }
        std::vector<PositionCallback>                   cbs_;
    };

    // impl
    PositionManager::impl::impl(const char *account_id, const char *db) : last_update_(0), storage_(account_id, db)
    {
        storage_.load(last_update_, trading_day_, long_pos_map_, short_pos_map_, bond_map_, long_detail_map_, short_detail_map_, frozen_map_);
    }

    PositionManager::impl::~impl()
    {
        storage_.save(last_update_, trading_day_, long_pos_map_, short_pos_map_, bond_map_, long_detail_map_, short_detail_map_, frozen_map_);
    }

    int64_t PositionManager::impl::get_long_tot(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.volume;
    }

    int64_t PositionManager::impl::get_long_tot_avail(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : (iter->second.yesterday_volume - iter->second.frozen_total);
    }

    int64_t PositionManager::impl::get_long_tot_fro(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.frozen_total;
    }

    int64_t PositionManager::impl::get_long_yd(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.yesterday_volume;
    }

    int64_t PositionManager::impl::get_long_yd_avail(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.yesterday_volume - iter->second.frozen_yesterday;
    }

    int64_t PositionManager::impl::get_long_yd_fro(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.frozen_yesterday;
    }

    double PositionManager::impl::get_long_realized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.realized_pnl;
    }

    double PositionManager::impl::get_long_unrealized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.unrealized_pnl;
    }

    double PositionManager::impl::get_long_open_price(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.open_price;
    }

    double PositionManager::impl::get_long_cost_price(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.cost_price;
    }

    double PositionManager::impl::get_long_margin(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.margin;
    }

    double PositionManager::impl::get_long_position_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.position_pnl;
    }

    double PositionManager::impl::get_long_close_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        return iter == long_pos_map_.end() ? 0 : iter->second.close_pnl;
    }

    Position PositionManager::impl::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        if (iter != long_pos_map_.end())
        {
            return iter->second;
        }
        else
        {
            Position pos = {};
            strcpy(pos.instrument_id, instrument_id);
            strcpy(pos.exchange_id, exchange_id);
            pos.instrument_type = get_instrument_type(instrument_id, exchange_id);
            return pos;
        }
    }

    int64_t PositionManager::impl::get_short_tot(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.volume;
    }

    int64_t PositionManager::impl::get_short_tot_avail(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : (iter->second.volume - iter->second.frozen_total);
    }

    int64_t PositionManager::impl::get_short_tot_fro(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.frozen_total;
    }

    int64_t PositionManager::impl::get_short_yd(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.yesterday_volume;
    }

    int64_t PositionManager::impl::get_short_yd_avail(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : (iter->second.yesterday_volume - iter->second.frozen_yesterday);
    }

    int64_t PositionManager::impl::get_short_yd_fro(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.frozen_yesterday;
    }

    double PositionManager::impl::get_short_realized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.realized_pnl;
    }

    double PositionManager::impl::get_short_unrealized_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.unrealized_pnl;
    }

    double PositionManager::impl::get_short_open_price(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.open_price;
    }

    double PositionManager::impl::get_short_cost_price(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.cost_price;
    }

    double PositionManager::impl::get_short_margin(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.margin;
    }

    double PositionManager::impl::get_short_position_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.position_pnl;
    }

    double PositionManager::impl::get_short_close_pnl(const char* instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        return iter == short_pos_map_.end() ? 0 : iter->second.close_pnl;
    }

    Position PositionManager::impl::get_short_pos(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = short_pos_map_.find(key);
        if (iter != short_pos_map_.end())
        {
            return iter->second;
        }
        else
        {
            Position pos = {};
            strcpy(pos.instrument_id, instrument_id);
            strcpy(pos.exchange_id, exchange_id);
            pos.instrument_type = get_instrument_type(instrument_id, exchange_id);
            return pos;
        }
    }

    double PositionManager::impl::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        auto key = get_symbol(instrument_id, exchange_id);
        auto iter = long_pos_map_.find(key);
        if (iter != long_pos_map_.end())
        {
            return iter->second.last_price;
        }
        auto iter1 = short_pos_map_.find(key);
        if (iter1 != short_pos_map_.end())
        {
            return iter1->second.last_price;
        }
        return 0;
    }

    std::vector<Instrument> PositionManager::impl::get_all_pos_instruments() const
    {
        std::map<std::string, Instrument> m;
        Instrument ins = {};
        auto func = [&](const std::map<std::string, Position>& pos_map) {
            for (const auto& iter : pos_map)
            {
                auto key = get_symbol(iter.second.instrument_id, iter.second.exchange_id);
                if (m.find(key) == m.end())
                {
                    ins = {};
                    strcpy(ins.instrument_id, iter.second.instrument_id);
                    strcpy(ins.exchange_id, iter.second.exchange_id);
                    ins.instrument_type = iter.second.instrument_type;
                    m[key] = ins;
                }
            }
        };
        func(long_pos_map_);
        func(short_pos_map_);

        std::vector<Instrument> ret;
        for (const auto& iter : m)
        {
            ret.emplace_back(iter.second);
        }
        return ret;
    }

    void PositionManager::impl::on_quote(const kungfu::Quote *quote)
    {
        last_update_ = quote->rcv_time;
        auto key = get_symbol(quote->instrument_id, quote->exchange_id);
        auto func = [&](std::map<std::string, Position>& m) {
            auto long_iter = m.find(key);
            if (long_iter != m.end())
            {
                auto& pos = long_iter->second;
                if (!is_zero(quote->last_price))
                {
                    pos.last_price = quote->last_price;
                }
                if (!is_zero(quote->close_price))
                {
                    pos.close_price = quote->close_price;
                }
                if (!is_zero(quote->pre_close_price))
                {
                    pos.pre_close_price = quote->pre_close_price;
                }
                if (!is_zero(quote->settlement_price))
                {
                    pos.settlement_price = quote->settlement_price;
                }
                if (!is_zero(quote->pre_settlement_price))
                {
                    pos.pre_settlement_price = quote->pre_settlement_price;
                }
                if (recalc_pos_by_price(pos))
                {
                    callback(pos);
                }
            }
        };
        func(long_pos_map_);
        func(short_pos_map_);
    }

    void PositionManager::impl::on_order(const kungfu::Order *order)
    {
        last_update_ = order->rcv_time;
        if (!is_final_status(order->status) || frozen_map_.find(order->order_id) == frozen_map_.end())
        {
            return;
        }

        if (order->instrument_type == InstrumentTypeStock || order->instrument_type == InstrumentTypeBond)
        {
            on_order_stock(order);
        }
        else if (order->instrument_type == InstrumentTypeFuture)
        {
            on_order_future(order);
        }
    }

    void PositionManager::impl::on_trade(const kungfu::Trade *trade)
    {
        last_update_ = trade->rcv_time;
        if (trade->volume == 0)
        {
            return;
        }

        if (trade->instrument_type == InstrumentTypeStock || trade->instrument_type == InstrumentTypeBond)
        {
            on_trade_stock(trade);
        }
        else if (trade->instrument_type == InstrumentTypeFuture)
        {
            on_trade_future(trade);
        }

        // 成交时一定写一下数据库, 解决前端切换页面时读库问题
        storage_.save(last_update_, trading_day_, long_pos_map_, short_pos_map_, bond_map_, long_detail_map_, short_detail_map_, frozen_map_);
    }

    void PositionManager::impl::on_positions(const vector<kungfu::Position> &positions)
    {
        for (const auto& pos : positions)
        {
            auto key = get_symbol(pos.instrument_id, pos.exchange_id);
            auto& pos_map = pos.direction == DirectionLong ? long_pos_map_ : short_pos_map_;
            if (pos_map.find(key) == pos_map.end())
            {
                pos_map[key] = pos;
                pos_map[key].cost_price = choose_price(pos);
            }
            else
            {
                auto& cur_pos = pos_map[key];
                cur_pos.rcv_time = pos.rcv_time;
                cur_pos.update_time = pos.update_time;
                cur_pos.volume = pos.volume;
                cur_pos.yesterday_volume = pos.yesterday_volume;
                if (!is_zero(pos.last_price))
                {
                    cur_pos.last_price = pos.last_price;
                }
                if (!is_zero(pos.close_price))
                {
                    cur_pos.close_price = pos.close_price;
                }
                if (!is_zero(pos.pre_close_price))
                {
                    cur_pos.pre_close_price = pos.pre_close_price;
                }
                if (!is_zero(pos.settlement_price))
                {
                    cur_pos.settlement_price = pos.settlement_price;
                }
                if (!is_zero(pos.pre_settlement_price))
                {
                    cur_pos.pre_settlement_price = pos.pre_settlement_price;
                }
                if (recalc_pos_by_price(cur_pos))
                {
                    callback(cur_pos);
                }
            }
        }
        storage_.save(last_update_, trading_day_, long_pos_map_, short_pos_map_, bond_map_, long_detail_map_, short_detail_map_, frozen_map_);
    }

    void PositionManager::impl::on_position_details(const vector<kungfu::Position> &details)
    {
        // detail只能全刷, 因为无法判断前后
        long_detail_map_.clear();
        short_detail_map_.clear();
        for (const auto& detail : details)
        {
            auto key = get_symbol(detail.instrument_id, detail.exchange_id);
            auto& detail_map = detail.direction == DirectionLong ? long_detail_map_ : short_detail_map_;
            detail_map[key].emplace_back(detail);
        }
    }

    void PositionManager::impl::on_account(const kungfu::AccountInfo &account)
    {
        boost::ignore_unused(account);
    }

    void PositionManager::impl::insert_order(const kungfu::OrderInput *input)
    {
        // 不处理开仓
        if (is_open(input->instrument_type, input->side, input->offset) || input->volume <= 0)
        {
            return;
        }

        auto key = get_symbol(input->instrument_id, input->exchange_id);
        auto direction = input->instrument_type == InstrumentTypeFuture ? get_future_direction(input->side, input->offset) : DirectionLong;
        auto& pos_map = direction == DirectionLong ? long_pos_map_ : short_pos_map_;

        if (pos_map.find(key) == pos_map.end())
        {
            init_pos_from_input(input);
        }
        auto& pos = pos_map[key];

        bool check_pos = false;
        if (input->instrument_type == InstrumentTypeStock || input->instrument_type == InstrumentTypeBond)
        {
            check_pos = (pos.yesterday_volume - pos.frozen_yesterday) >= input->volume;
            pos.frozen_yesterday += input->volume;
        }
        else if (input->instrument_type == InstrumentTypeFuture)
        {
            if (strcmp(input->exchange_id, EXCHANGE_SHFE) == 0)
            {
                check_pos = (input->offset == OffsetCloseToday ?
                        ((pos.volume - pos.frozen_total) - (pos.yesterday_volume - pos.frozen_yesterday)) :
                        pos.yesterday_volume) >= input->volume;
            }
            else
            {
                check_pos = (pos.volume - pos.frozen_yesterday) >= input->volume;
                pos.frozen_yesterday += input->volume;
            }
        }
        pos.frozen_total += input->volume;
        frozen_map_[input->order_id] = input->volume;

        if (!check_pos)
        {
            SPDLOG_WARN("not enough available positions");
        }

        callback(pos);
    }

    bool PositionManager::impl::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        boost::ignore_unused(algo_id, frozen);
        return false;
    }

    void PositionManager::impl::release_algo_order(uint64_t algo_id)
    {
        boost::ignore_unused(algo_id);
    }

    void PositionManager::impl::switch_day(const std::string &trading_day)
    {
        if (trading_day <= trading_day_)
        {
            return;
        }

        trading_day_ = trading_day;

        // 撤销所有冻结
        frozen_map_.clear();
        auto pos_func = [&](std::map<std::string, Position>& pos_map) {
            for (auto& iter : pos_map)
            {
                // 撤销冻结
                iter.second.yesterday_volume = iter.second.volume;
                iter.second.frozen_total = 0;
                iter.second.frozen_yesterday = 0;

                // 期货结算
                if (iter.second.instrument_type == InstrumentTypeFuture)
                {
                    iter.second.realized_pnl += iter.second.position_pnl;
                    iter.second.position_pnl = 0;
                    iter.second.close_pnl = 0;
                    iter.second.unrealized_pnl = 0;
                    iter.second.cost_price = choose_price(iter.second);
                }

                // 换价格
                if (iter.second.instrument_type == InstrumentTypeStock || iter.second.instrument_type == InstrumentTypeBond)
                {
                    if (!is_zero(iter.second.close_price))
                    {
                        iter.second.last_price = iter.second.close_price;
                    }
                }
                else if (iter.second.instrument_type == InstrumentTypeFuture)
                {
                    if (!is_zero(iter.second.settlement_price))
                    {
                        iter.second.last_price = iter.second.settlement_price;
                    }
                }
                iter.second.pre_close_price = iter.second.close_price;
                iter.second.close_price = 0;
                iter.second.pre_settlement_price = iter.second.settlement_price;
                iter.second.settlement_price = 0;
            }
        };
        pos_func(long_pos_map_);
        pos_func(short_pos_map_);

        // 到期债券
        for (auto iter = bond_map_.begin(); iter != bond_map_.end();)
        {
            if (iter->first <= trading_day_)
            {
                for (const auto& bond : iter->second)
                {
                    auto bond_key = get_symbol(bond.instrument_id, bond.exchange_id);
                    if (long_pos_map_.find(bond_key) != long_pos_map_.end())
                    {
                        auto profit = bond.volume * bond.open_price / 100.0 *
                                      get_reverse_repurchase_expire_days(bond.instrument_id) / 365;
                        auto& pos = long_pos_map_[bond_key];
                        pos.volume -= bond.volume;
                        pos.yesterday_volume -= bond.volume;
                        pos.realized_pnl += profit;
                        pos.unrealized_pnl -= profit;
                    }
                }
                iter = bond_map_.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        // 重算pnl
        auto calc_func = [&](std::map<std::string, Position>& pos_map) {
            for (auto& iter : pos_map)
            {
                recalc_pos_by_price(iter.second);
                callback(iter.second);
            }
        };
        calc_func(long_pos_map_);
        calc_func(short_pos_map_);

        storage_.save(last_update_, trading_day_, long_pos_map_, short_pos_map_, bond_map_, long_detail_map_, short_detail_map_, frozen_map_);
    }

    int64_t PositionManager::impl::get_last_update() const
    {
        return last_update_;
    }

    std::string PositionManager::impl::get_current_trading_day() const
    {
        return trading_day_;
    }

    void PositionManager::impl::set_current_trading_day(const std::string &trading_day)
    {
        if (trading_day >= trading_day_)
        {
            if (!trading_day_.empty())
            {
                switch_day(trading_day);
            }
            else
            {
                trading_day_ = trading_day;
            }
        }
    }

    void PositionManager::impl::register_pos_callback(kungfu::PositionCallback cb)
    {
        cbs_.emplace_back(std::move(cb));
    }

    void PositionManager::impl::register_acc_callback(kungfu::AccountCallback cb)
    {
        boost::ignore_unused(cb);
    }

    void PositionManager::impl::register_pnl_callback(kungfu::PnLCallback cb)
    {
        boost::ignore_unused(cb);
    }

    void PositionManager::impl::set_initial_equity(double equity)
    {
        boost::ignore_unused(equity);
    }

    void PositionManager::impl::set_static_equity(double equity)
    {
        boost::ignore_unused(equity);
    }

    double PositionManager::impl::get_market_value() const
    {
        double market_value = 0.0;
        for (const auto& iter : long_pos_map_)
        {
            if (iter.second.instrument_type == InstrumentTypeStock || iter.second.instrument_type == InstrumentTypeBond)
            {
                double price = choose_price(iter.second);
                if (is_reverse_repurchase(iter.second.instrument_id, iter.second.exchange_id))
                {
                    market_value += iter.second.volume;
                }
                else
                {
                    market_value += iter.second.volume * price;
                }
            }
        }
        return market_value;
    }

    double PositionManager::impl::choose_price(const kungfu::Position &pos) const
    {
        if (pos.instrument_type == InstrumentTypeStock || pos.instrument_type == InstrumentTypeBond)
        {
            if (!is_zero(pos.close_price))
            {
                return pos.close_price;
            }
            else if (!is_zero(pos.last_price))
            {
                return pos.last_price;
            }
            else
            {
                return is_zero(pos.pre_close_price) ? pos.last_price : pos.pre_close_price;
            }
        }
        else if (pos.instrument_type == InstrumentTypeFuture)
        {
            if (!is_zero(pos.settlement_price))
            {
                return pos.settlement_price;
            }
            else if (!is_zero(pos.last_price))
            {
                return pos.last_price;
            }
            else
            {
                return is_zero(pos.pre_settlement_price) ? pos.last_price : pos.pre_settlement_price;
            }
        }
        else
        {
            return pos.last_price;
        }
    }

    bool PositionManager::impl::recalc_pos_by_price(Position &pos)
    {
        if (is_zero(pos.cost_price))
        {
            // 账户初次创建并登陆td时查询回来的pos如果没有成本价的话，则用现价对其赋值，忽略其被加入功夫系统前的pnl
            // 这样计算不会太离谱
            pos.cost_price = pos.last_price;
        }
        int contract_multiplier = 1;
        double margin_ratio = 1;
        if (pos.instrument_type == InstrumentTypeFuture)
        {
            auto* instrument = InstrumentManager::get_instrument_manager()->get_future_instrument(pos.instrument_id, pos.exchange_id);
            contract_multiplier = instrument->contract_multiplier;
            margin_ratio = pos.direction == DirectionLong ? instrument->long_margin_ratio : instrument->short_margin_ratio;
        }

        auto old_unrealized = pos.unrealized_pnl;
        auto price = choose_price(pos);
        if (pos.instrument_type == InstrumentTypeFuture)
        {
            pos.margin = price * pos.volume * contract_multiplier * margin_ratio;
            pos.position_pnl = (price - pos.cost_price) * pos.volume * contract_multiplier;
        }
        pos.unrealized_pnl = (price - pos.cost_price) * pos.volume * contract_multiplier;
        auto changed = !is_equal(old_unrealized, pos.unrealized_pnl);
        return changed;
    }

    void PositionManager::impl::callback(const kungfu::Position &pos) const
    {
        for (const auto& cb : cbs_)
        {
            cb(pos);
        }
    }

    void PositionManager::impl::init_pos_from_input(const kungfu::OrderInput *input)
    {
        Position pos = {};
        pos.rcv_time = yijinjing::getNanoTime();
        pos.update_time = pos.rcv_time;
        strcpy(pos.instrument_id, input->instrument_id);
        pos.instrument_type = input->instrument_type;
        strcpy(pos.exchange_id, input->exchange_id);
        strcpy(pos.account_id, input->account_id);
        strcpy(pos.client_id, input->client_id);
        pos.direction = pos.instrument_type == InstrumentTypeFuture ? get_future_direction(input->side, input->offset) : DirectionLong;

        auto& pos_map = pos.direction == DirectionLong ? long_pos_map_ : short_pos_map_;
        pos_map[get_symbol(input->instrument_id, input->exchange_id)] = pos;
    }

    void PositionManager::impl::init_pos_from_trade(const kungfu::Trade *trade)
    {
        Position pos = {};
        pos.rcv_time = trade->rcv_time;
        pos.update_time = trade->rcv_time;
        strcpy(pos.instrument_id, trade->instrument_id);
        pos.instrument_type = trade->instrument_type;
        strcpy(pos.exchange_id, trade->exchange_id);
        strcpy(pos.account_id, trade->account_id);
        strcpy(pos.client_id, trade->client_id);
        pos.direction = pos.instrument_type == InstrumentTypeFuture ? get_future_direction(trade->side, trade->offset) : DirectionLong;

        auto& pos_map = pos.direction == DirectionLong ? long_pos_map_ : short_pos_map_;
        pos_map[get_symbol(trade->instrument_id, trade->exchange_id)] = pos;
    }

    void PositionManager::impl::deal_bond_from_buy(const kungfu::Trade *trade)
    {
        Position pos = {};
        pos.rcv_time = trade->rcv_time;
        pos.update_time = trade->rcv_time;
        strcpy(pos.instrument_id, trade->instrument_id);
        pos.instrument_type = trade->instrument_type;
        strcpy(pos.exchange_id, trade->exchange_id);
        strcpy(pos.account_id, trade->account_id);
        strcpy(pos.client_id, trade->client_id);
        pos.direction = DirectionLong;
        pos.volume = trade->volume;
        pos.open_price = trade->price;
        strcpy(pos.open_date, trading_day_.c_str());
        strcpy(pos.expire_date, get_reverse_repurchase_expire_date(trade->instrument_id, pos.open_date).c_str());

        bond_map_[pos.expire_date].emplace_back(pos);
    }

    void PositionManager::impl::insert_detail_from_trade(const kungfu::Trade *trade)
    {
        Position pos = {};
        pos.rcv_time = trade->rcv_time;
        pos.update_time = trade->rcv_time;
        strcpy(pos.instrument_id, trade->instrument_id);
        pos.instrument_type = trade->instrument_type;
        strcpy(pos.exchange_id, trade->exchange_id);
        strcpy(pos.account_id, trade->account_id);
        strcpy(pos.client_id, trade->client_id);
        pos.direction = get_future_direction(trade->side, trade->offset);
        pos.volume = trade->volume;
        pos.open_price = trade->price;
        strcpy(pos.open_date, trading_day_.c_str());
        pos.yesterday_volume =  strcmp(pos.open_date, trading_day_.c_str()) >= 0 ? 0 : trade->volume;

        auto& detail_map = pos.direction == DirectionLong ? long_detail_map_ : short_detail_map_;
        detail_map[get_symbol(trade->instrument_id, trade->exchange_id)].emplace_back(pos);
    }

    void PositionManager::impl::remove_detail_from_trade(const kungfu::Trade *trade)
    {
        auto direction = trade->instrument_type == InstrumentTypeFuture ? get_future_direction(trade->side, trade->offset) : DirectionLong;
        auto& detail_map = direction == DirectionLong ? long_detail_map_ : short_detail_map_;

        auto vol_left = trade->volume;
        bool today_only = strcmp(trade->exchange_id, EXCHANGE_SHFE) == 0 && trade->offset == OffsetCloseToday;
        auto key = get_symbol(trade->instrument_id, trade->exchange_id);
        if (detail_map.find(key) != detail_map.end())
        {
            auto& vec = detail_map[key];
            for (auto iter = vec.begin(); iter != vec.end(); )
            {
                if (today_only && trading_day_ > iter->open_date)
                {
                    ++iter;
                    continue;
                }

                if (iter->volume <= vol_left)
                {
                    vol_left -= iter->volume;
                    iter = vec.erase(iter);
                }
                else
                {
                    iter->volume -= vol_left;
                    if (!today_only)
                    {
                        iter->yesterday_volume = std::max<int64_t>(iter->yesterday_volume - vol_left, 0);
                    }
                    break;
                }
            }
        }
    }

    void PositionManager::impl::on_order_stock(const kungfu::Order *order)
    {
        auto key = get_symbol(order->instrument_id, order->exchange_id);
        if (long_pos_map_.find(key) != long_pos_map_.end())
        {
            auto fro = frozen_map_[order->order_id];
            frozen_map_.erase(order->order_id);
            auto& pos = long_pos_map_[key];
            pos.frozen_total -= fro;
            pos.frozen_yesterday -= fro;
            pos.update_time = order->rcv_time;
            recalc_pos_by_price(pos);
            callback(pos);
        }
    }

    void PositionManager::impl::on_order_future(const kungfu::Order *order)
    {
        auto key = get_symbol(order->instrument_id, order->exchange_id);
        auto& pos_map = get_future_direction(order->side, order->offset) == DirectionLong ? long_pos_map_ : short_pos_map_;
        if (pos_map.find(key) != pos_map.end())
        {
            auto fro = frozen_map_[order->order_id];
            frozen_map_.erase(order->order_id);
            auto& pos = pos_map[key];
            if (!(order->offset == OffsetCloseToday && strcmp(order->exchange_id, EXCHANGE_SHFE) == 0))
            {
                pos.frozen_yesterday = std::max<int64_t>(pos.frozen_yesterday - fro, 0);
            }
            pos.frozen_total -= fro;
            pos.update_time = order->rcv_time;
            recalc_pos_by_price(pos);
            callback(pos);
        }
    }

    void PositionManager::impl::on_trade_stock(const kungfu::Trade *trade)
    {
        auto key = get_symbol(trade->instrument_id, trade->exchange_id);
        if (frozen_map_.find(trade->order_id) != frozen_map_.end())
        {
            frozen_map_[trade->order_id] -= trade->volume;
            long_pos_map_[key].frozen_total -= trade->volume;
            long_pos_map_[key].frozen_yesterday -= trade->volume;
        }

        if (long_pos_map_.find(key) == long_pos_map_.end())
        {
            init_pos_from_trade(trade);
        }
        auto& pos = long_pos_map_[key];

        pos.update_time = trade->rcv_time;
        pos.last_price = trade->price;
        if (trade->side == SideBuy)
        {
            auto buy_cost = trade->price * trade->volume + trade->tax + trade->commission;
            pos.cost_price = pos.volume == 0 ?
                             (buy_cost - pos.realized_pnl) / trade->volume :
                             (pos.cost_price * pos.volume + buy_cost) / (pos.volume + trade->volume);
            pos.volume += trade->volume;
            if (is_reverse_repurchase(trade->instrument_id, trade->exchange_id))
            {
                auto profit = trade->volume * trade->price / 100.0 *
                              get_reverse_repurchase_expire_days(trade->instrument_id) / 360;
                pos.unrealized_pnl += profit;
            }
        }
        else
        {
            if (pos.yesterday_volume < trade->volume)
            {
                SPDLOG_WARN("Not enough available positions");
            }
            auto sell_gain = trade->price * trade->volume - trade->tax - trade->commission;
            pos.realized_pnl += sell_gain - pos.cost_price * trade->volume;
            pos.cost_price = pos.volume == trade->volume ?
                             0 :
                             (pos.cost_price * pos.volume - sell_gain) / (pos.volume - trade->volume);
            pos.volume -= trade->volume;
            pos.yesterday_volume -= trade->volume;
        }

        if (is_reverse_repurchase(trade->instrument_id, trade->exchange_id))
        {
            deal_bond_from_buy(trade);
        }

        recalc_pos_by_price(pos);
        callback(pos);
    }

    void PositionManager::impl::on_trade_future(const kungfu::Trade *trade)
    {
        auto key = get_symbol(trade->instrument_id, trade->exchange_id);
        auto direction = get_future_direction(trade->side, trade->offset);
        auto& pos_map = direction == DirectionLong ? long_pos_map_ : short_pos_map_;
        auto& detail_map = direction == DirectionLong ? long_detail_map_ : short_detail_map_;

        if (frozen_map_.find(trade->order_id) != frozen_map_.end())
        {
            frozen_map_[trade->order_id] -= trade->volume;
            pos_map[key].frozen_total -= trade->volume;
            if (!(trade->offset == OffsetCloseToday && strcmp(trade->exchange_id, EXCHANGE_SHFE) == 0))
            {
                pos_map[key].frozen_yesterday = std::max<int64_t>(pos_map[key].frozen_yesterday - trade->volume, 0);
            }
        }

        if (pos_map.find(key) == pos_map.end())
        {
            init_pos_from_trade(trade);
        }
        auto& pos = pos_map[key];
        pos.update_time = trade->rcv_time;
        pos.last_price = trade->price;
        auto* instrument = InstrumentManager::get_instrument_manager()->get_future_instrument(trade->instrument_id, trade->exchange_id);
        if (trade->offset == OffsetOpen)
        {
            auto open_cost = trade->price * trade->volume * instrument->contract_multiplier + trade->tax + trade->commission;
            pos.cost_price = (pos.cost_price * pos.volume * instrument->contract_multiplier + open_cost) /
                    (pos.volume + trade->volume) / instrument->contract_multiplier;
            pos.volume += trade->volume;

            insert_detail_from_trade(trade);
        }
        else
        {
            bool avail_enough = false;
            if (pos_map.find(key) != pos_map.end())
            {
                auto close_gain = trade->price * trade->volume * instrument->contract_multiplier - trade->tax - trade->commission;
                auto& pos = pos_map[key];
                auto close_profit = close_gain - pos.cost_price * trade->volume * instrument->contract_multiplier;
                pos.realized_pnl += close_profit;
                pos.close_pnl += close_profit;
                if (strcmp(trade->exchange_id, EXCHANGE_SHFE) == 0)
                {
                    if (trade->offset == OffsetCloseToday)
                    {
                        if (pos.volume - pos.yesterday_volume >= trade->volume)
                        {
                            avail_enough = true;
                            pos.cost_price = pos.volume == trade->volume ? 0 :
                                    (pos.cost_price * pos.volume * instrument->contract_multiplier - close_gain) /
                                    (pos.volume - trade->volume) / instrument->contract_multiplier;
                            pos.volume -= trade->volume;
                        }
                    }
                    else
                    {
                        if (pos.yesterday_volume >= trade->volume)
                        {
                            avail_enough = true;
                            pos.cost_price = pos.volume == trade->volume ? 0 :
                                    (pos.cost_price * pos.volume * instrument->contract_multiplier - close_gain) /
                                    (pos.volume - trade->volume) / instrument->contract_multiplier;
                            pos.volume -= trade->volume;
                            pos.yesterday_volume -= trade->volume;
                        }
                    }
                }
                else
                {
                    if (pos.volume >= trade->volume)
                    {
                        avail_enough = true;
                        pos.cost_price = pos.volume == trade->volume ? 0 :
                                (pos.cost_price * pos.volume * instrument->contract_multiplier - close_gain) /
                                (pos.volume - trade->volume) / instrument->contract_multiplier;
                        pos.volume -= trade->volume;
                        pos.yesterday_volume = std::max<int64_t>(pos.yesterday_volume - trade->volume, 0);
                    }
                }

                remove_detail_from_trade(trade);
            }
            if (!avail_enough)
            {
                SPDLOG_WARN("Not enough available positions");
            }
        }
    }
}

#endif //KUNGFU_POSITION_MANAGER_HPP