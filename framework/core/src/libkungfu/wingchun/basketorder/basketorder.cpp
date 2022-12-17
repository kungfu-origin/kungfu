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

state<longfist::types::BasketOrder> &BasketOrderState::get_state() { return state_data; }

bool BasketOrderState::is_all_order_end() {
  for (auto &iter : orders) {
    if (not is_final_status(iter.second.status)) {
      return false;
    }
  }
  return true;
}

bool BasketOrderState::is_all_order_filled() {
  for (auto &iter : orders) {
    if (iter.second.volume_left > 0) {
      return false;
    }
  }
  return true;
}

int64_t BasketOrderState::get_total_volume() {
  int64_t volume = 0;
  for (auto &iter : orders) {
    volume += iter.second.volume;
  }
  return volume;
}

int64_t BasketOrderState::get_total_volume_left() {
  int64_t volume_left = 0;
  for (auto &iter : orders) {
    volume_left += iter.second.volume_left;
  }
  return volume_left;
}
} // namespace kungfu::wingchun::basketorder