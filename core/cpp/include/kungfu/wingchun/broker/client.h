//
// Created by Keren Dong on 2020/3/12.
//

#ifndef WINGCHUN_BROKER_CLIENT_H
#define WINGCHUN_BROKER_CLIENT_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/broker/broker.h>

namespace kungfu::wingchun::broker
{
    class Client
    {
    public:
        explicit Client(yijinjing::practice::apprentice &app);

        [[nodiscard]] const std::unordered_map<uint32_t, longfist::types::Instrument> &get_instruments() const;

        void add(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id);

        void add_all(const yijinjing::data::location_ptr &md_location);

        [[nodiscard]] bool is_subscribed(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id) const;

        [[nodiscard]] bool is_all_subscribed(const yijinjing::data::location_ptr &md_location) const;

        void subscribe(const rx::connectable_observable<event_ptr> &events);

    private:
        yijinjing::practice::apprentice &app_;
        std::unordered_map<uint32_t, bool> md_locations_;
        std::unordered_map<uint32_t, yijinjing::data::location_ptr> instrument_md_locations_;
        std::unordered_map<uint32_t, longfist::types::Instrument> instruments_;

        std::unordered_map<uint32_t, longfist::enums::BrokerState> broker_states_;
        std::unordered_map<uint32_t, yijinjing::data::location_ptr> subscribed_md_locations_;

        [[nodiscard]] uint32_t hash_instrument(const longfist::types::Instrument &instrument) const;

        [[nodiscard]] bool should_subscribe(const yijinjing::data::location_ptr &md_location) const;

        void connect(const longfist::types::Register& register_data);
    };
}

#endif //WINGCHUN_BROKER_CLIENT_H
