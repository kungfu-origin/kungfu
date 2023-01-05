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
  events | is(Order::tag) | $$(update_basket_order(event->trigger_time(), event->data<Order>()));
  events | is(BasketOrder::tag) |
      $$(on_basket_order(event->trigger_time(), event->source(), event->dest(), event->data<BasketOrder>()));
}

void BasketOrderEngine::on_basket_order(int64_t trigger_time, uint32_t source, uint32_t dest,
                                        const longfist::types::BasketOrder &basket_order) {
  make_basket_order_state(source, dest, trigger_time, basket_order);
}

void BasketOrderEngine::insert_basket_order(int64_t trigger_time, uint32_t source, uint32_t dest,
                                            const longfist::types::BasketOrder &basket_order) {
  auto basket_order_state = make_basket_order_state(source, dest, trigger_time, basket_order);
  app_.get_writer(dest)->write(app_.now(), basket_order_state->get_state().data);
}

void BasketOrderEngine::update_basket_order(int64_t trigger_time, const longfist::types::Order &order) {

  if (order.parent_id == (uint64_t)0) {
    SPDLOG_DEBUG("not a basket order");
    return;
  }

  if (not has_basket_order_state(order.parent_id)) {
    throw wingchun_error(fmt::format("basket order is not exist {} {}", order.parent_id));
    return;
  }

  auto basket_order_state = get_basket_order_state(order.parent_id);
  basket_order_state->update(order);
  auto &basket_order = basket_order_state->get_state().data;
  auto is_all_order_end = basket_order_state->is_all_order_end();
  auto is_all_order_filled = basket_order_state->is_all_order_filled();
  // after supplementing order, the total volume may be changed, bigger than the original volume
  auto total_volume = std::max(basket_order.volume, basket_order_state->get_total_volume());
  auto total_volume_left = basket_order_state->get_total_volume_left();

  basket_order.update_time = trigger_time;
  basket_order.volume = total_volume;
  basket_order.volume_left = total_volume_left;

  if (is_all_order_end and is_all_order_filled) {
    basket_order.status = BasketOrderStatus::Filled;
  } else if (is_all_order_end) {
    basket_order.status = BasketOrderStatus::PartialFilledNotActive;
  } else if (basket_order.volume_left < total_volume) {
    basket_order.status = BasketOrderStatus::PartialFilledActive;
  } else {
    basket_order.status = BasketOrderStatus::Pending;
  }

  auto dest = basket_order_state->get_state().dest;
  app_.get_writer(dest)->write(app_.now(), basket_order_state->get_state().data);
}

bool BasketOrderEngine::has_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.find(basket_order_id) != basket_order_states_.end();
}

state<longfist::types::BasketOrder> &BasketOrderEngine::get_basket_order(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id)->get_state();
}

BasketOrderState_ptr BasketOrderEngine::get_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id);
}

BasketOrderState_ptr BasketOrderEngine::make_basket_order_state(uint32_t source, uint32_t dest, int64_t trigger_time,
                                                                const BasketOrder &basket_order) {
  auto basket_order_state = std::make_shared<BasketOrderState>(source, dest, trigger_time, basket_order);
  basket_order_states_.insert_or_assign(basket_order.order_id, basket_order_state);
  return basket_order_state;
}

} // namespace kungfu::wingchun::basketorder
