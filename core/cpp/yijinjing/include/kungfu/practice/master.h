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

        protected:
            void react(const rx::observable<yijinjing::event_ptr> &events) override;
        };
    }
}
#endif //KUNGFU_MASTER_H
