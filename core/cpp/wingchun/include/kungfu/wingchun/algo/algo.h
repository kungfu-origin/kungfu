//
// Created by qlu on 2019-10-26.
//

#ifndef KUNGFU_WINGCHUN_ALGO_H
#define KUNGFU_WINGCHUN_ALGO_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace algo
        {
            FORWARD_DECLARE_PTR(AlgoContext)

            class AlgoOrder
            {
            public:
                AlgoOrder(uint64_t order_id = 0): order_id_(order_id) {};

                virtual ~AlgoOrder() = default;

                uint64_t get_order_id() const { return order_id_; }

                virtual const std::string dumps() const = 0;

                virtual void on_start(AlgoContext_ptr context) {};

                virtual void on_stop(AlgoContext_ptr context) {};

                virtual void on_child_order(AlgoContext_ptr context, const msg::data::Order& order) {};

                virtual void on_child_trade(AlgoContext_ptr context, const msg::data::Trade& trade) {};

                virtual void on_quote(AlgoContext_ptr context, const msg::data::Quote& quote) {};

                virtual void on_order_report(AlgoContext_ptr context, const std::string& report_msg) {};

            private:
                uint64_t order_id_;
                friend class AlgoContext;
            };

            DECLARE_PTR(AlgoOrder)

            class AlgoContext : public std::enable_shared_from_this<AlgoContext>
            {
            public:
                explicit AlgoContext(practice::apprentice &app, const rx::connectable_observable <yijinjing::event_ptr> &events);

                void react();

                uint64_t add_order(const AlgoOrder_ptr& order);

                uint64_t cancel_order(const AlgoOrder_ptr& order);

                bool has_order(uint64_t order_id) const
                {
                    return orders_.find(order_id) != orders_.end();
                }

                int64_t now() const { return app_.now(); }

                void add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)> &callback) {  app_.add_timer(nanotime, callback); }

                void subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string &exchange = "");

                uint64_t insert_order(const msg::data::OrderInput& order);

                uint64_t cancel_order(uint64_t order_id);

                yijinjing::data::location_ptr get_service_location() const { return service_location_; }

            private:
                practice::apprentice &app_;
                const rx::connectable_observable<yijinjing::event_ptr> &events_;
                yijinjing::data::location_ptr service_location_;
                std::unordered_map<uint64_t, AlgoOrder_ptr> orders_;

            };

        }
    }
}

#endif //KUNGFU_WINGCHUN_ALGO_H
