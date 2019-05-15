//
// Created by PolarAir on 2019-05-15.
//

#ifndef KUNGFU_ACCOUNT_MANAGER_HXX
#define KUNGFU_ACCOUNT_MANAGER_HXX

#include "account_manager.hpp"

namespace kungfu
{

    // impl
    AccountManager::impl::impl(const char *account_id, AccountType type, const char *db) :
            last_update_(0), commission_(account_id), pos_manager_(account_id, db), account_{}
    {
        strcpy(account_.account_id, account_id);
        account_.type = type;
        pos_manager_.register_pos_callback(std::bind(&AccountManager::impl::on_pos_callback, this, std::placeholders::_1));
    }

    AccountManager::impl::~impl()
    {

    }

    Position AccountManager::impl::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        return pos_manager_.get_long_pos(instrument_id, exchange_id);
    }

    Position AccountManager::impl::get_short_pos(const char *instrument_id, const char *exchange_id) const
    {
        return pos_manager_.get_short_pos(instrument_id, exchange_id);
    }

    double AccountManager::impl::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        return pos_manager_.get_last_price(instrument_id, exchange_id);
    }

    std::vector<Instrument> AccountManager::impl::get_all_pos_instruments() const
    {
        return pos_manager_.get_all_pos_instruments();
    }

    void AccountManager::impl::on_quote(const kungfu::Quote *quote)
    {
        last_update_ = quote->rcv_time;

        // 账户初次创建并登陆td时查询回来的pos如果没有成本价的话，则用现价对其赋值，忽略其被加入功夫系统前的pnl
        // 这样计算不会太离谱
        auto long_pos = pos_manager_.get_long_pos(quote->instrument_id, quote->exchange_id);
        if (is_zero(long_pos.cost_price) && long_pos.volume > 0)
        {
            double base = quote->last_price * long_pos.volume;
            if (quote->instrument_type == InstrumentTypeFuture)
            {
                auto* instrument = InstrumentManager::get_instrument_manager()->get_future_instrument(quote->instrument_id, quote->exchange_id);
                base = base * instrument->contract_multiplier * instrument->long_margin_ratio;
            }
            account_.initial_equity += base;
            account_.static_equity += base;
        }
        auto short_pos = pos_manager_.get_short_pos(quote->instrument_id, quote->exchange_id);
        if (is_zero(short_pos.cost_price) && short_pos.volume > 0)
        {
            double base = quote->last_price * short_pos.volume;
            if (quote->instrument_type == InstrumentTypeFuture)
            {
                auto* instrument = InstrumentManager::get_instrument_manager()->get_future_instrument(quote->instrument_id, quote->exchange_id);
                base = base * instrument->contract_multiplier * instrument->short_margin_ratio;
            }
            account_.initial_equity += base;
            account_.static_equity += base;
        }

        pos_manager_.on_quote(quote);
        if (recalc_acc())
        {
            callback();
        }
    }

    void AccountManager::impl::on_order(const kungfu::Order *order)
    {
        if (strcmp(order->account_id, account_.account_id) != 0)
        {
            return;
        }

        last_update_ = order->rcv_time;
        if (!is_final_status(order->status) || frozen_map_.find(order->order_id) == frozen_map_.end())
        {
            return;
        }

        auto fro = frozen_map_[order->order_id];
        frozen_map_.erase(order->order_id);
        account_.avail += fro;
        account_.frozen_cash -= fro;
        account_.update_time = order->rcv_time;
        pos_manager_.on_order(order);
        recalc_acc();
        callback();
    }

    void AccountManager::impl::on_trade(const kungfu::Trade *trade)
    {
        SPDLOG_TRACE("trade: {}", to_string(*trade));
        SPDLOG_TRACE("acc before: {}", to_string(account_));

        if (strcmp(trade->account_id, account_.account_id) != 0)
        {
            return;
        }

        last_update_ = trade->rcv_time;
        if (trade->volume == 0)
        {
            return;
        }

        double buy_cost = (is_reverse_repurchase(trade->instrument_id, trade->exchange_id) ? trade->volume : trade->price * trade->volume) + trade->tax + trade->commission;
        double sell_profit = trade->price * trade->volume - trade->tax - trade->commission;

        bool frozen = false;
        if (frozen_map_.find(trade->order_id) != frozen_map_.end())
        {
            frozen = true;
            frozen_map_[trade->order_id] -= buy_cost;
            account_.frozen_cash -= buy_cost;
        }

        account_.update_time = trade->rcv_time;
        account_.accumulated_fee += (trade->tax + trade->commission);
        account_.intraday_fee += (trade->tax + trade->commission);
        if (trade->side == SideBuy)
        {
            if (!frozen)
            {
                account_.avail -= buy_cost;
            }
            if (is_reverse_repurchase(trade->instrument_id, trade->exchange_id))
            {
                auto days = get_reverse_repurchase_expire_days(trade->instrument_id);
                auto expire_date = get_reverse_repurchase_expire_date(trade->instrument_id,
                                                                      yijinjing::parseNano(trade->trade_time, "%Y%m%d").c_str());
                bond_map_[expire_date] += trade->volume * trade->price / 100.0 * days / 360 + trade->volume;
            }
        }
        else
        {
            account_.avail += sell_profit;
        }

        pos_manager_.on_trade(trade);
        recalc_acc();
        callback();

        SPDLOG_TRACE("acc after: {}", to_string(account_));
    }

    void AccountManager::impl::on_positions(const vector<kungfu::Position> &positions)
    {
        vector<kungfu::Position> filtered;
        for (const auto& pos : positions)
        {
            if (strcmp(pos.account_id, account_.account_id) == 0)
            {
                filtered.emplace_back(pos);
            }
        }
        pos_manager_.on_positions(filtered);
        if (recalc_acc())
        {
            callback();
        }
    }

    void AccountManager::impl::on_position_details(const vector<kungfu::Position> &details)
    {
        // 股票期权可能跟期货类似的逻辑, 目前不支持
        pos_manager_.on_position_details(details);
    }

    void AccountManager::impl::on_account(const kungfu::AccountInfo &account)
    {
        if (strcmp(account.account_id, account_.account_id) != 0)
        {
            return;
        }
        last_update_ = account.rcv_time;
        account_.rcv_time = account.rcv_time;
        account_.update_time = account.update_time;
        strcpy(account_.trading_day, account.trading_day);
        account_.type = account.type;
        strcpy(account_.broker_id, account.broker_id);
        strcpy(account_.source_id, account.source_id);
        if (is_zero(account_.initial_equity))
        {
            account_.initial_equity = account.initial_equity;
        }
        if (is_zero(account_.static_equity))
        {
            account_.static_equity = account.static_equity;
        }
        account_.avail = account.avail;
        account_.margin = account.margin;
        recalc_acc();
        callback();
    }

    void AccountManager::impl::insert_order(const kungfu::OrderInput *input)
    {
        if (strcmp(input->account_id, account_.account_id) != 0)
        {
            return;
        }

        // 不处理卖出
        if (input->side == SideSell || input->volume <= 0)
        {
            return;
        }

        double buy_cost = 0;
        if (is_reverse_repurchase(input->instrument_id, input->exchange_id))
        {
            buy_cost = input->volume * (1 + get_reverse_repurchase_commission_rate(input->instrument_id));
        }
        else
        {
            buy_cost = input->volume * input->frozen_price + commission_.calc_commission(input);
        }

        if (is_less(account_.avail, buy_cost))
        {
            SPDLOG_ERROR("Not enough available cash");
        }

        account_.avail -= buy_cost;
        account_.frozen_cash += buy_cost;
        frozen_map_[input->order_id] = buy_cost;

        callback();
    }

    bool AccountManager::impl::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        boost::ignore_unused(algo_id, frozen);
        return false;
    }

    void AccountManager::impl::release_algo_order(uint64_t algo_id)
    {
        boost::ignore_unused(algo_id);
    }

    void AccountManager::impl::switch_day(const std::string &trading_day)
    {
        if (trading_day <= trading_day_)
        {
            return;
        }
        trading_day_ = trading_day;
        strcpy(account_.trading_day, trading_day.c_str());

        pos_manager_.switch_day(trading_day);

        // 撤销所有冻结
        frozen_map_.clear();
        account_.avail += account_.frozen_cash;
        account_.frozen_cash = 0;

        // 到期债券
        for (auto iter = bond_map_.begin(); iter != bond_map_.end();)
        {
            if (iter->first <= trading_day_)
            {
                account_.avail += iter->second;
                iter = bond_map_.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        // 重算pnl
        recalc_acc();
        account_.static_equity = account_.dynamic_equity;
        account_.intraday_pnl = 0;
        account_.intraday_pnl_ratio = 0;
        callback();
    }

    int64_t AccountManager::impl::get_last_update() const
    {
        return last_update_;
    }

    std::string AccountManager::impl::get_current_trading_day() const
    {
        return trading_day_;
    }

    void AccountManager::impl::set_current_trading_day(const std::string &trading_day)
    {
        pos_manager_.set_current_trading_day(trading_day);
        if (trading_day >= trading_day_)
        {
            if (!trading_day_.empty())
            {
                switch_day(trading_day);
            }
            else
            {
                trading_day_ = trading_day;
                strcpy(account_.trading_day, trading_day.c_str());
            }
        }
    }

    void AccountManager::impl::register_pos_callback(kungfu::PositionCallback cb)
    {
        pos_cbs_.emplace_back(std::move(cb));
    }

    void AccountManager::impl::register_acc_callback(kungfu::AccountCallback cb)
    {
        cbs_.emplace_back(std::move(cb));
    }

    void AccountManager::impl::register_pnl_callback(kungfu::PnLCallback cb)
    {
        boost::ignore_unused(cb);
    }

    void AccountManager::impl::set_initial_equity(double equity)
    {
        account_.initial_equity = equity;
    }

    void AccountManager::impl::set_static_equity(double equity)
    {
        account_.static_equity = equity;
    }

    double AccountManager::impl::calc_commission(const kungfu::Trade *trade) const
    {
        if (is_reverse_repurchase(trade->instrument_id, trade->exchange_id))
        {
            return get_reverse_repurchase_expire_days(trade->instrument_id) * trade->volume;
        }
        else
        {
            auto* rate = commission_.get_commission_rate(trade->instrument_id, trade->exchange_id);
            double base = rate->mode == CommissionRateModeByVolume ? trade->volume : trade->price * trade->volume;
            return std::max((trade->side == SideBuy ? rate->open_ratio : rate->close_ratio) * base, rate->min_commission);
        }
    }

    double AccountManager::impl::calc_tax(const kungfu::Trade *trade) const
    {
        // 税费共有四种，经手费(券商收取)、证管费(证监会收取)、过户费(中登收取)、印花税(国税，卖方收取)
        // 前两种合称佣金，其中征管费由券商代收，本系统内称为commission
        // 后两种本系统内合称tax
        // 以中信建投为例
        // 经手费 0.0487‰，征管费 0.02‰，过户费 0.02‰，印花税 1‰
        return trade->volume * trade->price * ((trade->side == SideSell ? 0.001 : 0) + 0.00002);
    }

    AccountInfo AccountManager::impl::get_account_info() const
    {
        return account_;
    }

    bool AccountManager::impl::recalc_acc()
    {
        auto old_dynamic = account_.dynamic_equity;
        account_.market_value = pos_manager_.get_market_value();
        account_.dynamic_equity = account_.market_value + account_.avail + account_.frozen_cash;
        if (is_zero(account_.initial_equity))
        {
            account_.initial_equity = account_.dynamic_equity;
        }
        if (is_zero(account_.static_equity))
        {
            account_.static_equity = account_.dynamic_equity;
        }
        account_.accumulated_pnl = account_.dynamic_equity - account_.initial_equity;
        account_.accumulated_pnl_ratio = is_zero(account_.initial_equity) ? 0 : account_.accumulated_pnl / account_.initial_equity;
        account_.intraday_pnl = account_.dynamic_equity - account_.static_equity;
        account_.intraday_pnl_ratio = is_zero(account_.static_equity) ? 0 : account_.intraday_pnl / account_.static_equity;
        return !is_equal(old_dynamic, account_.dynamic_equity);
    }

    void AccountManager::impl::callback() const
    {
        for (const auto& cb : cbs_)
        {
            cb(account_);
        }
    }

    void AccountManager::impl::on_pos_callback(const kungfu::Position &pos) const
    {
        for (const auto& cb : pos_cbs_)
        {
            cb(pos);
        }
    }
}

#endif //KUNGFU_ACCOUNT_MANAGER_HXX
