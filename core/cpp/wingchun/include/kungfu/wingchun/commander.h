//
// Created by Keren Dong on 2019-08-10.
//

#ifndef KUNGFU_WINGCHUN_COMMANDER_H
#define KUNGFU_WINGCHUN_COMMANDER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        class Commander : public practice::apprentice
        {
        public:
            explicit Commander(yijinjing::data::locator_ptr locator, bool low_latency = false, const std::string &name = "ledger");

            virtual ~Commander() = default;

            void new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input);

            void cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id);

            virtual std::string handle_request(const yijinjing::event_ptr &event, const std::string &msg) = 0;

            virtual void on_order(yijinjing::event_ptr event, const msg::data::Order &order) = 0;

        protected:

            void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

            void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

            void on_write_to(const yijinjing::event_ptr &event) override;

            void on_read_from(const yijinjing::event_ptr &event) override;

            void on_start() override;

        private:

            void connect(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

        };
    }
}
#endif //KUNGFU_WINGCHUN_COMMANDER_H
