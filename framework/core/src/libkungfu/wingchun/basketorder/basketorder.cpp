// SPDX-License-Identifier: Apache-2.0

#include <kungfu/wingchun/basketorder/basketorder.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::basketorder {
BasketOrderState::BasketOrderState(uint32_t source, uint32_t dest, int64_t trigger_time,
                                   const longfist::types::BasketOrder &basket_order)
    : state_data(state<BasketOrder>(source, dest, trigger_time, basket_order)){};

void BasketOrderState::update(const longfist::types::Order &order) { orders.insert_or_assign(order.order_id, order); }

const state<longfist::types::BasketOrder> &BasketOrderState::get_state() { return state_data; }
} // namespace kungfu::wingchun::basketorder