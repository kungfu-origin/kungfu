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

void MarketData::on_react() {
  Broker::on_react();
  events_ | is(Instrument::tag) | $$(update_instrument(event->data<Instrument>()));
}

void MarketData::on_start() {
  Broker::on_start();
  events_ | is(SubscribeAll::tag) | $$(subscribe_all());
  events_ | is(InstrumentKey::tag) | $$(subscribe({event->data<InstrumentKey>()}));
}

bool MarketData::has_instrument(const std::string& instrument_id) const {
  return instruments_.find(instrument_id) != instruments_.end();
}

const Instrument &MarketData::get_instrument(const std::string& instrument_id) const {
  return instruments_.at(instrument_id);
}

void MarketData::update_instrument(Instrument instrument) {
  instruments_.emplace(instrument.instrument_id, instrument);
}
} // namespace kungfu::wingchun::broker
