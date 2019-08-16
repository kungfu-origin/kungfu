//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_TRADER_H
#define EXT_PASSIVE_TRADER_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            class TraderSim : public broker::Trader
            {
            public:
                TraderSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config);

                const AccountType get_account_type() const override
                { return AccountType::Stock; }

                bool insert_order(const yijinjing::event_ptr &event) override;

                bool cancel_order(const yijinjing::event_ptr &event) override;

                bool req_position() override;

                bool req_account() override;

                void on_trading_day(const yijinjing::event_ptr &event, int64_t daytime) override
                {};

                void on_start() override;

                void on_md(const msg::data::Quote &quote, uint32_t source);

                void add_md(std::string source_);

                void rtn_order_from(const msg::data::OrderInput &input, OrderStatus status, int error_id = 0);

            private:
                std::map<std::string, msg::data::Quote> mds_;
                std::map<int, msg::data::Order> pending_orders_;
                int order_id_ = 0;
            };
        }
    }
}

#endif //EXT_PASSIVE_TRADER_H
