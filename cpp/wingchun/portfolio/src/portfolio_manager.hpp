//
// Created by PolarAir on 2019-04-18.
//

#ifndef KUNGFU_PORTFOLIO_MANAGER_HPP
#define KUNGFU_PORTFOLIO_MANAGER_HPP

#include "../include/portfolio_manager.h"
#include "../include/account_manager.h"
#include "portfolio_storage.h"
#include "util/include/business_helper.h"
#include "serialize.h"
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>

namespace kungfu
{
    class PortfolioManager::impl final : public IPnLDataHandler, public IPosDataFetcher, public IAccDataFetcher
    {
    public:
        explicit impl(const char* db);
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
        double get_short_position_pnl(const char* instrument_id, const char* exchange_id) const override;
        double get_short_margin(const char* instrument_id, const char* exchange_id) const override;
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

        const PortfolioInfo* get_pnl() const;
        const AccountManagerPtr get_account(const char* account_id) const;

    private:
        bool recalc_pnl();
        void callback() const;
        void on_acc_callback(const AccountInfo& acc) const;
        void on_pos_callback(const Position& pos) const;

    private:
        std::string                                     db_;
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        std::map<std::string, AccountManagerPtr>        accounts_;
        PortfolioStorage                                storage_;
        PortfolioInfo                                   pnl_;
        std::vector<PnLCallback>                        cbs_;
        std::vector<AccountCallback>                    acc_cbs_;
        std::vector<PositionCallback>                   pos_cbs_;
    };

    // impl
    PortfolioManager::impl::impl(const char *db) :
            db_(db), last_update_(0), storage_(db), pnl_{}
    {
        storage_.load(last_update_, trading_day_, pnl_, accounts_);
        for (auto& iter : accounts_)
        {
            iter.second->register_pos_callback(std::bind(&PortfolioManager::impl::on_pos_callback, this, std::placeholders::_1));
            iter.second->register_acc_callback(std::bind(&PortfolioManager::impl::on_acc_callback, this, std::placeholders::_1));
        }
    }

    PortfolioManager::impl::~impl()
    {
        storage_.save(last_update_, trading_day_, pnl_);
    }

#define IMPLEMENT_POS_FUNC(ret_type, func_name) \
    ret_type PortfolioManager::impl::func_name(const char *instrument_id, const char *exchange_id) const \
    { \
        ret_type ret = 0; \
        for (const auto& iter : accounts_) \
        { \
            if (nullptr != iter.second) \
            { \
                ret += iter.second->func_name(instrument_id, exchange_id); \
            } \
        } \
        return ret; \
    }

    IMPLEMENT_POS_FUNC(int64_t, get_long_tot)
    IMPLEMENT_POS_FUNC(int64_t, get_long_tot_avail)
    IMPLEMENT_POS_FUNC(int64_t, get_long_tot_fro)
    IMPLEMENT_POS_FUNC(int64_t, get_long_yd)
    IMPLEMENT_POS_FUNC(int64_t, get_long_yd_avail)
    IMPLEMENT_POS_FUNC(int64_t, get_long_yd_fro)
    IMPLEMENT_POS_FUNC(double, get_long_realized_pnl)
    IMPLEMENT_POS_FUNC(double, get_long_unrealized_pnl)
    IMPLEMENT_POS_FUNC(double, get_long_margin)
    IMPLEMENT_POS_FUNC(double, get_long_position_pnl)
    IMPLEMENT_POS_FUNC(double, get_long_close_pnl)
    IMPLEMENT_POS_FUNC(int64_t, get_short_tot)
    IMPLEMENT_POS_FUNC(int64_t, get_short_tot_avail)
    IMPLEMENT_POS_FUNC(int64_t, get_short_tot_fro)
    IMPLEMENT_POS_FUNC(int64_t, get_short_yd)
    IMPLEMENT_POS_FUNC(int64_t, get_short_yd_avail)
    IMPLEMENT_POS_FUNC(int64_t, get_short_yd_fro)
    IMPLEMENT_POS_FUNC(double, get_short_realized_pnl)
    IMPLEMENT_POS_FUNC(double, get_short_unrealized_pnl)
    IMPLEMENT_POS_FUNC(double, get_short_margin)
    IMPLEMENT_POS_FUNC(double, get_short_position_pnl)
    IMPLEMENT_POS_FUNC(double, get_short_close_pnl)

