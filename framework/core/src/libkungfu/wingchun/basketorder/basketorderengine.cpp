#include <kungfu/wingchun/basketorder/basketorderengine.h>

using namespace kungfu::rx;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::basketorder;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::basketorder {
BasketOrderEngine::BasketOrderEngine(apprentice &app) : app_(app) {}

void BasketOrderEngine::on_start(const rx::connectable_observable<event_ptr> &events) {
  restore(app_.get_state_bank());

  events | is(BasketOrder::tag) | $$(on_basket_order(event->trigger_time(), event->data<BasketOrder>()));
  events | is(Order::tag) | $$(update_basket_order(event->trigger_time(), event->data<Order>()));
  events | is(Basket::tag) | $$(update_basket(event->data<Basket>()));
  events | is(BasketInstrument::tag) | $$(update_basket_instrument(event->data<BasketInstrument>()));
}

void BasketOrderEngine::restore(const cache::bank &state_bank) {
  for (const auto &pair : state_bank[boost::hana::type_c<BasketOrder>]) {
    auto basketorder_state = pair.second;
    make_basket_order_state(basketorder_state.update_time, basketorder_state.data);
  }

  for (const auto &pair : state_bank[boost::hana::type_c<Order>]) {
    auto order_state = pair.second;
    try_update_basket_order(order_state.update_time, order_state.data);
  }

  for (auto &pair : state_bank[boost::hana::type_c<Basket>]) {
    SPDLOG_INFO("restore basket {}", pair.second.data.to_string());
    update_basket(pair.second.data);
  }

  for (auto &pair : state_bank[boost::hana::type_c<BasketInstrument>]) {
    update_basket_instrument(pair.second.data);
  }
}

void BasketOrderEngine::on_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order) {
  make_basket_order_state(trigger_time, basket_order);
}

void BasketOrderEngine::insert_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order) {
  auto basket_order_state = make_basket_order_state(trigger_time, basket_order);

  if (app_.get_home_uid() == basket_order.source_id) {
    app_.get_writer(basket_order.dest_id)->write(app_.now(), basket_order_state->get_state().data);
  }
}

void BasketOrderEngine::update_basket_order(int64_t trigger_time, const longfist::types::Order &order) {

  if (not try_update_basket_order(trigger_time, order)) {
    return;
  }

  auto basket_order_state = get_basket_order_state(order.parent_id);
  auto source_id = basket_order_state->get_state().data.source_id;
  auto dest_id = basket_order_state->get_state().data.dest_id;

  if (not app_.has_writer(dest_id)) {
    SPDLOG_WARN("no writer for basket order dest_id {} {}", dest_id, app_.get_location_uname(dest_id));
    return;
  }
  if (app_.get_home_uid() == source_id) {
    app_.get_writer(dest_id)->write(app_.now(), basket_order_state->get_state().data);
  }
}

bool BasketOrderEngine::try_update_basket_order(int64_t, const longfist::types::Order &order) {
  if (order.parent_id == (uint64_t)0) {
    SPDLOG_TRACE("not a basket order");
    return false;
  }

  if (not has_basket_order_state(order.parent_id)) {
    SPDLOG_ERROR("basket order is not exist {} {}", order.parent_id, order.to_string());
    return false;
  }

  auto basket_order_state = get_basket_order_state(order.parent_id);
  basket_order_state->update(order);
  return true;
}

bool BasketOrderEngine::has_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.find(basket_order_id) != basket_order_states_.end();
}

BasketOrderStateMap &BasketOrderEngine::get_all_basket_order_states() { return basket_order_states_; }

state<longfist::types::BasketOrder> &BasketOrderEngine::get_basket_order(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id)->get_state();
}

BasketOrderState_ptr BasketOrderEngine::get_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id);
}

BasketOrderState_ptr BasketOrderEngine::make_basket_order_state(int64_t trigger_time, const BasketOrder &basket_order) {
  // for third process accept basket order by other process writer for updating;
  if (has_basket_order_state(basket_order.order_id)) {
    return basket_order_states_.at(basket_order.order_id);
  }

  auto basket_order_state =
      std::make_shared<BasketOrderState>(basket_order.source_id, basket_order.dest_id, trigger_time, basket_order);
  basket_order_states_.insert_or_assign(basket_order.order_id, basket_order_state);
  return basket_order_states_.at(basket_order.order_id);
}

} // namespace kungfu::wingchun::basketorder
