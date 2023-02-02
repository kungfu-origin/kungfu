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

inline int64_t get_total_volume(const OrderMap &orders) {
  int64_t volume = 0;
  for (auto &iter : orders) {
    volume += iter.second.volume;
  }
  return volume;
}

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

inline int64_t get_merged_total_volume(const OrderMap &orders) {
  std::unordered_map<uint32_t, int64_t> long_hash_to_total_merge_volume = {};
  std::unordered_map<uint32_t, int64_t> short_hash_to_total_merge_volume = {};
  for (auto &iter : orders) {
    auto &order = iter.second;
    auto instrument_key = hash_instrument(order.exchange_id, order.instrument_id);
    auto direction = get_direction(order.instrument_type, order.side, order.offset);
    auto is_long = direction == longfist::enums::Direction::Long;
    auto &hash_to_total_merge_volume = is_long ? long_hash_to_total_merge_volume : short_hash_to_total_merge_volume;
    int64_t final_merge_volume = (std::max)(hash_to_total_merge_volume[instrument_key], order.volume);
    hash_to_total_merge_volume.insert_or_assign(instrument_key, final_merge_volume);
  }

  int64_t total_volume = 0;
  for (auto &iter : long_hash_to_total_merge_volume) {
    total_volume += iter.second;
  }
  for (auto &iter : short_hash_to_total_merge_volume) {
    total_volume += iter.second;
  }
  return total_volume;
}

struct BasketOrderState {
  kungfu::state<longfist::types::BasketOrder> state_data;
  OrderMap orders = {};

  BasketOrderState(uint32_t source, uint32_t dest, int64_t trigger_time,
                   const longfist::types::BasketOrder &basket_order);

  void update(const longfist::types::Order &order);

  kungfu::state<longfist::types::BasketOrder> &get_state();

  OrderMap &get_orders();
}; // namespace kungfu::wingchun::basketorder
} // namespace kungfu::wingchun::basketorder
#endif