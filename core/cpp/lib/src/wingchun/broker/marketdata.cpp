//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/wingchun/broker/marketdata.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
MarketData::MarketData(bool low_latency, locator_ptr locator, const std::string &source)
    : Broker(location::make_shared(mode::LIVE, category::MD, source, source, std::move(locator)), low_latency) {}

void MarketData::on_start() {
  Broker::on_start();
  events_ | is(SubscribeAll::tag) | $$(subscribe_all());
  events_ | is(InstrumentKey::tag) | $$(subscribe({event->data<InstrumentKey>()}));
}
} // namespace kungfu::wingchun::broker
