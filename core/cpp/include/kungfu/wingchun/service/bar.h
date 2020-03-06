#ifndef KF_SERVICE_BAR_GENERATOR
#define KF_SERVICE_BAR_GENERATOR

#include <unordered_set>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/common.h>

namespace kungfu::wingchun::service
{
    class BarGenerator : public yijinjing::practice::apprentice
    {
    public:
        BarGenerator(yijinjing::data::locator_ptr locator, longfist::enums::mode m, bool low_latency, const std::string &json_config);

        bool subscribe(const std::vector<longfist::types::Instrument> &instruments);

        void on_start() override;

    private:
        yijinjing::data::location_ptr source_location_;
        int64_t time_interval_;
        std::unordered_map<uint32_t, longfist::types::Bar> bars_;
    };
}

#endif //KF_SERVICE_BAR_GENERATOR
