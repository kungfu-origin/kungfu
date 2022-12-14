#ifndef WINGCHUN_BASKETORDER_H
#define WINGCHUN_BASKETORDER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/common.h>

namespace kungfu::wingchun::basketorder {

FORWORD_DECLARE_STRUCT_PTR(BasketOrder)
FORWORD_DECLARE_CLASS_PTR(BasketOrderKeeper)

// key = order_id
typedef std::unordered_map<uint64_t, longfist::types::Order> OrderMap;

// key = trade_id
typedef std::unordered_map<uint64_t, longfist::types::Trade> TradeMap;

struct BasketOrder {
  uint64_t basket_order_id;
  int64_t volume_total;
  double asset_total;
  OrderMap orders = {};
  TradeMap trades = {};
}

}; // namespace kungfu::wingchun::basketorder

#endif // WINGCHUN_BASKETORDER_H