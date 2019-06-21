//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_MARKETDATA_H
#define WINGCHUN_MARKETDATA_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

#include <kungfu/wingchun/gateway/subscription_storage.hpp>

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            class MarketData : public practice::apprentice
            {
            public:
                MarketData(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source);

                ~MarketData();

                virtual bool subscribe(const std::vector<msg::data::Instrument> &instruments, bool is_level2) = 0;

                virtual bool unsubscribe(const std::vector<msg::data::Instrument> &instruments) = 0;

                SubscriptionStorage& get_subscription_storage()
                { return subscription_storage_; }

            protected:
                void react(rx::observable<yijinjing::event_ptr> events) override;

                void start() override ;

            private:
                SubscriptionStorage subscription_storage_;
            };
        }
    }
}

#endif //WINGCHUN_MARKETDATA_H
