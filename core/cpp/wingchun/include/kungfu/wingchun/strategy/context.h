//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/strategy/strategy.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            class Runner;

            class Context : public std::enable_shared_from_this<Context>
            {
            public:
                explicit Context(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events);

                ~Context() = default;

                //获取当前纳秒时间
                //@return            当前纳秒时间
                int64_t now() const;

                virtual void add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)> &callback);

                virtual void add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)> &callback);

                //添加策略使用的交易账户
                //@param source_id   柜台ID
                //@param account_id  账户ID
                //@param cash_limit  可用资金限制
                virtual void add_account(const std::string &source, const std::string &account, double cash_limit);

                //订阅行情
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                virtual void subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string &exchange = "");

                //报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param type          报单价格类型
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                virtual uint64_t insert_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                              double limit_price, int64_t volume, PriceType type, Side side, Offset offset = Offset::Open);


                //撤单
                //@param order_id      订单ID
                //@return              撤单操作ID
                virtual uint64_t cancel_order(uint64_t order_id);

            protected:
                virtual void react();

                std::unordered_map<uint32_t, msg::data::Quote> quotes_;
                practice::apprentice &app_;
                const rx::connectable_observable<yijinjing::event_ptr> &events_;

            private:
                uint32_t lookup_account_location_id(const std::string &account);

                void request_subscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange);

            private:
                std::unordered_map<uint32_t, uint32_t> account_location_ids_;
                std::unordered_map<uint32_t, std::string> accounts_;
                std::unordered_map<std::string, uint32_t> market_data_;

                friend class Runner;
            };

            enum OrderType
            {
                UNKNOWN = 0,
                MARKET = 1,
                LIMIT = 2,
            };
        }
    }
}

#endif //WINGCHUN_CONTEXT_H
