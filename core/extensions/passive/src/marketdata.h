//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_MARKETDATA_H
#define EXT_PASSIVE_MARKETDATA_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/marketdata.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            class PassiveMarketData : public gateway::MarketData
            {
            public:
                PassiveMarketData(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str,
                                  std::map<std::string, int> &config_int,
                                  std::map<std::string, double> &config_double);

                bool subscribe(const std::vector<msg::data::Instrument> &instruments) override ;

                bool unsubscribe(const std::vector<msg::data::Instrument> &instruments) override ;

            protected:

                void on_start() override;

                void write_1();
            private:
                void sin_quota(msg::data::Quote &quote, double cycle, double amplitude, double initial_value, double pass_time);
            };
        }
    }
}

#endif //EXT_PASSIVE_MARKETDATA_H
