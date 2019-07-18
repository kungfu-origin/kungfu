//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_TRADER_H
#define EXT_PASSIVE_TRADER_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/gateway/trader.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            class PassiveTrader : public gateway::Trader
            {
            public:
                PassiveTrader(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str, std::map<std::string, int> &config_int,
                          std::map<std::string, double> &config_double);

                const AccountType get_account_type() const override
                { return AccountType::Stock; }

                bool insert_order(const yijinjing::event_ptr& event) override ;

                bool cancel_order(const yijinjing::event_ptr& event) override ;

                bool req_position() override ;

                bool req_account() override ;
                
            private:
                void on_start() override;
                void on_passivectrl(const yijinjing::event_ptr& event);
            };
        }
    }
}

#endif //EXT_PASSIVE_TRADER_H
