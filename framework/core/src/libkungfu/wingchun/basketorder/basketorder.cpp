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

void BasketOrderState::update_success_ordered_volume(const longfist::types::Order &order) {
  int32_t instrument_key = hash_instrument(order);
  auto success_ordered_volume = get_success_ordered_volume(order);
  bool is_final = is_final_status(order.status);

  if (orders.find(order.order_id) == orders.end()) {
    if (is_final) {
      success_ordered_volume += order.volume - order.volume_left;
    } else {
      success_ordered_volume += order.volume;
    }
  } else if (is_final) {
    success_ordered_volume -= order.volume_left;
  }

  success_ordered_volume_map.insert_or_assign(instrument_key, success_ordered_volume);
}

void BasketOrderState::update_ordered_volume(const longfist::types::Order &order) {
  if (orders.find(order.order_id) != orders.end()) {
    return;
  }

  int32_t instrument_key = hash_instrument(order);
  int64_t ordered_volume = get_success_ordered_volume(order);
  ordered_volume += order.volume;
  ordered_volume_map.insert_or_assign(instrument_key, ordered_volume);
}

int64_t BasketOrderState::get_success_ordered_volume(const longfist::types::Order &order) {
  int32_t instrument_key = hash_instrument(order);
  return success_ordered_volume_map.find(instrument_key) != success_ordered_volume_map.end()
             ? success_ordered_volume_map.at(instrument_key)
             : 0;
}

int64_t BasketOrderState::get_total_volume(const longfist::types::Order &order) {
  int32_t instrument_key = hash_instrument(order);
  auto &basket_order = state_data.data;

  // if it is a new order
  if (orders.find(order.order_id) != orders.end()) {
    return basket_order.volume;
  }

  int64_t total_volume = 0;
  for (auto &iter : ordered_volume_map) {
    total_volume += iter.second;
  }
  return total_volume;
}

void BasketOrderState::update(const longfist::types::Order &order) {
  auto &basket_order = state_data.data;

  update_ordered_volume(order);
  basket_order.volume = (basket_order.volume, get_total_volume(order));
  update_success_ordered_volume(order);

  orders.insert_or_assign(order.order_id, order);
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