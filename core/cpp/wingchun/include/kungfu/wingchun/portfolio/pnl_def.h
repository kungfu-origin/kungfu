//
// Created by PolarAir on 2019-04-16.
//

#ifndef KUNGFU_PNL_DEF_H
#define KUNGFU_PNL_DEF_H

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace portfolio_util
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger);
    }

    typedef std::function<void (const flying::Position&)> PositionCallback;
    typedef std::function<void (const flying::AccountInfo&)> AccountCallback;
    typedef std::function<void (const flying::PortfolioInfo&)> PnLCallback;

    class IPnLDataHandler
    {
    public:
        virtual void on_order_input(const journal::OrderInput* input) = 0;
        virtual void on_quote(const journal::Quote* quote) = 0;
        virtual void on_order(const journal::Order* order) = 0;
        virtual void on_trade(const journal::Trade* trade) = 0;

        // position和account接口用于一次性初始化或者刷新持仓和账户
        virtual void on_positions(const std::vector<flying::Position>& positions) = 0;
        virtual void on_position_details(const std::vector<flying::Position>& details) = 0;
        virtual void on_account(const flying::AccountInfo& account) = 0;

        virtual void on_switch_day(const std::string& trading_day) = 0;

        virtual int64_t get_last_update() const = 0;
        virtual std::string get_current_trading_day() const = 0;
        virtual void set_current_trading_day(const std::string& trading_day) = 0;

        virtual void register_pos_callback(PositionCallback cb) = 0;
        virtual void register_acc_callback(AccountCallback cb) = 0;
        virtual void register_pnl_callback(PnLCallback cb) = 0;

    };
}

#endif //KUNGFU_PNL_DEF_H
