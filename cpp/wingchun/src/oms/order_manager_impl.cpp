//
// Created by PolarAir on 2019-02-28.
//

#ifndef KUNGFU_ORDER_MANAGER_H
#define KUNGFU_ORDER_MANAGER_H

#include <kungfu/wingchun/oms/def.h>
#include <kungfu/wingchun/util/business_helper.h>

#include <unordered_map>
#include <iostream>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    namespace oms
    {
        class OrderManagerImpl: public OrderManager
        {
        public:
            OrderManagerImpl() {}
            virtual ~OrderManagerImpl() {}

            void on_order(const Order* order)
            {
                if (order->parent_id != 0)
                {
                    return;
                }
                else
                {
                    if (orders_.find(order->order_id) == orders_.end())
                    {
                        auto simple_order = make_simple_order(*order);
                        orders_[order->order_id] = simple_order;
                    }
                    else
                    {
                        auto simple_order = std::dynamic_pointer_cast<SimpleOrder>(orders_[order->order_id]);
                        simple_order->on_order(order);
                    }
                }
            };

            void on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& order_status)
            {
                auto it = orders_.find(order_id);
                if (it == orders_.end())
                {
                    AlgoOrderPtr order = AlgoOrderFactory::get()->create_order(algo_type);
                    if (order != nullptr)
                    {
                        order->set_order_id(order_id);
                        order->loads(order_status);
                        orders_[order->get_order_id()] = order;
                    }
                }
                else
                {
                    std::dynamic_pointer_cast<AlgoOrder>(it->second)->loads(order_status);
                }

            }

            OrderPtr get_order(uint64_t order_id) const
            {
                //TODO
                return nullptr;
            }

            std::vector<OrderPtr> get_pending_orders() const
            {
                std::vector<OrderPtr> order_vec;
                for (const auto& element: orders_)
                {
                    if (element.second->is_active())
                    {
                        order_vec.push_back(element.second);
                    }
                }
                return order_vec;
            }

        private:
            std::unordered_map<uint64_t, OrderPtr> orders_;
        };

        OrderManagerPtr create_order_manager()
        {
            return OrderManagerPtr(new OrderManagerImpl());
        }
    }
}
#endif //KUNGFU_ORDER_MANAGER_H
