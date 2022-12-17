#ifndef WINGCHUN_BASKETORDER_ENGINE_H
#define WINGCHUN_BASKETORDER_ENGINE_H

#include <kungfu/common.h>
#include <kungfu/wingchun/basketorder/basketorder.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::basketorder {

FORWARD_DECLARE_CLASS_PTR(BasketOrderEngine)

// key = basket_uid
typedef std::unordered_map<uint32_t, longfist::types::Basket> BasketMap;

// key = hash_basket_instrument(basket_uid, exchange_id, instrument_id)
typedef std::unordered_map<uint32_t, longfist::types::BasketInstrument> BasketInstrumentMap;

// key = order_id
typedef std::unordered_map<uint64_t, BasketOrderState_ptr> BasketOrderStateMap;

class BasketOrderEngine {
public:
  explicit BasketOrderEngine(yijinjing::practice::apprentice &app);

  virtual ~BasketOrderEngine() = default;

  void on_start(const rx::connectable_observable<event_ptr> &events);

  void on_basket_order(int64_t trigger_time, uint32_t source, uint32_t dest,
                       const longfist::types::BasketOrder &basket_order);

  void insert_basket_order(int64_t trigger_time, uint32_t source, uint32_t dest,
                       const longfist::types::BasketOrder &basket_order);

  void update_basket_order(int64_t trigger_time, const longfist::types::Order &order);

  bool has_basket_order_state(uint64_t basket_order_id);

  BasketOrderState_ptr get_basket_order_state(uint64_t basket_order_id);

  kungfu::state<longfist::types::BasketOrder> &get_basket_order(uint64_t basket_order_id);

private:
  yijinjing::practice::apprentice &app_;
  BasketMap baskets_ = {};
  BasketInstrumentMap basket_instruments_{};
  BasketOrderStateMap basket_order_states_ = {};

  BasketOrderState_ptr make_basket_order_state(uint32_t source, uint32_t dest, int64_t update_time,
                                               const longfist::types::BasketOrder &basket_order);
};

} // namespace kungfu::wingchun::basketorder

#endif // WINGCHUN_BASKETORDER_ENGINE_H