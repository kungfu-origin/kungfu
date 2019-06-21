//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/util/uid_generator.h>
#include <kungfu/wingchun/portfolio/portfolio_manager.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            class Runner;

            class Context
            {
            public:
                explicit Context(practice::apprentice &app);

                ~Context() = default;

                //获取当前纳秒时间
                //@return            当前纳秒时间
                int64_t now() const;

                //添加策略使用的行情柜台
                //@param source_id   柜台ID
                //@return            成功或者失败
                bool add_md(const std::string &source_id);

                //添加策略使用的交易账户
                //@param source_id   柜台ID
                //@param account_id  账户ID
                //@param cash_limit  可用资金限制
                //@return            成功或者失败
                bool add_account(const std::string &source_id, const std::string &account_id, double cash_limit);

                //订阅行情
                //@param source_id   柜台ID
                //@param instruments 合约列表
                //@param exchange_id 交易所ID
                //@param is_level2   是否订阅Level2数据
                void subscribe(const std::string &source_id, const std::vector<std::string> &instruments, const std::string &exchange_id = "",
                               bool is_level2 = false);

                //限价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param limit_price   价格
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_limit_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &account_id,
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
                uint64_t insert_fok_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &account_id,
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
                uint64_t insert_fak_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &account_id,
                                          double limit_price, int64_t volume, Side side, Offset offset);

                //市价单报单
                //@param instrument_id 合约ID
                //@param exchange_id   交易所ID
                //@param account_id    账户ID
                //@param volume        数量
                //@param side          买卖方向
                //@param offset        开平方向
                //@return              订单ID
                uint64_t insert_market_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &account_id,
                                             int64_t volume, Side side, Offset offset);

                //撤单
                //@param order_id      订单ID
                //@return              撤单操作ID
                uint64_t cancel_order(uint64_t order_id);

            protected:
                void react(rx::observable<yijinjing::event_ptr> events);

            private:
                uint64_t next_id();

            private:
                practice::apprentice& app_;

                storage::UidWorkerStorage uid_worker_storage_;
                storage::OrderStorage order_storage_;
                storage::TradeStorage trade_storage_;

                Calendar calendar_;
                UidGenerator uid_generator_;

                std::map<std::string, AccountManager_ptr> account_managers_;
                std::map<std::string, msg::data::AccountInfo> accounts_;

                bool has_stock_account_;
                bool has_future_account_;
                std::map<std::string, std::set<std::string>> subscribed_;

                friend class Runner;
            };
        }
    }
}

#endif //WINGCHUN_CONTEXT_H
