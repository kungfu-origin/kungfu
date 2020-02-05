//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_MARKETDATA_H
#define WINGCHUN_MARKETDATA_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker
{
    class MarketData : public yijinjing::practice::apprentice
    {
    public:
        explicit MarketData(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source);

        virtual ~MarketData() = default;

        virtual void on_start() override;

        virtual bool subscribe(const std::vector<longfist::types::Instrument> &instruments) = 0;

        virtual bool subscribe_all() = 0;

        virtual bool unsubscribe(const std::vector<longfist::types::Instrument> &instruments) = 0;

    protected:
        void publish_state(longfist::BrokerState state)
        {
            auto s = static_cast<int32_t>(state);
            write_to(0, longfist::types::BrokerStateUpdate::tag, s);
        }
    };
}

#endif //WINGCHUN_MARKETDATA_H