    double PortfolioManager::impl::get_long_open_price(const char *instrument_id, const char *exchange_id) const
    {
        int64_t vol_tot = 0;
        double open_tot = 0.0;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                vol_tot += iter.second->get_long_tot(instrument_id, exchange_id);
                open_tot += iter.second->get_long_tot(instrument_id, exchange_id) *
                            iter.second->get_long_open_price(instrument_id, exchange_id);
            }
        }
        return open_tot / vol_tot;
    }

    double PortfolioManager::impl::get_long_cost_price(const char *instrument_id, const char *exchange_id) const
    {
        int64_t vol_tot = 0;
        double cost_tot = 0.0;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                vol_tot += iter.second->get_long_tot(instrument_id, exchange_id);
                cost_tot += iter.second->get_long_tot(instrument_id, exchange_id) *
                        iter.second->get_long_cost_price(instrument_id, exchange_id);
            }
        }
        return cost_tot / vol_tot;
    }

    Position PortfolioManager::impl::get_long_pos(const char *instrument_id, const char *exchange_id) const
    {
        Position pos = {};
        strcpy(pos.instrument_id, instrument_id);
        strcpy(pos.exchange_id, exchange_id);
        pos.direction = DirectionLong;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                auto cur_pos = iter.second->get_long_pos(instrument_id, exchange_id);
                pos.open_price = (pos.open_price * pos.volume + cur_pos.open_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                pos.cost_price = (pos.cost_price * pos.volume + cur_pos.cost_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                pos.margin += cur_pos.margin;
                pos.position_pnl += cur_pos.position_pnl;
                pos.close_pnl += cur_pos.close_pnl;
                pos.realized_pnl += cur_pos.realized_pnl;
                pos.unrealized_pnl += cur_pos.unrealized_pnl;
            }
        }
        return pos;
    }

    double PortfolioManager::impl::get_short_open_price(const char *instrument_id, const char *exchange_id) const
    {
        int64_t vol_tot = 0;
        double open_tot = 0.0;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                vol_tot += iter.second->get_short_tot(instrument_id, exchange_id);
                open_tot += iter.second->get_short_tot(instrument_id, exchange_id) *
                            iter.second->get_short_open_price(instrument_id, exchange_id);
            }
        }
        return open_tot / vol_tot;
    }

    double PortfolioManager::impl::get_short_cost_price(const char *instrument_id, const char *exchange_id) const
    {
        int64_t vol_tot = 0;
        double cost_tot = 0.0;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                vol_tot += iter.second->get_short_tot(instrument_id, exchange_id);
                cost_tot += iter.second->get_short_tot(instrument_id, exchange_id) *
                            iter.second->get_short_cost_price(instrument_id, exchange_id);
            }
        }
        return cost_tot / vol_tot;
    }

    Position PortfolioManager::impl::get_short_pos(const char *instrument_id, const char *exchange_id) const
    {
        Position pos = {};
        strcpy(pos.instrument_id, instrument_id);
        strcpy(pos.exchange_id, exchange_id);
        pos.direction = DirectionShort;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                auto cur_pos = iter.second->get_short_pos(instrument_id, exchange_id);
                pos.open_price = (pos.open_price * pos.volume + cur_pos.open_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                pos.cost_price = (pos.cost_price * pos.volume + cur_pos.cost_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                pos.margin += cur_pos.margin;
                pos.position_pnl += cur_pos.position_pnl;
                pos.close_pnl += cur_pos.close_pnl;
                pos.realized_pnl += cur_pos.realized_pnl;
                pos.unrealized_pnl += cur_pos.unrealized_pnl;
            }
        }
        return pos;
    }

    double PortfolioManager::impl::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                double price = iter.second->get_last_price(instrument_id, exchange_id);
                if (!is_zero(price))
                {
                    return price;
                }
            }
        }

        return 0;
    }

    std::vector<Instrument> PortfolioManager::impl::get_all_pos_instruments() const
    {
        std::map<std::string, Instrument> m;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                auto ins_vec = iter.second->get_all_pos_instruments();
                for (const auto& pos : ins_vec)
                {
                    auto key = get_symbol(pos.instrument_id, pos.exchange_id);
                    if (m.find(key) == m.end())
                    {
                        m[key] = pos;
                    }
                }
            }
        }
        std::vector<Instrument> ret;
        for (const auto& iter : m)
        {
            ret.emplace_back(iter.second);
        }
        return ret;
    }

#define IMPLEMENT_PNL_FUNC(ret_type, field_name) \
    ret_type PortfolioManager::impl::get_##field_name() const \
    { \
        return pnl_.field_name; \
    }

    IMPLEMENT_PNL_FUNC(double, initial_equity)
    IMPLEMENT_PNL_FUNC(double, static_equity)
    IMPLEMENT_PNL_FUNC(double, dynamic_equity)
    IMPLEMENT_PNL_FUNC(double, accumulated_pnl)
    IMPLEMENT_PNL_FUNC(double, accumulated_pnl_ratio)
    IMPLEMENT_PNL_FUNC(double, intraday_pnl)
    IMPLEMENT_PNL_FUNC(double, intraday_pnl_ratio)

