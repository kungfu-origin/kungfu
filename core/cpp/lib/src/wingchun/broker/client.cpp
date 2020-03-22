//
// Created by Keren Dong on 2020/3/12.
//

#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/utils.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
inline uint32_t hash_instrument(const longfist::types::Instrument &instrument) {
  return get_symbol_id(instrument.exchange_id, instrument.instrument_id);
}

Client::Client(apprentice &app) : app_(app) {}

const std::unordered_map<uint32_t, longfist::types::Instrument> &Client::get_instruments() const {
  return instruments_;
}

bool Client::is_ready(uint32_t broker_location_uid) const {
  if (app_.has_location(broker_location_uid) and app_.has_writer(broker_location_uid)) {
    auto broker_location = app_.get_location(broker_location_uid);
    bool md_test = broker_location->category == category::MD and
                   ready_md_locations_.find(broker_location->uid) != ready_md_locations_.end();
    bool td_test = broker_location->category == category::TD and
                   ready_td_locations_.find(broker_location->uid) != ready_td_locations_.end();
    return md_test or td_test;
  }
  return false;
}

bool Client::is_subscribed(uint32_t md_location_uid, const std::string &exchange_id,
                           const std::string &instrument_id) const {
  return instruments_.find(get_symbol_id(exchange_id.c_str(), instrument_id.c_str())) != instruments_.end();
}

void Client::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                       const std::string &instrument_id) {
  Instrument instrument = {};
  strcpy(instrument.instrument_id, instrument_id.c_str());
  strcpy(instrument.exchange_id, exchange_id.c_str());
  uint32_t key = hash_instrument(instrument);
  instruments_.emplace(key, instrument);
  instrument_md_locations_.emplace(key, md_location);
}

void Client::on_start(const rx::connectable_observable<event_ptr> &events) {
  events | is(Register::tag) | $([&](const event_ptr &event) { connect(event->data<Register>()); });

  events | is(BrokerStateUpdate::tag) | $([&](const event_ptr &event) {
    auto state = event->data<BrokerStateUpdate>().state;
    auto broker_location = app_.get_location(event->source());

    bool state_ok = state == BrokerState::LoggedIn or state == BrokerState::Ready;
    bool state_reset = state == BrokerState::Connected or state == BrokerState::DisConnected;

    if (broker_location->category == category::MD) {
      bool ready = ready_md_locations_.find(broker_location->uid) != ready_md_locations_.end();
      if (state_ok and not ready and app_.has_writer(broker_location->uid)) {
        ready_md_locations_.emplace(broker_location->uid, broker_location);
        subscribe_instruments(event->gen_time(), broker_location);
      }
      if (state_reset) {
        ready_md_locations_.erase(broker_location->uid);
      }
    }
    if (broker_location->category == category::TD) {
      if (state_ok) {
        ready_td_locations_.emplace(broker_location->uid, broker_location);
      }
      if (state_reset) {
        ready_td_locations_.erase(broker_location->uid);
      }
    }
    broker_states_.emplace(broker_location->uid, state);
  });

  events | is(Deregister::tag) | $([&](const event_ptr &event) {
    auto location_uid = event->data<Deregister>().location_uid;
    broker_states_.emplace(location_uid, BrokerState::DisConnected);
    ready_md_locations_.erase(location_uid);
    ready_td_locations_.erase(location_uid);
  });
}

void Client::subscribe_instruments(int64_t trigger_time, const yijinjing::data::location_ptr &md_location) {
  auto writer = app_.get_writer(md_location->uid);
  for (auto &pair : instruments_) {
    auto instrument = pair.second;
    if (md_location->uid == instrument_md_locations_.at(hash_instrument(instrument))->uid) {
      write_subscribe_msg(writer, trigger_time, instrument.exchange_id, instrument.instrument_id);
    }
  }
}

void Client::connect(const longfist::types::Register &register_data) {
  auto app_location = app_.get_location(register_data.location_uid);
  if (app_location->category == category::MD and should_connect_md(app_location)) {
    app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
    app_.request_write_to(app_.now(), app_location->uid);
  }
  if (app_location->category == category::TD and should_connect_td(app_location)) {
    app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
    app_.request_read_from(app_.now(), app_location->uid, register_data.checkin_time);
    app_.request_write_to(app_.now(), app_location->uid);
  }
}

AutoClient::AutoClient(apprentice &app) : Client(app) {}

bool AutoClient::should_connect_md(const yijinjing::data::location_ptr &md_location) const { return true; }

bool AutoClient::should_connect_td(const yijinjing::data::location_ptr &td_location) const { return true; }

EnrollmentClient::EnrollmentClient(apprentice &app) : Client(app) {}

bool EnrollmentClient::is_subscribed(uint32_t md_location_uid, const std::string &exchange_id,
                                     const std::string &instrument_id) const {
  return is_all_subscribed(md_location_uid) or Client::is_subscribed(md_location_uid, exchange_id, instrument_id);
}

void EnrollmentClient::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                                 const std::string &instrument_id) {
  if (not is_all_subscribed(md_location->uid)) {
    enrolled_md_locations_.emplace(md_location->uid, false);
  }
  Client::subscribe(md_location, exchange_id, instrument_id);
}

bool EnrollmentClient::is_all_subscribed(uint32_t md_location_uid) const {
  return should_connect_md(app_.get_location(md_location_uid)) and enrolled_md_locations_.at(md_location_uid);
}

void EnrollmentClient::subscribe_all(const location_ptr &md_location) {
  enrolled_md_locations_.emplace(md_location->uid, true);
}

void EnrollmentClient::enroll_account(const yijinjing::data::location_ptr &td_location) {
  enrolled_td_locations_.emplace(td_location->uid, true);
}

bool EnrollmentClient::should_connect_md(const yijinjing::data::location_ptr &md_location) const {
  return enrolled_md_locations_.find(md_location->uid) != enrolled_md_locations_.end();
}

bool EnrollmentClient::should_connect_td(const yijinjing::data::location_ptr &td_location) const {
  return enrolled_td_locations_.find(td_location->uid) != enrolled_td_locations_.end();
}

void EnrollmentClient::subscribe_instruments(int64_t trigger_time, const yijinjing::data::location_ptr &md_location) {
  if (is_all_subscribed(md_location->uid)) {
    app_.get_writer(md_location->uid)->mark(trigger_time, SubscribeAll::tag);
  } else {
    Client::subscribe_instruments(trigger_time, md_location);
  }
}
} // namespace kungfu::wingchun::broker
