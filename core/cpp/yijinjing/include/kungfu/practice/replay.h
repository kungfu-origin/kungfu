//
// Created by Keren Dong on 2019-07-06.
//

#ifndef KUNGFU_REPLAY_H
#define KUNGFU_REPLAY_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace practice
    {
        class replay : public apprentice
        {
        public:
            replay(const yijinjing::data::location_ptr &home, bool low_latency = false);

            replay(apprentice_ptr app);

        protected:

            void react(const rx::observable<yijinjing::event_ptr> &events) override;

        private:

            apprentice_ptr app_;
        };
    }
}
#endif //KUNGFU_REPLAY_H
