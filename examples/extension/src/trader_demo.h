#ifndef TD_KFEXT_DEMO_H
#define TD_KFEXT_DEMO_H

#include <memory>
#include <string>
#include <map>

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/trader.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            class TraderDemo : public broker::Trader
            {
            public:
                TraderDemo(bool low_latency, yijinjing::data::locator_ptr locator,
                          std::map<std::string, std::string> &config_str,
                          std::map<std::string, int> &config_int,
                          std::map<std::string, double> &config_double);

                const AccountType get_account_type() const override
                { return AccountType::Stock; }

                void on_start() override;

                bool insert_order(const yijinjing::event_ptr &event) override;

                bool cancel_order(const yijinjing::event_ptr &event) override;

                bool req_position() override;

                bool req_account() override;
            };
        }
    }
}
#endif //TD_KFEXT_DEMO_H
