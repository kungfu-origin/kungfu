//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/common.h>
#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id)
    : Broker(location::make_shared(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
      source_(source), account_id_(account_id) {}

void Trader::on_start() {
  Broker::on_start();

  events_ | is(OrderInput::tag) | $$(insert_order(event));
  events_ | is(OrderAction::tag) | $$(cancel_order(event));
  events_ | is(AssetRequest::tag) | $$(req_account());
  events_ | is(PositionRequest::tag) | $$(req_position());
  events_ | is(ResetBookRequest::tag) | $$(get_writer(location::PUBLIC)->mark(now(), ResetBookRequest::tag));

  clean_orders();
}

void Trader::clean_orders() {
  std::set<uint32_t> strategy_uids = {};
  auto master_cmd_writer = get_writer(get_master_commands_uid());
  for (auto &pair : state_bank_[boost::hana::type_c<Order>]) {
    auto &order_state = pair.second;
    auto &order = const_cast<Order &>(order_state.data);
    auto strategy_uid = order_state.dest;
    if (order.status == OrderStatus::Submitted or order.status == OrderStatus::Pending) {
      if (strategy_uid == location::PUBLIC) {
        write_to(now(), order);
        continue;
      }
      strategy_uids.emplace(strategy_uid);
      order.status = OrderStatus::Lost;
      events_ | is(Channel::tag) | filter([&, strategy_uid](const event_ptr &event) {
        const Channel &channel = event->data<Channel>();
        return channel.source_id == get_home_uid() and channel.dest_id == strategy_uid;
      }) | first() |
          $([this, order, strategy_uid](auto event) { write_to(now(), order, strategy_uid); });
    }
  }
  for (auto uid : strategy_uids) {
    if (not has_writer(uid)) {
      request_write_to(now(), uid);
    }
  }
}
} // namespace kungfu::wingchun::broker