#define IMPLEMENT_ACC_FUNC(ret_type, func_name) \
    ret_type PortfolioManager::impl::func_name() const \
    { \
        ret_type ret = 0; \
        for (const auto& iter : accounts_) \
        { \
            if (nullptr != iter.second) \
            { \
                ret += iter.second->func_name(); \
            } \
        } \
        return ret; \
    }

    IMPLEMENT_ACC_FUNC(double, get_avail)
    IMPLEMENT_ACC_FUNC(double, get_market_value)
    IMPLEMENT_ACC_FUNC(double, get_margin)
    IMPLEMENT_ACC_FUNC(double, get_accumulated_fee)
    IMPLEMENT_ACC_FUNC(double, get_intraday_fee)
    IMPLEMENT_ACC_FUNC(double, get_frozen_cash)
    IMPLEMENT_ACC_FUNC(double, get_frozen_margin)
    IMPLEMENT_ACC_FUNC(double, get_frozen_fee)
    IMPLEMENT_ACC_FUNC(double, get_position_pnl)
    IMPLEMENT_ACC_FUNC(double, get_close_pnl)

#define IMPLEMENT_DATA_BODY(func_name, ...) \
    for (const auto& iter : accounts_) \
    { \
        if (nullptr != iter.second) \
        { \
            iter.second->func_name(__VA_ARGS__); \
        } \
    } \
    if (recalc_pnl()) \
    { \
        callback(); \
    }

    void PortfolioManager::impl::on_quote(const kungfu::Quote *quote)
    {
        last_update_ = quote->rcv_time;
        IMPLEMENT_DATA_BODY(on_quote, quote)
    }

    void PortfolioManager::impl::on_order(const kungfu::Order *order)
    {
        last_update_ = order->rcv_time;
        IMPLEMENT_DATA_BODY(on_order, order)
    }

    void PortfolioManager::impl::on_trade(const kungfu::Trade *trade)
    {
        SPDLOG_TRACE("trade: {}", to_string(*trade));
        SPDLOG_TRACE("pnl before: {}", to_string(pnl_));
        last_update_ = trade->rcv_time;
        IMPLEMENT_DATA_BODY(on_trade, trade)
        // 成交时一定写一下数据库, 解决前端切换页面时读库问题
        storage_.save(last_update_, trading_day_, pnl_);
        SPDLOG_TRACE("pnl after: {}", to_string(pnl_));
    }

    void PortfolioManager::impl::on_positions(const std::vector<kungfu::Position> &positions)
    {
        IMPLEMENT_DATA_BODY(on_positions, positions)
    }

    void PortfolioManager::impl::on_position_details(const std::vector<kungfu::Position> &details)
    {
        IMPLEMENT_DATA_BODY(on_position_details, details)
    }

    void PortfolioManager::impl::on_account(const kungfu::AccountInfo &account)
    {
        if (accounts_.find(account.account_id) == accounts_.end())
        {
            auto account_manager = std::make_shared<AccountManager>(account.account_id, account.type, db_.c_str());
            accounts_[account.account_id] = account_manager;
            account_manager->register_pos_callback(std::bind(&PortfolioManager::impl::on_pos_callback, this, std::placeholders::_1));
            account_manager->register_acc_callback(std::bind(&PortfolioManager::impl::on_acc_callback, this, std::placeholders::_1));
        }
        accounts_[account.account_id]->on_account(account);
        SPDLOG_TRACE("pnl after on_account: {}", to_string(pnl_));
    }

    void PortfolioManager::impl::insert_order(const kungfu::OrderInput *input)
    {
        IMPLEMENT_DATA_BODY(insert_order, input)
    }

    bool PortfolioManager::impl::freeze_algo_order(uint64_t algo_id, const kungfu::AssetsFrozen &frozen)
    {
        IMPLEMENT_DATA_BODY(freeze_algo_order, algo_id, frozen)
        return true;
    }

    void PortfolioManager::impl::release_algo_order(uint64_t algo_id)
    {
        IMPLEMENT_DATA_BODY(release_algo_order, algo_id)
    }

    void PortfolioManager::impl::switch_day(const std::string & trading_day)
    {
        if (trading_day <= trading_day_)
        {
            return;
        }
        trading_day_ = trading_day;
        strcpy(pnl_.trading_day, trading_day.c_str());

        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                iter.second->switch_day(trading_day);
            }
        }

        recalc_pnl();
        callback();

        storage_.save(last_update_, trading_day_, pnl_);
    }

    int64_t PortfolioManager::impl::get_last_update() const
    {
        return last_update_;
    }

    std::string PortfolioManager::impl::get_current_trading_day() const
    {
        return trading_day_;
    }

    void PortfolioManager::impl::set_current_trading_day(const std::string &trading_day)
    {
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                iter.second->set_current_trading_day(trading_day);
            }
        }

        if (trading_day >= trading_day_)
        {
            if (!trading_day_.empty())
            {
                switch_day(trading_day);
            }
            else
            {
                trading_day_ = trading_day;
                strcpy(pnl_.trading_day, trading_day.c_str());
            }
        }
    }

    void PortfolioManager::impl::register_pos_callback(kungfu::PositionCallback cb)
    {
        pos_cbs_.emplace_back(std::move(cb));
    }

    void PortfolioManager::impl::register_acc_callback(kungfu::AccountCallback cb)
    {
        acc_cbs_.emplace_back(std::move(cb));
    }

    void PortfolioManager::impl::register_pnl_callback(kungfu::PnLCallback cb)
    {
        cbs_.emplace_back(std::move(cb));
    }

    void PortfolioManager::impl::set_initial_equity(double equity)
    {
        boost::ignore_unused(equity);
    }

    void PortfolioManager::impl::set_static_equity(double equity)
    {
        boost::ignore_unused(equity);
    }

    bool PortfolioManager::impl::recalc_pnl()
    {
        double old_dynamic = pnl_.dynamic_equity;

        pnl_.initial_equity = 0;
        pnl_.static_equity = 0;
        pnl_.dynamic_equity = 0;
        for (const auto& iter : accounts_)
        {
            pnl_.initial_equity += iter.second->get_initial_equity();
            pnl_.static_equity += iter.second->get_static_equity();
            pnl_.dynamic_equity += iter.second->get_dynamic_equity();
        }
        pnl_.accumulated_pnl = pnl_.dynamic_equity - pnl_.initial_equity;
        pnl_.accumulated_pnl_ratio = is_zero(pnl_.initial_equity) ? 0 : pnl_.accumulated_pnl / pnl_.initial_equity;
        pnl_.intraday_pnl = pnl_.dynamic_equity - pnl_.static_equity;
        pnl_.intraday_pnl_ratio = is_zero(pnl_.static_equity) ? 0 : pnl_.intraday_pnl / pnl_.static_equity;
        return !is_equal(old_dynamic, pnl_.dynamic_equity);
    }

    void PortfolioManager::impl::callback() const
    {
        for (const auto& cb : cbs_)
        {
            cb(pnl_);
        }
    }

    const PortfolioInfo* PortfolioManager::impl::get_pnl() const
    {
        return &pnl_;
    }

    const AccountManagerPtr PortfolioManager::impl::get_account(const char *account_id) const
    {
        if (accounts_.find(account_id) != accounts_.end())
        {
            return accounts_.at(account_id);
        }
        return AccountManagerPtr();
    }

    void PortfolioManager::impl::on_pos_callback(const kungfu::Position &pos) const
    {
        Position total_pos = {};
        memcpy(&total_pos, &pos, sizeof(Position));
        strcpy(total_pos.account_id, "");
        total_pos.volume = total_pos.direction == DirectionLong ?
                get_long_tot(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_tot(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.yesterday_volume = total_pos.direction == DirectionLong ?
                get_long_yd(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_yd(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.frozen_total = total_pos.direction == DirectionLong ?
                get_long_tot_fro(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_tot_fro(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.frozen_yesterday = total_pos.direction == DirectionLong ?
                get_long_yd_fro(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_yd_fro(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.cost_price = total_pos.direction == DirectionLong ?
                get_long_cost_price(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_cost_price(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.margin = total_pos.direction == DirectionLong ?
                get_long_margin(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_margin(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.position_pnl = total_pos.direction == DirectionLong ?
                get_long_position_pnl(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_position_pnl(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.close_pnl = total_pos.direction == DirectionLong ?
                get_long_close_pnl(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_close_pnl(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.realized_pnl = total_pos.direction == DirectionLong ?
                get_long_realized_pnl(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_realized_pnl(total_pos.instrument_id, total_pos.exchange_id);
        total_pos.unrealized_pnl = total_pos.direction == DirectionLong ?
                get_long_unrealized_pnl(total_pos.instrument_id, total_pos.exchange_id) :
                get_short_unrealized_pnl(total_pos.instrument_id, total_pos.exchange_id);

        for (auto& cb : pos_cbs_)
        {
            cb(total_pos);
        }
    }

    void PortfolioManager::impl::on_acc_callback(const kungfu::AccountInfo &acc) const
    {
        for (auto& cb : acc_cbs_)
        {
            cb(acc);
        }
    }
}

#endif //KUNGFU_PORTFOLIO_MANAGER_HPP
