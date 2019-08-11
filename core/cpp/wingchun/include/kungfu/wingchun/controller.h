//
// Created by Keren Dong on 2019-08-10.
//

#ifndef KUNGFU_WINGCHUN_CONTROLLER_H
#define KUNGFU_WINGCHUN_CONTROLLER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        class Controller : public practice::apprentice
        {
        public:
            explicit Controller(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency = false);

            virtual ~Controller() = default;

            virtual std::string handle_request(const std::string &msg) = 0;

        protected:

            void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

            void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

            void on_write_to(const yijinjing::event_ptr &event) override;

            void on_read_from(const yijinjing::event_ptr &event) override;

            void on_start() override;

        private:

            void watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

        };
    }
}
#endif //KUNGFU_WINGCHUN_CONTROLLER_H
