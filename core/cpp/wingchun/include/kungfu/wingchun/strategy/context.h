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

            inline uint32_t get_symbol_id(const std::string &symbol, const std::string &exchange)
            {
                return yijinjing::util::hash_str_32(symbol) ^ yijinjing::util::hash_str_32(exchange);
            }

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

                virtual uint64_t insert_order(const msg::data::OrderInput &input);

                //限价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                virtual uint64_t insert_limit_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                            double limit_price, int64_t volume, Side side, Offset offset);

                //fok单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                virtual uint64_t insert_fok_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                          double limit_price, int64_t volume, Side side, Offset offset);

                //fak单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                virtual uint64_t insert_fak_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                          double limit_price, int64_t volume, Side side, Offset offset);

                //市价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                virtual uint64_t insert_market_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                             int64_t volume, Side side, Offset offset);

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

            FORWARD_DECLARE_PTR(ContextBackTest)
        
        enum OrderType
        {
            UNKNOWN = 0,
            MARKET = 1,
            LIMIT = 2,
        };

        class ContextBackTest: public Context
            {
            public:
                ContextBackTest(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events, std::vector<Strategy_ptr> strategies);

                ~ContextBackTest() = default;
                void react() override;
                void add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)> &callback) override;
                void add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)> &callback) override;

                void add_account(const std::string &source, const std::string &account, double cash_limit) override;
                void subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string &exchange = "") override;
                
                uint64_t insert_order(const msg::data::OrderInput &input) override;
                
                //限价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_limit_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                            double limit_price, int64_t volume, Side side, Offset offset) override;

                //fok单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_fok_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                          double limit_price, int64_t volume, Side side, Offset offset) override;

                //fak单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_fak_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                          double limit_price, int64_t volume, Side side, Offset offset) override;

                //市价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_market_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                             int64_t volume, Side side, Offset offset) override;

                //撤单
                //@param order_id      订单ID
                //@return              撤单操作ID
                uint64_t cancel_order(uint64_t order_id) override;

            public:
                std::vector<kungfu::wingchun::msg::data::Quote> get_quotes() {return all_quotes_;}
                std::unordered_map<uint64_t, kungfu::wingchun::msg::data::Order> get_orders() {return orders_;}
                std::unordered_map<uint64_t, kungfu::wingchun::msg::data::Trade> get_trades() {return trades_;}

            private:
                std::vector<Strategy_ptr> strategies_;
                uint64_t operation_count_;
                std::vector<kungfu::wingchun::msg::data::Quote> all_quotes_;
                std::unordered_map<uint64_t, kungfu::wingchun::msg::data::OrderInput> order_inputs_;
                std::unordered_map<uint64_t, kungfu::wingchun::msg::data::Order> orders_;
                std::unordered_map<uint64_t, kungfu::wingchun::msg::data::Trade> trades_;

            private:
                void process_order(const kungfu::wingchun::msg::data::OrderInput &);
                void trade_from_input(const kungfu::wingchun::msg::data::OrderInput &, kungfu::wingchun::msg::data::Trade &);
            };

             void volume_price_generator(kungfu::wingchun::msg::data::Trade &, const kungfu::wingchun::msg::data::Quote &, const int);
        }
    }
}

#endif //WINGCHUN_CONTEXT_H
