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
        class master : public hero
        {
        public:
            master(yijinjing::data::location_ptr home, bool low_latency = false);

            void on_notify() override ;

            virtual void on_notice(yijinjing::event_ptr event)
            {}

            void register_app(yijinjing::event_ptr e);

            void deregister_app(int64_t trigger_time, uint32_t app_location_uid);

        protected:
            void react(const rx::observable<yijinjing::event_ptr> &events) override;

        private:
            int64_t last_check_;
            std::unordered_map<uint32_t, uint32_t> app_locations_;
        };
    }
}
#endif //KUNGFU_MASTER_H
