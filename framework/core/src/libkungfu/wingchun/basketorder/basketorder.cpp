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

void BasketOrderState::update_success_ordered_volume(const longfist::types::Order &order, int32_t instrument_key) {

  bool is_new_order = (orders.find(order.order_id) == orders.end());
  int64_t success_ordered_volume;
  auto it = success_ordered_volume_map.find(instrument_key);
  if (it != success_ordered_volume_map.end()) {
    success_ordered_volume = it->second;
  } else {
    success_ordered_volume = 0;
  }
  if (is_new_order) {
    if (order.status == OrderStatus::Error) {
      // do nothing
    } else if (order.status == OrderStatus::PartialFilledNotActive || order.status == OrderStatus::Cancelled) {
      success_ordered_volume += order.volume - order.volume_left;
    } else {
      success_ordered_volume += order.volume;
    }
  } else {
    if (order.status == OrderStatus::Error || order.status == OrderStatus::PartialFilledNotActive ||
        order.status == OrderStatus::Cancelled) {
      success_ordered_volume -= order.volume_left;
    }
  }
  success_ordered_volume_map.insert_or_assign(instrument_key, success_ordered_volume);
}

void BasketOrderState::update(const longfist::types::Order &order) {
  int32_t instrument_key = hash_instrument(order.exchange_id, order.instrument_id);
  auto direction = get_direction(order.instrument_type, order.side, order.offset);
  if (direction == Direction::Short) {
    instrument_key = instrument_key * (-1);
  }
  auto &basket_order = state_data.data;
  int64_t total_volume = 0;
  // if it is a new order
  if (orders.find(order.order_id) == orders.end()) {
    int64_t ordered_volume = 0;
    auto it = success_ordered_volume_map.find(instrument_key);
    if (it != success_ordered_volume_map.end()) {
      ordered_volume = it->second;
    } else {
      ordered_volume = 0;
    }
    ordered_volume += order.volume;
    ordered_volume_map.insert_or_assign(instrument_key, ordered_volume);
    for (auto &iter : ordered_volume_map) {
      total_volume += iter.second;
    }
  } else {
    total_volume = basket_order.volume;
  }
  update_success_ordered_volume(order, instrument_key);
  orders.insert_or_assign(order.order_id, order);

  // after supplementing order, the total volume may be changed, bigger than the original volume
  // for algo trade, the volume is hard to be cacluated, required to be set at init;
  basket_order.volume = total_volume;
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

OrderMap &BasketOrderState::get_orders() { return orders; }

} // namespace kungfu::wingchun::basketorder