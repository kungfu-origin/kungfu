//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_MARKETDATA_H
#define WINGCHUN_MARKETDATA_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            class MarketData : public practice::apprentice
            {
            public:
                explicit MarketData(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source);

                virtual ~MarketData() = default;

                virtual bool subscribe(const std::vector<msg::data::Instrument> &instruments) = 0;

                virtual bool unsubscribe(const std::vector<msg::data::Instrument> &instruments) = 0;

            protected:
                void react(const rx::observable<yijinjing::event_ptr> &events) override;

                void on_start(const rx::observable<yijinjing::event_ptr> &events) override;
            };
        }
    }
}

#endif //WINGCHUN_MARKETDATA_H
