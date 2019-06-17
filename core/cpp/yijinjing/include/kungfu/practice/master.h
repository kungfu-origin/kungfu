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
        class master : public hero, public yijinjing::event_handler, public std::enable_shared_from_this<master>
        {
        public:
            master(yijinjing::data::location_ptr home, bool low_latency = false);

            void subscribe(const yijinjing::data::location_ptr location) override {}

            const std::string &get_name() const override
            { return get_io_device()->get_home()->name; }

            void configure_event_source(yijinjing::event_source_ptr event_source) override
            {}

            void handle(const yijinjing::event_ptr e) override;

            void finish() override
            {}
        };
    }
}
#endif //KUNGFU_MASTER_H
