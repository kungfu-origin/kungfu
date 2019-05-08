//
// Created by PolarAir on 2019-04-16.
//

#ifndef KUNGFU_PNL_DEF_H
#define KUNGFU_PNL_DEF_H

#include "md_struct.h"
#include "oms_struct.h"
#include <functional>

namespace kungfu
{
    typedef std::function<void (const Position&)> PositionCallback;
    typedef std::function<void (const AccountInfo&)> AccountCallback;
    typedef std::function<void (const PortfolioInfo&)> PnLCallback;

    class IPnLDataHandler
    {
    public:
        // 柜台数据
        virtual void on_quote(const Quote* quote) = 0;
        virtual void on_order(const Order* order) = 0;
        virtual void on_trade(const Trade* trade) = 0;

        // position和account接口用于一次性初始化或者刷新持仓和账户
        virtual void on_positions(const std::vector<Position>& positions) = 0;
        virtual void on_position_details(const std::vector<Position>& details) = 0;
        virtual void on_account(const AccountInfo& account) = 0;

        // 操作
        virtual void insert_order(const OrderInput* input) = 0;
        virtual bool freeze_algo_order(uint64_t algo_id, const AssetsFrozen& frozen) { return false; };
        virtual void release_algo_order(uint64_t algo_id) {};
        virtual void switch_day(const std::string& trading_day) = 0;
        virtual int64_t get_last_update() const = 0;
        virtual std::string get_current_trading_day() const = 0;
        virtual void set_current_trading_day(const std::string& trading_day) = 0;
        virtual void register_pos_callback(PositionCallback cb) = 0;
        virtual void register_acc_callback(AccountCallback cb) = 0;
        virtual void register_pnl_callback(PnLCallback cb) = 0;
        virtual void set_initial_equity(double equity) = 0;
        virtual void set_static_equity(double equity) = 0;
    };
}

#endif //KUNGFU_PNL_DEF_H
