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

    class IPosDataFetcher
    {
    public:
        virtual int64_t get_long_tot(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_long_tot_avail(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_long_tot_fro(const char* instrument_id, const char* exchange_id) const = 0;

        virtual int64_t get_long_yd(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_long_yd_avail(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_long_yd_fro(const char* instrument_id, const char* exchange_id) const = 0;

        // 已实现盈亏, 含历史
        virtual double get_long_realized_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        // 未实现盈亏
        virtual double get_long_unrealized_pnl(const char* instrument_id, const char* exchange_id) const = 0;

        // 开仓价
        // 股票: 不记录此字段，为0
        // 期货统计: 开仓均价，开仓价的加权平均值  SUM(开仓价 * 量) / SUM(量)
        // 期货明细: 开仓价
        virtual double get_long_open_price(const char* instrument_id, const char* exchange_id) const = 0;

        // 成本价
        // 股票: (历史买入总资金 - 历史卖出总资金) / 当前仓位。如果当前仓位是0则为0
        // 期货: ((昨结算 * 昨仓量 + SUM(今开 * 量)) * 合约乘数 + SUM(今手续费)) / 合约乘数
        virtual double get_long_cost_price(const char* instrument_id, const char* exchange_id) const = 0;

        // 保证金
        virtual double get_long_margin(const char* instrument_id, const char* exchange_id) const = 0;
        // 持仓盈亏(仅期货), 日内持仓盈亏
        virtual double get_long_position_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        // 平仓盈亏(仅期货), 日内平仓盈亏
        virtual double get_long_close_pnl(const char* instrument_id, const char* exchange_id) const = 0;

        // 获取完整的pos结构
        virtual Position get_long_pos(const char* instrument_id, const char* exchange_id) const = 0;

        // 空仓部分和多仓逻辑一致, 仅期货有效
        virtual int64_t get_short_tot(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_short_tot_avail(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_short_tot_fro(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_short_yd(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_short_yd_avail(const char* instrument_id, const char* exchange_id) const = 0;
        virtual int64_t get_short_yd_fro(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_realized_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_unrealized_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_open_price(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_cost_price(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_margin(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_position_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_short_close_pnl(const char* instrument_id, const char* exchange_id) const = 0;
        virtual Position get_short_pos(const char* instrument_id, const char* exchange_id) const = 0;

        // 最新价
        virtual double get_last_price(const char* iinstrument_id, const char* exchange_id) const = 0;

        // 获取所有持仓
        virtual std::vector<Instrument> get_all_pos_instruments() const = 0;
    };

    class IAccDataFetcher
    {
    public:
        virtual double get_initial_equity() const = 0;
        virtual double get_static_equity() const = 0;
        virtual double get_dynamic_equity() const = 0;
        virtual double get_accumulated_pnl() const = 0;
        virtual double get_accumulated_pnl_ratio() const = 0;
        virtual double get_intraday_pnl() const = 0;
        virtual double get_intraday_pnl_ratio() const = 0;
        virtual double get_avail() const = 0;
        virtual double get_market_value() const = 0;
        virtual double get_margin() const = 0;
        virtual double get_accumulated_fee() const = 0;
        virtual double get_intraday_fee() const = 0;
        virtual double get_frozen_cash() const = 0;
        virtual double get_frozen_margin() const = 0;
        virtual double get_frozen_fee() const = 0;
        virtual double get_position_pnl() const = 0;
        virtual double get_close_pnl() const = 0;
    };

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
