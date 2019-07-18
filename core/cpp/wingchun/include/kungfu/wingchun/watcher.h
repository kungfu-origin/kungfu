//
// Created by Keren Dong on 2019-06-28.
//

#ifndef WINGCHUN_WATCHER_H
#define WINGCHUN_WATCHER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        class Watcher : public practice::apprentice
        {
        public:
            explicit Watcher(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency = false);

            virtual ~Watcher() = default;

            void publish(const std::string &msg);

            void publish_state(int64_t trigger_time, yijinjing::data::category c, const std::string &group, const std::string &name,
                               msg::data::GatewayState state);

            virtual std::string handle_request(const std::string &msg) = 0;

            virtual void on_quote(yijinjing::event_ptr event, const msg::data::Quote &quote) = 0;

            virtual void on_order(yijinjing::event_ptr event, const msg::data::Order &order) = 0;

            virtual void on_trade(yijinjing::event_ptr event, const msg::data::Trade &trade) = 0;

            virtual void on_assets(const msg::data::AssetInfo& info, const std::vector<msg::data::Position>& positions) = 0;

            virtual void pre_start() = 0;

        protected:

            void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

            void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

            void on_write_to(const yijinjing::event_ptr &event) override;

            void on_read_from(const yijinjing::event_ptr &event) override;

            void on_start() override;

        private:
            yijinjing::nanomsg::socket_ptr pub_sock_;

            std::unordered_map<std::string, int> accounts_;

            void watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

            msg::data::AssetInfo asset_info_;
            std::vector<msg::data::Position> position_buffer_;

        };
    }
}

#endif //WINGCHUN_WATCHER_H
