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

  events_ | is(OrderInput::tag) | $([&](const event_ptr &event) { insert_order(event); });

  events_ | is(OrderAction::tag) | $([&](const event_ptr &event) { cancel_order(event); });

  events_ | is(AssetRequest::tag) | $([&](const event_ptr &event) { req_account(); });

  events_ | is(PositionRequest::tag) | $([&](const event_ptr &event) { req_position(); });
}
} // namespace kungfu::wingchun::broker
