//
// Created by qlu on 2019/5/21.
//

#include "position_manager_impl.h"
#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/util/instrument.h>
#include <kungfu/wingchun/serialize.h>
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    void load_single_pos(SQLite::Statement &query, kungfu::flying::Position &pos)
    {
        pos = {};
        pos.rcv_time = query.getColumn(0);
        pos.update_time = query.getColumn(1);
        strcpy(pos.instrument_id, query.getColumn(2));
        pos.instrument_type = query.getColumn(3)[0];
        strcpy(pos.exchange_id, query.getColumn(4));
        strcpy(pos.account_id, query.getColumn(5));
        strcpy(pos.client_id, query.getColumn(6));
        pos.direction = query.getColumn(7)[0];
        pos.volume = query.getColumn(8);
        pos.yesterday_volume = query.getColumn(9);
        pos.frozen_total = query.getColumn(10);
        pos.frozen_yesterday = query.getColumn(11);
        pos.last_price = query.getColumn(12);
        pos.open_price = query.getColumn(13);
        pos.cost_price = query.getColumn(14);
        pos.close_price = query.getColumn(15);
        pos.pre_close_price = query.getColumn(16);
        pos.settlement_price = query.getColumn(17);
        pos.pre_settlement_price = query.getColumn(18);
        pos.margin = query.getColumn(19);
        pos.position_pnl = query.getColumn(20);
        pos.close_pnl = query.getColumn(21);
        pos.realized_pnl = query.getColumn(22);
        pos.unrealized_pnl = query.getColumn(23);
        strcpy(pos.open_date, query.getColumn(24));
        strcpy(pos.expire_date, query.getColumn(25));
    }

    void save_single_pos(SQLite::Database &db, const char *table, const kungfu::flying::Position &pos)
    {
        std::string s = fmt::format("REPLACE INTO {}("
                                            "rcv_time, update_time, instrument_id, instrument_type, exchange_id, account_id, client_id, "
                                            "direction, volume, yesterday_volume, frozen_total, frozen_yesterday, last_price, open_price, "
                                            "cost_price, close_price, pre_close_price, settlement_price, pre_settlement_price, margin, "
                                            "position_pnl, close_pnl, realized_pnl, unrealized_pnl, open_date, expire_date"
                                            ") "
                                            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? , ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? , ?)", table);
        SQLite::Statement insert(db, s);
        insert.bind(1, pos.rcv_time);
        insert.bind(2, pos.update_time);
        insert.bind(3, pos.instrument_id);
        insert.bind(4, std::string(1, pos.instrument_type));
        insert.bind(5, pos.exchange_id);
        insert.bind(6, pos.account_id);
        insert.bind(7, pos.client_id);
        insert.bind(8, std::string(1, pos.direction));
        insert.bind(9, pos.volume);
        insert.bind(10, pos.yesterday_volume);
        insert.bind(11, pos.frozen_total);
        insert.bind(12, pos.frozen_yesterday);
        insert.bind(13, pos.last_price);
        insert.bind(14, pos.open_price);
        insert.bind(15, pos.cost_price);
        insert.bind(16, pos.close_price);
        insert.bind(17, pos.pre_close_price);
        insert.bind(18, pos.settlement_price);
        insert.bind(19, pos.pre_settlement_price);
        insert.bind(20, pos.margin);
        insert.bind(21, pos.position_pnl);
        insert.bind(22, pos.close_pnl);
        insert.bind(23, pos.realized_pnl);
        insert.bind(24, pos.unrealized_pnl);
        insert.bind(25, pos.open_date);
        insert.bind(26, pos.expire_date);

        insert.exec();
    }

    PositionManagerImpl::PositionManagerImpl(const char* account_id): account_id_(account_id) {}

    Position PositionManagerImpl::get_long_pos(const char *instrument_id, const char *exchange_id) const 
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

    Position PositionManagerImpl::get_short_pos(const char *instrument_id, const char *exchange_id) const
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

    double PositionManagerImpl::get_last_price(const char *instrument_id, const char *exchange_id) const
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

    std::vector<Instrument> PositionManagerImpl::get_all_pos_instruments() const
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

    double PositionManagerImpl::get_market_value() const
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

    void PositionManagerImpl::on_quote(const kungfu::journal::Quote *quote)
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

    void PositionManagerImpl::on_order(const kungfu::journal::Order *order)
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

    void PositionManagerImpl::on_trade(const kungfu::journal::Trade *trade)
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
    }

    void PositionManagerImpl::on_positions(const vector<kungfu::flying::Position> &positions)
    {
        for (const auto& pos : positions)
        {
            last_update_ = pos.rcv_time;
            auto key = get_symbol(pos.instrument_id, pos.exchange_id);
            auto& pos_map = pos.direction == DirectionLong ? long_pos_map_ : short_pos_map_;
            if (pos_map.find(key) == pos_map.end())
            {
                pos_map[key] = pos;
            }
            else
            {
                auto& cur_pos = pos_map[key];
                cur_pos.rcv_time = pos.rcv_time;
                cur_pos.update_time = pos.update_time;
                cur_pos.volume = pos.volume;
                cur_pos.yesterday_volume = pos.yesterday_volume;
                cur_pos.cost_price = pos.cost_price;
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
            }
            if (recalc_pos_by_price(pos_map[key]))
            {
                callback(pos_map[key]);
            }
        }
    }

    void PositionManagerImpl::on_position_details(const std::vector<kungfu::flying::Position> &details)
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

    void PositionManagerImpl::on_account(const kungfu::flying::AccountInfo &account)
    {
        boost::ignore_unused(account);
    }

    void PositionManagerImpl::on_order_input(const kungfu::journal::OrderInput *input)
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

    void PositionManagerImpl::on_switch_day(const std::string &trading_day)
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
    }

    int64_t PositionManagerImpl::get_last_update() const
    {
        return last_update_;
    }

    std::string PositionManagerImpl::get_current_trading_day() const
    {
        return trading_day_;
    }

    void PositionManagerImpl::set_current_trading_day(const std::string &trading_day)
    {
        if (trading_day >= trading_day_)
        {
            if (!trading_day_.empty())
            {
                on_switch_day(trading_day);
            }
            else
            {
                trading_day_ = trading_day;
            }
        }
    }

    void PositionManagerImpl::register_pos_callback(kungfu::PositionCallback cb)
    {
        cbs_.emplace_back(std::move(cb));
    }

    void PositionManagerImpl::register_acc_callback(kungfu::AccountCallback cb)
    {
        boost::ignore_unused(cb);
    }

    void PositionManagerImpl::register_pnl_callback(kungfu::PnLCallback cb)
    {
        boost::ignore_unused(cb);
    }

    double PositionManagerImpl::choose_price(const kungfu::flying::Position &pos) const
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

    bool PositionManagerImpl::recalc_pos_by_price(Position &pos)
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

    void PositionManagerImpl::callback(const kungfu::flying::Position &pos) const
    {
        for (const auto& cb : cbs_)
        {
            cb(pos);
        }
    }

    void PositionManagerImpl::init_pos_from_input(const kungfu::journal::OrderInput *input)
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

    void PositionManagerImpl::init_pos_from_trade(const kungfu::journal::Trade *trade)
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

    void PositionManagerImpl::deal_bond_from_buy(const kungfu::journal::Trade *trade)
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

    void PositionManagerImpl::insert_detail_from_trade(const kungfu::journal::Trade *trade)
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

    void PositionManagerImpl::remove_detail_from_trade(const kungfu::journal::Trade *trade)
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

    void PositionManagerImpl::on_order_stock(const kungfu::journal::Order *order)
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

    void PositionManagerImpl::on_order_future(const kungfu::journal::Order *order)
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

    void PositionManagerImpl::on_trade_stock(const kungfu::journal::Trade *trade)
    {
        auto key = get_symbol(trade->instrument_id, trade->exchange_id);
        if (frozen_map_.find(trade->order_id) != frozen_map_.end())
            long_pos_map_[key].frozen_yesterday -= trade->volume;

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
                auto profit = trade->volume * trade->price / 100.0 * get_reverse_repurchase_expire_days(trade->instrument_id) / 360;
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
            pos.cost_price = pos.volume == trade->volume ? 0 : (pos.cost_price * pos.volume - sell_gain) / (pos.volume - trade->volume);
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

    void PositionManagerImpl::on_trade_future(const kungfu::journal::Trade *trade)
    {
        auto key = get_symbol(trade->instrument_id, trade->exchange_id);
        auto direction = get_future_direction(trade->side, trade->offset);
        auto &pos_map = direction == DirectionLong ? long_pos_map_ : short_pos_map_;
        auto &detail_map = direction == DirectionLong ? long_detail_map_ : short_detail_map_;

        if (frozen_map_.find(trade->order_id) != frozen_map_.end()) {
            frozen_map_[trade->order_id] -= trade->volume;
            pos_map[key].frozen_total -= trade->volume;
            if (!(trade->offset == OffsetCloseToday && strcmp(trade->exchange_id, EXCHANGE_SHFE) == 0)) {
                pos_map[key].frozen_yesterday = std::max<int64_t>(pos_map[key].frozen_yesterday - trade->volume, 0);
            }
        }

        if (pos_map.find(key) == pos_map.end()) {
            init_pos_from_trade(trade);
        }
        auto &pos = pos_map[key];
        pos.update_time = trade->rcv_time;
        pos.last_price = trade->price;
        auto *instrument = InstrumentManager::get_instrument_manager()->get_future_instrument(trade->instrument_id,
                                                                                              trade->exchange_id);
        if (trade->offset == OffsetOpen) {
            auto open_cost =
                    trade->price * trade->volume * instrument->contract_multiplier + trade->tax + trade->commission;
            pos.cost_price = (pos.cost_price * pos.volume * instrument->contract_multiplier + open_cost) /
                             (pos.volume + trade->volume) / instrument->contract_multiplier;
            pos.volume += trade->volume;

            insert_detail_from_trade(trade);
        } else {
            bool avail_enough = false;
            if (pos_map.find(key) != pos_map.end()) {
                auto close_gain =
                        trade->price * trade->volume * instrument->contract_multiplier - trade->tax - trade->commission;
                auto &pos = pos_map[key];
                auto close_profit = close_gain - pos.cost_price * trade->volume * instrument->contract_multiplier;
                pos.realized_pnl += close_profit;
                pos.close_pnl += close_profit;
                if (strcmp(trade->exchange_id, EXCHANGE_SHFE) == 0) {
                    if (trade->offset == OffsetCloseToday) {
                        if (pos.volume - pos.yesterday_volume >= trade->volume) {
                            avail_enough = true;
                            pos.cost_price = pos.volume == trade->volume ? 0 :
                                             (pos.cost_price * pos.volume * instrument->contract_multiplier -
                                              close_gain) /
                                             (pos.volume - trade->volume) / instrument->contract_multiplier;
                            pos.volume -= trade->volume;
                        }
                    } else {
                        if (pos.yesterday_volume >= trade->volume) {
                            avail_enough = true;
                            pos.cost_price = pos.volume == trade->volume ? 0 :
                                             (pos.cost_price * pos.volume * instrument->contract_multiplier -
                                              close_gain) /
                                             (pos.volume - trade->volume) / instrument->contract_multiplier;
                            pos.volume -= trade->volume;
                            pos.yesterday_volume -= trade->volume;
                        }
                    }
                } else {
                    if (pos.volume >= trade->volume) {
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
            if (!avail_enough) {
                SPDLOG_WARN("Not enough available positions");
            }
        }
    }

    void PositionManagerImpl::load_from_db(const char* db_file)
    {
        SQLite::Database db(db_file, SQLite::OPEN_READONLY);
        load_from_db(db);
    }

    void PositionManagerImpl::load_from_db(SQLite::Database& db)
    {
        auto pos_func = [&](std::map<std::string, Position> &pos_map, Direction direction)
        {
            pos_map.clear();
            SQLite::Statement query_pos(db, "SELECT * FROM position WHERE account_id = ? and direction = ?");
            query_pos.bind(1, account_id_);
            query_pos.bind(2, std::string(1, direction));
            Position pos = {};
            while (query_pos.executeStep())
            {
                load_single_pos(query_pos, pos);
                pos_map[get_symbol(pos.instrument_id, pos.exchange_id)] = pos;
            }
        };
        pos_func(long_pos_map_, DirectionLong);
        pos_func(short_pos_map_, DirectionShort);

        bond_map_.clear();
        SQLite::Statement query_bond(db, "SELECT * FROM bond_position WHERE account_id = ?");
        query_bond.bind(1, account_id_);
        Position bond = {};
        while (query_bond.executeStep())
        {
            load_single_pos(query_bond, bond);
            bond_map_[bond.expire_date].emplace_back(bond);
        }

        auto detail_func = [&](std::map<std::string, std::vector<Position>> &detail_map, Direction direction)
        {
            detail_map.clear();
            SQLite::Statement query_detail(db, "SELECT * FROM pos_detail WHERE account_id = ? and direction = ? ORDER BY ROWID");
            query_detail.bind(1, account_id_);
            query_detail.bind(2, std::string(1, direction));
            Position pos = {};
            while (query_detail.executeStep())
            {
                load_single_pos(query_detail, pos);
                detail_map[get_symbol(pos.instrument_id, pos.exchange_id)].emplace_back(pos);
            }
        };
        detail_func(long_detail_map_, DirectionLong);
        detail_func(short_detail_map_, DirectionShort);

        frozen_map_.clear();
        SQLite::Statement query_frozen(db, "SELECT order_id, frozen_vol FROM pos_frozen WHERE account_id = ?");
        query_frozen.bind(1, account_id_);
        while (query_frozen.executeStep())
        {
            frozen_map_[query_frozen.getColumn(0)] = query_frozen.getColumn(1);
        }
    }

    void PositionManagerImpl::dump_to_db(SQLite::Database& db)
    {
        SQLite::Statement delete_position(db, "DELETE FROM position WHERE account_id = ?");
        delete_position.bind(1, account_id_);
        delete_position.exec();

        SQLite::Statement delete_bond_position(db, "DELETE FROM bond_position WHERE account_id = ?");
        delete_bond_position.bind(1, account_id_);
        delete_bond_position.exec();

        SQLite::Statement delete_pos_detail(db, "DELETE FROM pos_detail WHERE account_id = ?");
        delete_pos_detail.bind(1, account_id_);
        delete_pos_detail.exec();

        SQLite::Statement delete_pos_frozen(db, "DELETE FROM pos_frozen WHERE account_id = ?");
        delete_pos_frozen.bind(1, account_id_);
        delete_pos_frozen.exec();

        for (const auto &iter : long_pos_map_)
        {
            save_single_pos(db, "position", iter.second);
        }
        for (const auto &iter : short_pos_map_)
        {
            save_single_pos(db, "position", iter.second);
        }

        for (const auto &iter : bond_map_)
        {
            for (const auto &bond : iter.second)
            {
                save_single_pos(db, "bond_position", bond);
            }
        }

        for (const auto &iter : long_detail_map_)
        {
            for (const auto &detail : iter.second)
            {
                save_single_pos(db, "pos_detail", detail);
            }
        }
        for (const auto &iter : short_detail_map_)
        {
            for (const auto &detail : iter.second)
            {
                save_single_pos(db, "pos_detail", detail);
            }
        }

        for (const auto &iter : frozen_map_)
        {
            SQLite::Statement insert_pos_frozen(db, "INSERT INTO pos_frozen(order_id, account_id, frozen_vol) VALUES(?, ?, ?)");
            insert_pos_frozen.bind(1, iter.first);
            insert_pos_frozen.bind(2, account_id_);
            insert_pos_frozen.bind(3, iter.second);
            insert_pos_frozen.exec();
        }
    }

    void PositionManagerImpl::dump_to_db(const char* db_file)
    {
        SQLite::Database db(db_file, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        dump_to_db(db);
    }

    PositionManagerPtr create_position_manager(const char* account_id, const char* db)
    {
        auto position_manager = PositionManagerPtr(new PositionManagerImpl(account_id));
        if (db != nullptr)
        {
            position_manager->load_from_db(db);
        }
        return position_manager;
    }
}
