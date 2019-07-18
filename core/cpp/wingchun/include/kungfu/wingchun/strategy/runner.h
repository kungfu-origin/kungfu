//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_RUNNER_H
#define WINGCHUN_RUNNER_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/strategy.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            class Runner : public practice::apprentice
            {
            public:
                Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name, yijinjing::data::mode m, bool low_latency);

                virtual ~Runner() = default;

                void add_strategy(const Strategy_ptr& strategy);

            protected:

                void on_start() override ;

            private:
                Context_ptr context_;
                std::vector<Strategy_ptr> strategies_;
            };
        }
    }
}

#endif //WINGCHUN_RUNNER_H
