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

struct BasketOrderState {
  kungfu::state<longfist::types::BasketOrder> state_data;
  OrderMap orders = {};

  BasketOrderState(uint32_t source, uint32_t dest, int64_t trigger_time,
                   const longfist::types::BasketOrder &basket_order);

  void update(const longfist::types::Order &order);

    kungfu::state<longfist::types::BasketOrder> &get_state();

  //   int64_t get_total_volume();

  //   int64_t get_volume_left();

  //   int64_t get_volume_alive();
};

} // namespace kungfu::wingchun::basketorder

#endif