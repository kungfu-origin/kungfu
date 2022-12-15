#ifndef WINGCHUN_BASKETORDER_ENGINE_H
#define WINGCHUN_BASKETORDER_ENGINE_H

#include <kungfu/common.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::basketorder {

FORWARD_DECLARE_CLASS_PTR(BasketOrderEngine)

// key = basket_uid
typedef std::unordered_map<uint32_t, longfist::types::Basket> BasketMap;

// key = hash_basket_instrument(basket_uid, exchange_id, instrument_id)
typedef std::unordered_map<uint32_t, longfist::types::BasketInstrument> BasketInstrumentMap;

// key = order_id
typedef std::unordered_map<uint64_t, state<longfist::types::BasketOrder>> BasketOrderMap;

class BasketOrderEngine {
public:
  explicit BasketOrderEngine(yijinjing::practice::apprentice &app);

  virtual ~BasketOrderEngine() = default;

  void on_start(const rx::connectable_observable<event_ptr> &events);

  void on_basket_order(int64_t update_time, uint32_t source, uint32_t dest,
                       const longfist::types::BasketOrder &basket_order);

  void update_basket_order(int64_t trigger_time, const longfist::types::Order &order);

  bool has_basket_order(uint64_t order_id);

  state<longfist::types::BasketOrder> &get_basket_order(uint64_t order_id);

private:
  yijinjing::practice::apprentice &app_;
  BasketMap baskets_ = {};
  BasketInstrumentMap basket_instruments_{};
  BasketOrderMap basket_orders_ = {};
};

} // namespace kungfu::wingchun::basketorder

#endif // WINGCHUN_BASKETORDER_ENGINE_H