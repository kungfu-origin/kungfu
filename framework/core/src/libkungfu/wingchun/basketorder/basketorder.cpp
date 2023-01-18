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

void BasketOrderState::update(const longfist::types::Order &order) {

  orders.insert_or_assign(order.order_id, order);
  auto &basket_order = state_data.data;

  // after supplementing order, the total volume may be changed, bigger than the original volume
  basket_order.volume = std::max(basket_order.volume, get_merged_total_volume(orders));
  basket_order.volume_left = basket_order.volume - get_total_traded_volume(orders);
  auto is_all_order_end_val = is_all_order_end(orders);
  auto is_all_order_filled_val = basket_order.volume_left == 0;
  if (is_all_order_end_val and is_all_order_filled_val) {
    basket_order.status = BasketOrderStatus::Filled;
  } else if (is_all_order_end_val) {
    basket_order.status = BasketOrderStatus::PartialFilledNotActive;
  } else if (basket_order.volume_left < basket_order.volume) {
    basket_order.status = BasketOrderStatus::PartialFilledActive;
  } else {
    basket_order.status = BasketOrderStatus::Pending;
  }
}

state<longfist::types::BasketOrder> &BasketOrderState::get_state() { return state_data; }

} // namespace kungfu::wingchun::basketorder