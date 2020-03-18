#ifndef MD_KFEXT_DEMO_H
#define MD_KFEXT_DEMO_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/broker/marketdata.h>

namespace kungfu::wingchun::kfext_demo
{
    class MarketDataDemo : public broker::MarketData
    {
    public:
        MarketDataDemo(bool low_latency, yijinjing::data::locator_ptr locator,
                       std::map<std::string, std::string> &config_str,
                       std::map<std::string, int> &config_int,
                       std::map<std::string, double> &config_double);

        void on_start() override;

        bool subscribe(const std::vector<longfist::types::Instrument> &instruments) override;

        bool subscribe_all() override;

        bool unsubscribe(const std::vector<longfist::types::Instrument> &instruments) override;
    };
}

#endif //MD_KFEXT_DEMO_H
