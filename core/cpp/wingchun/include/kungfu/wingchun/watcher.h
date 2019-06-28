//
// Created by Keren Dong on 2019-06-28.
//

#ifndef WINGCHUN_WATCHER_H
#define WINGCHUN_WATCHER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace kungfu
{
    namespace wingchun
    {
        class Watcher : public practice::apprentice
        {
        public:
            explicit Watcher(bool low_latency, yijinjing::data::locator_ptr locator);

            virtual ~Watcher() = default;

            void publish(const std::string &msg);

            void publish_state(int64_t trigger_time, yijinjing::data::category c, const std::string &group, const std::string &name,
                               msg::data::GatewayState state);

            virtual std::string handle_request(const std::string &msg) = 0;

            virtual std::string on_quote(yijinjing::event_ptr event, const msg::data::Quote &quote) = 0;

            virtual std::string on_order(yijinjing::event_ptr event, const msg::data::Order &order) = 0;

            virtual std::string on_trade(yijinjing::event_ptr event, const msg::data::Trade &trade) = 0;

        protected:
            void react(const rx::observable<yijinjing::event_ptr> &events) override;

            void on_write_to(const yijinjing::event_ptr &event) override;

            void on_read_from(const yijinjing::event_ptr &event) override;

            void start() override;

        private:
            yijinjing::nanomsg::socket_ptr pub_sock_;
            Calendar calendar_;

            std::unordered_map<std::string, int> accounts_;
        };
    }
}

#endif //WINGCHUN_WATCHER_H
