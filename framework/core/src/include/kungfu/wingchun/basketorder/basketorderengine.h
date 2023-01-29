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

  void restore(const yijinjing::cache::bank &state_bank);

  void on_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order);

  void insert_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order);

  void update_basket_order(int64_t trigger_time, const longfist::types::Order &order);

  bool try_update_basket_order(int64_t trigger_time, const longfist::types::Order &order);

  bool has_basket_order_state(uint64_t basket_order_id);

  BasketOrderStateMap &get_all_basket_order_states();

  BasketOrderState_ptr get_basket_order_state(uint64_t basket_order_id);

  kungfu::state<longfist::types::BasketOrder> &get_basket_order(uint64_t basket_order_id);

  [[nodiscard]] const BasketMap &get_baskets() const { return baskets_; }

  [[nodiscard]] const BasketInstrumentMap &get_basket_instruments() const { return basket_instruments_; }

private:
  yijinjing::practice::apprentice &app_;
  BasketMap baskets_ = {};
  BasketInstrumentMap basket_instruments_{};
  BasketOrderStateMap basket_order_states_ = {};

  BasketOrderState_ptr make_basket_order_state(int64_t update_time, const longfist::types::BasketOrder &basket_order);

  void update_basket(const longfist::types::Basket &basket) { baskets_.insert_or_assign(basket.id, basket); }

  void update_basket_instrument(const longfist::types::BasketInstrument &basket_instrument) {
    auto basket_instrument_hashed = hash_basket_instrument(basket_instrument.basket_uid, basket_instrument.exchange_id,
                                                           basket_instrument.instrument_id);
    basket_instruments_.insert_or_assign(basket_instrument_hashed, basket_instrument);
  }
};

} // namespace kungfu::wingchun::basketorder

#endif // WINGCHUN_BASKETORDER_ENGINE_H