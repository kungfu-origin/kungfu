//
// Created by qlu on 2019-10-31.
//

#ifndef KUNGFU_WINGCHUN_ALGO_SERVICE_H
#define KUNGFU_WINGCHUN_ALGO_SERVICE_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/algo/algo.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            class Algo: public practice::apprentice
            {
            public:
                explicit Algo(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency = false);

                virtual ~Algo() = default;

                virtual void insert_order(const yijinjing::event_ptr& e, const std::string& msg) = 0;

                virtual void cancel_order(const yijinjing::event_ptr& e, const msg::data::OrderAction& action) = 0;

                virtual void modify_order(const yijinjing::event_ptr& e, const std::string& msg) = 0;

                void on_start() override ;

                void add_order(const algo::AlgoOrder_ptr& order) { context_->add_order(order); }

                algo::AlgoContext_ptr get_algo_context() const { return context_; }
                                
            protected:
                void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

            private:
                algo::AlgoContext_ptr context_;
            };

            DECLARE_PTR(Algo)
        }
    }
}
#endif //KUNGFU_WINGCHUN_ALGO_SERVICE_H
