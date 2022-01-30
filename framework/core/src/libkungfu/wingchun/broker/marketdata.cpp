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
MarketDataVendor::MarketDataVendor(locator_ptr locator, const std::string &group, const std::string &name,
                                   bool low_latency)
    : BrokerVendor(location::make_shared(mode::LIVE, category::MD, group, name, std::move(locator)), low_latency) {}

void MarketDataVendor::setup(MarketData_ptr service) { service_ = std::move(service); }

void MarketDataVendor::on_react() {
  BrokerVendor::on_react();
  events_ | is(Instrument::tag) | $$(service_->update_instrument(event->data<Instrument>()));
}

void MarketDataVendor::on_start() {
  BrokerVendor::on_start();
  events_ | is(SubscribeAll::tag) | $$(service_->subscribe_all());
  events_ | is(InstrumentKey::tag) | $$(service_->subscribe({event->data<InstrumentKey>()}));
  service_->on_start();
}

BrokerService_ptr MarketDataVendor::get_service() { return service_; }

bool MarketData::has_instrument(const std::string &instrument_id) const {
  return instruments_.find(instrument_id) != instruments_.end();
}

const Instrument &MarketData::get_instrument(const std::string &instrument_id) const {
  return instruments_.at(instrument_id);
}

void MarketData::update_instrument(Instrument instrument) {
  instruments_.emplace(instrument.instrument_id, instrument);
}
} // namespace kungfu::wingchun::broker
