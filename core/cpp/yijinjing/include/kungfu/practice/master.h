//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_MASTER_H
#define KUNGFU_MASTER_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/hero.h>

namespace kungfu
{
    namespace practice
    {
        struct TimerTask
        {
            int64_t checkpoint;
            int64_t duration;
            int64_t repeat_limit;
            int64_t repeat_count;
        };

        class master : public hero
        {
        public:
            master(yijinjing::data::location_ptr home, bool low_latency = false);

            void on_notify() override ;

            virtual void on_notice(const yijinjing::event_ptr &event)
            {}

            virtual void on_interval_check(int64_t nanotime)
            {}

            virtual void on_register(const yijinjing::event_ptr &event)
            {}

            void register_app(const yijinjing::event_ptr& e);

            void deregister_app(int64_t trigger_time, uint32_t app_location_uid);

            void publish_time(int32_t msg_type, int64_t nanotime);
            void send_time(uint32_t dest, int32_t msg_type, int64_t nanotime);

        protected:

            bool produce_one(const rx::subscriber<yijinjing::event_ptr> &sb) override ;

            void react() override;

        private:
            int64_t last_check_;
            std::unordered_map<uint32_t, uint32_t> app_locations_;
            std::unordered_map<uint32_t, std::unordered_map<int32_t, TimerTask>> timer_tasks_;
        };
    }
}
#endif //KUNGFU_MASTER_H
