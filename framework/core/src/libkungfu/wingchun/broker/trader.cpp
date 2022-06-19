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
TraderVendor::TraderVendor(locator_ptr locator, const std::string &group, const std::string &name, bool low_latency)
    : BrokerVendor(location::make_shared(mode::LIVE, category::TD, group, name, std::move(locator)), low_latency) {}

void TraderVendor::set_service(Trader_ptr service) { service_ = std::move(service); }

void TraderVendor::on_start() {
  BrokerVendor::on_start();

  events_ | is(OrderInput::tag) | $$(service_->insert_order(event));
  events_ | is(OrderAction::tag) | $$(service_->cancel_order(event));
  events_ | is(AssetRequest::tag) | $$(service_->req_account());
  events_ | is(PositionRequest::tag) | $$(service_->req_position());
  events_ | is(RequestHistoryOrder::tag) | $$(service_->req_history_order(event));
  events_ | is(RequestHistoryTrade::tag) | $$(service_->req_history_trade(event));
  events_ | is(AssetSync::tag) | $$(service_->req_account());
  events_ | is(PositionSync::tag) | $$(service_->req_position());
  events_ | is(ResetBookRequest::tag) | $$(get_writer(location::PUBLIC)->mark(now(), ResetBookRequest::tag));

  clean_orders();

  service_->on_start();
}

BrokerService_ptr TraderVendor::get_service() { return service_; }

void TraderVendor::clean_orders() {
  std::set<uint32_t> strategy_uids = {};
  auto master_cmd_writer = get_writer(get_master_commands_uid());
  for (auto &pair : state_bank_[boost::hana::type_c<Order>]) {
    auto &order_state = pair.second;
    auto &order = const_cast<Order &>(order_state.data);
    auto strategy_uid = order_state.dest;
    if (order.status == OrderStatus::Submitted or order.status == OrderStatus::Pending or
        order.status == OrderStatus::PartialFilledActive) {

      order.status = OrderStatus::Lost;
      order.update_time = time::now_in_nano();

      if (strategy_uid == location::PUBLIC) {
        write_to(now(), order);
        continue;
      }

      strategy_uids.emplace(strategy_uid);

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

void TraderVendor::on_trading_day(const event_ptr &event, int64_t daytime) { service_->on_trading_day(event, daytime); }

const std::string &Trader::get_account_id() const { return get_home()->name; }

yijinjing::journal::writer_ptr Trader::get_asset_writer() const {
  return get_writer(sync_asset_ ? location::SYNC : location::PUBLIC);
}

yijinjing::journal::writer_ptr Trader::get_position_writer() const {
  return get_writer(sync_position_ ? location::SYNC : location::PUBLIC);
}

void Trader::enable_asset_sync() { sync_asset_ = true; }

void Trader::enable_positions_sync() { sync_position_ = true; }

} // namespace kungfu::wingchun::broker
