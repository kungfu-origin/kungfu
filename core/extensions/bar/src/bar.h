#ifndef KF_EXT_BAR_GENERATOR
#define KF_EXT_BAR_GENERATOR

#include <kungfu/wingchun/broker/marketdata.h>
#include <kungfu/yijinjing/common.h>

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace bar
        {
            typedef std::shared_ptr<Bar> BarPtr;

            class BarGenerator:  public broker::MarketData
            {
            public:
                BarGenerator(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config);
                bool subscribe(const std::vector<Instrument> &instruments) override;
                bool unsubscribe(const std::vector<Instrument> &instruments) override { return true; };
                void on_start() override;
            private:
                location_ptr source_location_;
                int64_t time_interval_;
                void request_subscribe(int64_t start_time, int64_t end_time, const Instrument& instrument);
            };
        }
    }
}

#endif //KF_EXT_BAR_GENERATOR