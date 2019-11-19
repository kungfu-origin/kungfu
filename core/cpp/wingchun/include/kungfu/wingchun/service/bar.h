#ifndef KF_SERVICE_BAR_GENERATOR
#define KF_SERVICE_BAR_GENERATOR

#include <kungfu/practice/apprentice.h>
#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <unordered_set>

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            class BarGenerator:  public practice::apprentice
            {
            public:
                BarGenerator(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency, const std::string &json_config);
                bool subscribe(const std::vector<Instrument> &instruments);
                void on_start() override;
            protected:
                void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;
            private:
                location_ptr source_location_;
                int64_t time_interval_;
                std::unordered_map<uint32_t, Bar> bars_;
            };
        }
    }
}

#endif //KF_SERVICE_BAR_GENERATOR
