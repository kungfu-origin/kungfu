//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/book/book.h>
#include <kungfu/wingchun/algo/algo.h>

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

                //获取交易账户列表
                virtual std::vector<yijinjing::data::location_ptr> list_accounts();

                //获取交易账户可用资金限制
                //@param account_id  账户ID
                virtual double get_account_cash_limit(const std::string &account);

                //订阅行情
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                virtual void subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string &exchange = "");

                virtual void subscribe_all(const std::string &source);

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
                                              double limit_price, int64_t volume, PriceType type, Side side, Offset offset = Offset::Open,
                                              HedgeFlag hedge_flag = HedgeFlag::Speculation);

                //撤单
                //@param order_id      订单ID
                //@return              撤单操作ID
                virtual uint64_t cancel_order(uint64_t order_id);

                virtual book::BookContext_ptr get_book_context() const { return book_context_; };

                virtual algo::AlgoContext_ptr get_algo_context() const { return algo_context_; }

            protected:
                virtual void react();

                std::unordered_map<uint32_t, msg::data::Quote> quotes_;
                practice::apprentice &app_;
                const rx::connectable_observable<yijinjing::event_ptr> &events_;

            private:
                book::BookContext_ptr book_context_;
                algo::AlgoContext_ptr algo_context_;

                uint32_t lookup_account_location_id(const std::string &account);

                void subscribe_instruments();

                uint32_t add_marketdata(const std::string &source);

                void request_subscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange);

                template <class T> bool is_subscribed(const T& data)
                {
                    return subscribe_all_ or subscribed_symbols_.find(get_symbol_id(data.instrument_id, data.exchange_id)) != subscribed_symbols_.end();
                }

            private:
                std::unordered_map<uint32_t, uint32_t> account_location_ids_;
                std::unordered_map<uint32_t, yijinjing::data::location_ptr> accounts_;
                std::unordered_map<uint32_t, double> account_cash_limits_;
                std::unordered_map<std::string, uint32_t> market_data_;

                bool subscribe_all_;

                std::unordered_map<uint32_t, uint32_t> subscribed_symbols_;

                friend class Runner;
            };
        }
    }
}

#endif //WINGCHUN_CONTEXT_H
