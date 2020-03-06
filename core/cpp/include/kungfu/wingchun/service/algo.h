//
// Created by qlu on 2019-10-31.
//

#ifndef KUNGFU_WINGCHUN_ALGO_SERVICE_H
#define KUNGFU_WINGCHUN_ALGO_SERVICE_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/algo/algo.h>

namespace kungfu::wingchun::service
{
    class Algo : public yijinjing::practice::apprentice
    {
    public:
        explicit Algo(yijinjing::data::locator_ptr locator, longfist::enums::mode m, bool low_latency = false);

        virtual ~Algo() = default;

        virtual void insert_order(const event_ptr &e, const std::string &msg) = 0;

        virtual void cancel_order(const event_ptr &e, const longfist::types::OrderAction &action) = 0;

        virtual void modify_order(const event_ptr &e, const std::string &msg) = 0;

        void on_start() override;

        void add_order(const algo::AlgoOrder_ptr &order)
        { context_->add_order(order); }

        algo::AlgoContext_ptr get_algo_context() const
        { return context_; }

    private:
        algo::AlgoContext_ptr context_;
    };

    DECLARE_PTR(Algo)
}
#endif //KUNGFU_WINGCHUN_ALGO_SERVICE_H
