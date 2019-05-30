//
// Created by qlu on 2019/3/3.
//

#include <kungfu/wingchun/oms/def.h>
#include <kungfu/wingchun/util/business_helper.h>

using namespace kungfu::journal;

namespace kungfu
{
    namespace oms
    {
        class SimpleOrderImpl: public SimpleOrder
        {
        public:
            SimpleOrderImpl(const OrderInput& input): util_(nullptr)
            {
                memcpy(&input_, &input, sizeof(OrderInput));
                Order order = get_order(input);
                memcpy(&order_, &order, sizeof(Order));
            };

            SimpleOrderImpl(const Order& order): util_(nullptr)
            {
                on_order(&order);
            };

            virtual ~SimpleOrderImpl() {}

            uint64_t get_order_id() const { return order_.order_id; }

            std::string get_account_id() const { return order_.account_id; }

            std::string get_client_id() const { return order_.client_id; }

            void execute()
            {
                auto order_id = get_util()->submit(input_);
                input_.order_id = order_id;
            }
            
             void cancel()
            {
                get_util()->cancel(get_order_id());
            }

            void set_util(SimpleOrderUtil* util)
            {
                util_ = util;
            };

            SimpleOrderUtil* get_util() const { return util_; };

            bool is_algo() const { return false; }
            bool is_active() const { return !is_final_status(get_status()); }

            OrderStatus get_status() const { return order_.status; }
            bool is_children_order() const { return order_.parent_id != 0; };
            uint64_t get_parent_id() const { return order_.parent_id; };

            void on_order(const Order *order)
            {
                memcpy(&order_, order, sizeof(Order));
            };

        private:
            SimpleOrderUtil* util_;
            OrderInput input_;
            Order order_;
        };

        SimpleOrderPtr make_simple_order(const OrderInput& input)
        {
            return  SimpleOrderPtr(new SimpleOrderImpl(input));
        }

        SimpleOrderPtr make_simple_order(const Order& order)
        {
            return SimpleOrderPtr(new SimpleOrderImpl(order));
        }
    }
}
