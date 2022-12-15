#include <kungfu/wingchun/basketorder/engine.h>

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
}

void BasketOrderEngine::on_basket_order(int64_t update_time, uint32_t source, uint32_t dest,
                                        const longfist::types::BasketOrder &basket_order) {

  if (basket_orders_.find(basket_order.order_id) != basket_orders_.end()) {
    throw wingchun_error(fmt::format("duplicated basket_order from {} to {} {}", app_.get_location_uname(source),
                                     app_.get_location_uname(dest), basket_order.order_id));
    return;
  }

  basket_orders_.emplace(basket_order.order_id, state<BasketOrder>(source, dest, update_time, basket_order));
  app_.get_writer(dest)->write(app_.now(), basket_order);
}

void BasketOrderEngine::update_basket_order(int64_t trigger_time, const longfist::types::Order &order) {
  if (basket_orders_.find(order.parent_id) == basket_orders_.end()) {
    return;
  }

  auto &basket_order_data = basket_orders_.at(order.parent_id);
  auto &basket_order = basket_orders_.at(order.parent_id).data;
  basket_order.update_time = trigger_time;
  basket_order.volume_left -= order.volume - order.volume_left;
  if (is_final_status(order.status)) {
    basket_order.volume_alive -= order.volume;
  }

  if (basket_order.volume_left < basket_order.volume) {
    basket_order.status = BasketOrderStatus::PartialFilledActive;

    if (basket_order.volume_alive == 0 && basket_order.volume_left == 0) {
      basket_order.status = BasketOrderStatus::Filled;
    } else if (basket_order.volume_left == 0) {
      basket_order.status = BasketOrderStatus::PartialFilledNotActive;
    }
  } else {
    basket_order.status = BasketOrderStatus::Pending;
  }

  basket_orders_.insert_or_assign(
      basket_order.order_id,
      state<BasketOrder>(basket_order_data.source, basket_order_data.dest, trigger_time, basket_order));
  app_.get_writer(basket_order_data.dest)->write(app_.now(), basket_order);
}

bool BasketOrderEngine::has_basket_order(uint64_t order_id) {
  return basket_orders_.find(order_id) != basket_orders_.end();
}

state<longfist::types::BasketOrder> &BasketOrderEngine::get_basket_order(uint64_t order_id) {
  return basket_orders_.at(order_id);
}

} // namespace kungfu::wingchun::basketorder
