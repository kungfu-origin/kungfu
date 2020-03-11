//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_MARKETDATA_H
#define WINGCHUN_MARKETDATA_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/broker/broker.h>

namespace kungfu::wingchun::broker
{
    class MarketData : public Broker
    {
    public:
        explicit MarketData(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source);

        virtual ~MarketData() = default;

        virtual void on_start() override;

        virtual bool subscribe(const std::vector<longfist::types::Instrument> &instruments) = 0;

        virtual bool subscribe_all() = 0;

        virtual bool unsubscribe(const std::vector<longfist::types::Instrument> &instruments) = 0;
    };

    class Subscription
    {
    public:
        explicit Subscription(yijinjing::practice::apprentice &app);

        [[nodiscard]] const std::unordered_map<uint32_t, longfist::types::Instrument> &get_instruments() const;

        void add(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id);

        void add_all(const yijinjing::data::location_ptr &md_location);

        bool is_subscribed(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id) const;

        bool is_all_subscribed(const yijinjing::data::location_ptr &md_location) const;

        void subscribe(const rx::connectable_observable<event_ptr> &events);

    private:
        yijinjing::practice::apprentice &app_;
        std::unordered_map<uint32_t, bool> md_locations_;
        std::unordered_map<uint32_t, yijinjing::data::location_ptr> instrument_md_locations_;
        std::unordered_map<uint32_t, longfist::types::Instrument> instruments_;

        std::unordered_map<uint32_t, longfist::enums::BrokerState> broker_states_;
        std::unordered_map<uint32_t, yijinjing::data::location_ptr> subscribed_md_locations_;

        uint32_t hash_instrument(const longfist::types::Instrument &instrument) const;

        bool should_subscribe(const yijinjing::data::location_ptr &md_location) const;

        void connect(const longfist::types::Register& register_data);
    };
}

#endif //WINGCHUN_MARKETDATA_H
