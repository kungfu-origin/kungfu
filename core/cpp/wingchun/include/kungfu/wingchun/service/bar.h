#ifndef KF_SERVICE_BAR_GENERATOR
#define KF_SERVICE_BAR_GENERATOR

#include <kungfu/practice/apprentice.h>
#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>

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
                void request_subscribe(int64_t start_time, int64_t end_time, const Instrument& instrument);
            };
        }
    }
}

#endif //KF_SERVICE_BAR_GENERATOR
