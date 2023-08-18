// SPDX-License-Identifier: Apache-2.0

#include <kungfu/wingchun/basketorder/basketorder.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::basketorder {
BasketOrderState::BasketOrderState(uint32_t source, uint32_t dest, int64_t trigger_time,
                                   const longfist::types::BasketOrder &basket_order)
    : state_data(state<BasketOrder>(source, dest, trigger_time, basket_order)){};

void BasketOrderState::update_lastst_order(const longfist::types::Order &order) {
  orders.insert_or_assign(order.order_id, order);

  auto hashed_key = hash_instrument(order);
  if (last_order_map.find(hashed_key) == last_order_map.end()) {
    last_order_map.emplace(hashed_key, order);
    return;
  }

  auto &old_last_order = last_order_map.at(hashed_key);
  if (old_last_order.insert_time > order.insert_time) {
    return;
  }

  last_order_map.insert_or_assign(hashed_key, order);
}

void BasketOrderState::update(const longfist::types::Order &order) {
  auto &basket_order = state_data.data;
  update_lastst_order(order);
  basket_order.volume =
      basket_order.calculation_mode == BasketOrderCalculationMode::Static ? basket_order.volume : get_total_volume();
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

int64_t BasketOrderState::get_total_volume() {
  int64_t total_volume = 0;
  for (auto &iter : orders) {
    auto &order = iter.second;
    auto hashed_key = hash_instrument(order);
    auto is_last_order = last_order_map.find(hashed_key) != last_order_map.end() &&
                         last_order_map.at(hashed_key).order_id == order.order_id;
    total_volume += is_last_order                   ? order.volume
                    : is_final_status(order.status) ? order.volume - order.volume_left
                                                    : order.volume;
  }
  return total_volume;
}

state<longfist::types::BasketOrder> &BasketOrderState::get_state() { return state_data; }

OrderMap &BasketOrderState::get_orders() { return orders; }

} // namespace kungfu::wingchun::basketorder