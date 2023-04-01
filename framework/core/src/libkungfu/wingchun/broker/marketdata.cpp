// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/wingchun/broker/marketdata.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
MarketDataVendor::MarketDataVendor(locator_ptr locator, const std::string &group, const std::string &name,
                                   bool low_latency)
    : BrokerVendor(location::make_shared(mode::LIVE, category::MD, group, name, std::move(locator)), low_latency) {}

void MarketDataVendor::set_service(MarketData_ptr service) { service_ = std::move(service); }

void MarketDataVendor::on_react() {
  BrokerVendor::on_react();
  events_ | is(Instrument::tag) | $$(service_->update_instrument(event->data<Instrument>()));
}

void MarketDataVendor::on_start() {
  BrokerVendor::on_start();
  events_ | is(CustomSubscribe::tag) | $$(service_->subscribe_custom(event->data<CustomSubscribe>()));
  events_ | is(InstrumentKey::tag) | $$(service_->add_instrument_key(event->data<InstrumentKey>()));
  events_ | is_custom() | $$(service_->on_custom_event(event));
  service_->on_start();

  add_time_interval(time_unit::NANOSECONDS_PER_SECOND, [&](auto e) { service_->try_subscribe(); });
}

BrokerService_ptr MarketDataVendor::get_service() { return service_; }

void MarketDataVendor::on_trading_day(const event_ptr &event, int64_t daytime) {
  service_->on_trading_day(event, daytime);
}

[[maybe_unused]] bool MarketData::has_instrument(const std::string &instrument_id) const {
  return instruments_.find(instrument_id) != instruments_.end();
}

[[maybe_unused]] const Instrument &MarketData::get_instrument(const std::string &instrument_id) const {
  return instruments_.at(instrument_id);
}

void MarketData::update_instrument(Instrument instrument) {
  instruments_.emplace(instrument.instrument_id, instrument);
}

void MarketData::try_subscribe() {
  if (not instruments_to_subscribe_.empty()) {
    subscribe(instruments_to_subscribe_);
  }
  instruments_to_subscribe_.clear();
}

void MarketData::add_instrument_key(const InstrumentKey &key) { instruments_to_subscribe_.push_back(key); }

} // namespace kungfu::wingchun::broker
