// SPDX-License-Identifier: Apache-2.0

#ifndef WINGCHUN_BAKSETORDER_H
#define WINGCHUN_BAKSETORDER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/common.h>

namespace kungfu::wingchun::basketorder {
FORWARD_DECLARE_STRUCT_PTR(BasketOrderState)
FORWARD_DECLARE_CLASS_PTR(BasketOrderEngine)

// key = order_id
typedef std::unordered_map<uint64_t, longfist::types::Order> OrderMap;
typedef std::unordered_map<int32_t, longfist::types::Order> HashedInstrumentToOrderMap;

inline int64_t get_total_traded_volume(const OrderMap &orders) {
  int64_t volume = 0;
  for (auto &iter : orders) {
    auto &order = iter.second;
    volume += order.volume - order.volume_left;
  }
  return volume;
}

inline bool is_all_order_end(const OrderMap &orders) {
  for (auto &iter : orders) {
    if (not is_final_status(iter.second.status)) {
      return false;
    }
  }
  return true;
}

struct BasketOrderState {
  kungfu::state<longfist::types::BasketOrder> state_data;
  OrderMap orders = {};
  HashedInstrumentToOrderMap lastest_order_map = {};

  BasketOrderState(uint32_t source, uint32_t dest, int64_t trigger_time,
                   const longfist::types::BasketOrder &basket_order);

  void update(const longfist::types::Order &order);
  void update_lastst_order(const longfist::types::Order &order);

  int64_t get_total_volume();

  kungfu::state<longfist::types::BasketOrder> &get_state();
  OrderMap &get_orders();
}; // namespace kungfu::wingchun::basketorder
} // namespace kungfu::wingchun::basketorder
#endif