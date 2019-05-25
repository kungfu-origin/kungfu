//
// Created by qlu on 2019/4/2.
//

#include <kungfu/wingchun/oms/def.h>

namespace kungfu
{
    namespace oms
    {
        class AlgoOrder::impl
        {
        public:
            impl(): order_id_(0), util_(nullptr) {}
            AlgoOrderUtil* get_util() const { return util_;};
            void set_util(AlgoOrderUtil* util) { util_ = util; }
            void set_order_id(uint64_t order_id) { order_id_ = order_id; }
            uint64_t get_order_id() const { return order_id_;};
        private:
            uint64_t order_id_;
            AlgoOrderUtil* util_;
        };

        AlgoOrder::AlgoOrder(): impl_(new impl()) {}
        AlgoOrder::~AlgoOrder() {}

        void AlgoOrder::set_order_id(uint64_t order_id)
        {
            impl_->set_order_id(order_id);
        }

        uint64_t AlgoOrder::get_order_id() const
        {
            return impl_->get_order_id();
        }

        AlgoOrderUtil* AlgoOrder::get_util() const
        {
            return impl_->get_util();
        };

        void AlgoOrder::set_util(AlgoOrderUtil* util)
        {
            impl_->set_util(util);
        };
    }
}
