//
// Created by qlu on 2019/1/16.
//

#ifndef WINGCHUN_STRATEGY_H
#define WINGCHUN_STRATEGY_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            class Strategy
            {
            public:
                virtual ~Strategy() = default;

                //运行前
                virtual void pre_start(Context_ptr context)
                {};

                virtual void post_start(Context_ptr context)
                {};

                //退出前
                virtual void pre_stop(Context_ptr context)
                {};

                virtual void post_stop(Context_ptr context)
                {};

                //交易日切换回调
                //@param next_trading_day  下一个交易日
                virtual void on_trading_day(Context_ptr context, const std::string &next_trading_day)
                {};

                //行情数据更新回调
                //@param quote             行情数据
                virtual void on_quote(Context_ptr context, const msg::data::Quote &quote)
                {};

                //逐笔委托更新回调
                //@param entrust           逐笔委托数据
                virtual void on_entrust(Context_ptr context, const msg::data::Entrust &entrust)
                {};

                //逐笔成交更新回调
                //@param transaction       逐笔成交数据
                virtual void on_transaction(Context_ptr context, const msg::data::Transaction &transaction)
                {};

                //订单信息更新回调
                //@param order             订单信息数据
                virtual void on_order(Context_ptr context, const msg::data::Order &order)
                {};

                //订单成交回报回调
                //@param trade             订单成交数据
                virtual void on_trade(Context_ptr context, const msg::data::Trade &trade)
                {};

            };

            DECLARE_PTR(Strategy)
        }
    }
}
#endif //WINGCHUN_STRATEGY_H
