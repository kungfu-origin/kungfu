#ifndef MD_KFEXT_DEMO_H
#define MD_KFEXT_DEMO_H

#include <string>
#include <map>

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/marketdata.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            class MarketDataDemo : public broker::MarketData
            {
            public:
                MarketDataDemo(bool low_latency, yijinjing::data::locator_ptr locator,
                          std::map<std::string, std::string> &config_str,
                          std::map<std::string, int> &config_int,
                          std::map<std::string, double> &config_double);

                void on_start() override;

                bool subscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override;

                bool unsubscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override;
            };
        }
    }
}

#endif //MD_KFEXT_DEMO_H
