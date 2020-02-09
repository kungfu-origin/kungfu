//
// Created by Keren Dong on 2020/2/9.
//

#ifndef KUNGFU_WATCHER_H
#define KUNGFU_WATCHER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::yijinjing::practice
{
    class watcher : public yijinjing::practice::apprentice
    {
    public:
        explicit watcher(yijinjing::data::location_ptr &location, bool low_latency = false);

        virtual ~watcher() = default;

        void publish(const std::string &msg);

        void publish_broker_states(int64_t trigger_time);

    protected:

        void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

        void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

        void on_write_to(const event_ptr &event) override;

        void on_read_from(const event_ptr &event) override;

        void on_start() override;

        void publish_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, longfist::BrokerState state);

        void update_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, longfist::BrokerState state);

    private:
        yijinjing::nanomsg::socket_ptr pub_sock_;
    };
}

#endif //KUNGFU_WATCHER_H
