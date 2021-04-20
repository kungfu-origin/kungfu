//
// Created by Keren Dong on 2020/3/12.
//

#include <kungfu/wingchun/broker/client.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
int64_t ResumePolicy::get_connect_time(const apprentice &app, const Register &broker) const {
  if (app.get_last_active_time() == INT64_MIN) {
    return broker.checkin_time;
  }
  if (broker.checkin_time >= app.get_checkin_time() and broker.last_active_time >= app.get_checkin_time()) {
    return broker.checkin_time;
  }
  if (broker.checkin_time >= app.get_checkin_time() and broker.last_active_time <= app.get_last_active_time()) {
    return broker.checkin_time;
  }
  if (broker.checkin_time <= app.get_last_active_time()) {
    return app.get_last_active_time();
  }
  return get_resume_time(app, broker);
}

int64_t StatelessResumePolicy::get_resume_time(const apprentice &app, const Register &broker) const {
  return broker.checkin_time;
}

int64_t ContinuousResumePolicy::get_resume_time(const apprentice &app, const Register &broker) const {
  return app.get_last_active_time();
}

int64_t IntradayResumePolicy::get_resume_time(const apprentice &app, const Register &broker) const {
  return std::max(app.get_last_active_time(), time::calendar_day_start(app.now()));
}

Client::Client(apprentice &app) : app_(app) {}

const Client::InstrumentKeyMap &Client::get_instrument_keys() const { return instrument_keys_; }

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

bool Client::is_subscribed(const std::string &exchange_id, const std::string &instrument_id) const {
  return instrument_keys_.find(hash_instrument(exchange_id.c_str(), instrument_id.c_str())) != instrument_keys_.end();
}

void Client::subscribe(const InstrumentKey &instrument_key) {
  instrument_keys_.emplace(instrument_key.key, instrument_key);
}

void Client::subscribe(const std::string &exchange_id, const std::string &instrument_id) {
  uint32_t key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  if (instrument_keys_.find(key) != instrument_keys_.end()) {
    return;
  }
  InstrumentKey instrument_key = {};
  instrument_key.key = key;
  strcpy(instrument_key.instrument_id, instrument_id.c_str());
  strcpy(instrument_key.exchange_id, exchange_id.c_str());
  instrument_key.instrument_type = get_instrument_type(exchange_id, instrument_id);
  subscribe(instrument_key);
}

void Client::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                       const std::string &instrument_id) {
  subscribe(exchange_id, instrument_id);
  exchange_md_locations_.emplace(exchange_id, md_location);
  instrument_md_locations_.emplace(hash_instrument(exchange_id.c_str(), instrument_id.c_str()), md_location);
}

void Client::renew(int64_t trigger_time, const location_ptr &md_location) {
  auto writer = app_.get_writer(md_location->uid);
  for (const auto &pair : instrument_keys_) {
    auto &instrument_key = pair.second;
    location_ptr source_location = {};
    if (exchange_md_locations_.find(instrument_key.exchange_id) != exchange_md_locations_.end()) {
      source_location = exchange_md_locations_.at(instrument_key.exchange_id);
    }
    if (instrument_md_locations_.find(instrument_key.key) != instrument_md_locations_.end()) {
      source_location = instrument_md_locations_.at(instrument_key.key);
    }
    if (source_location and md_location->uid == source_location->uid) {
      writer->write(trigger_time, instrument_key);
    }
  }
}

bool Client::try_renew(int64_t trigger_time, const location_ptr &md_location) {
  if (ready_md_locations_.find(md_location->uid) == ready_md_locations_.end()) {
    return false;
  }
  renew(trigger_time, md_location);
  return true;
}

void Client::sync(int64_t trigger_time, const yijinjing::data::location_ptr &td_location) {
  auto writer = app_.get_writer(td_location->uid);
  writer->mark(trigger_time, AssetRequest::tag);
  writer->mark(trigger_time, PositionRequest::tag);
}

bool Client::try_sync(int64_t trigger_time, const location_ptr &td_location) {
  if (ready_td_locations_.find(td_location->uid) == ready_td_locations_.end()) {
    return false;
  }
  sync(trigger_time, td_location);
  return true;
}

void Client::on_start(const rx::connectable_observable<event_ptr> &events) {
  events | is(Register::tag) | $$(connect(event, event->data<Register>()));
  events | is(BrokerStateUpdate::tag) | $$(update_broker_state(event, event->data<BrokerStateUpdate>()));
  events | is(Deregister::tag) | $$(update_broker_state(event, event->data<Deregister>()));
}

void Client::connect(const event_ptr &event, const Register &register_data) {
  auto app_uid = register_data.location_uid;
  auto app_location = app_.get_location(app_uid);
  auto resume_time_point = get_resume_policy().get_connect_time(app_, register_data);
  if (app_location->category == category::MD and should_connect_md(app_location)) {
    app_.request_write_to(app_.now(), app_uid);
    app_.request_read_from_public(app_.now(), app_uid, resume_time_point);
    SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
  }
  if (app_location->category == category::TD and should_connect_td(app_location)) {
    app_.request_write_to(app_.now(), app_uid);
    app_.request_read_from(app_.now(), app_uid, resume_time_point);
    app_.request_read_from_public(app_.now(), app_uid, resume_time_point);
    SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
  }
  if (app_location->category == category::STRATEGY and should_connect_strategy(app_location)) {
    app_.request_write_to(app_.now(), app_location->uid);
    app_.request_read_from(app_.now(), app_location->uid, resume_time_point);
    app_.request_read_from_public(app_.now(), app_location->uid, resume_time_point);
    SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
  }
}

void Client::update_broker_state(const event_ptr &event, const BrokerStateUpdate &state) {
  auto state_value = state.state;
  auto broker_location = app_.get_location(event->source());

  bool state_ready = state_value == BrokerState::Ready;
  bool state_reset = state_value == BrokerState::Connected or state_value == BrokerState::DisConnected;

  auto switch_broker_state = [&](category broker_category, location_map &ready_locations, auto on_broker_ready) {
    bool ready_recorded = ready_locations.find(broker_location->uid) != ready_locations.end();
    if (state_ready and app_.has_writer(broker_location->uid) and not ready_recorded) {
      ready_locations.emplace(broker_location->uid, broker_location);
      SPDLOG_INFO("{} ready, state {}", broker_location->uname, (int)state_value);
      on_broker_ready();
    }
    if (state_reset and ready_recorded) {
      ready_locations.erase(broker_location->uid);
      SPDLOG_INFO("{} reset, state {}", broker_location->uname, (int)state_value);
    }
  };

  if (broker_location->category == category::MD) {
    switch_broker_state(category::MD, ready_md_locations_, [&]() { renew(event->gen_time(), broker_location); });
  }
  if (broker_location->category == category::TD) {
    switch_broker_state(category::TD, ready_td_locations_, [&]() { sync(event->gen_time(), broker_location); });
  }

  broker_states_.emplace(broker_location->uid, state_value);
}

void Client::update_broker_state(const event_ptr &event, const longfist::types::Deregister &deregister_data) {
  auto location_uid = deregister_data.location_uid;
  auto broker_location = app_.get_location(location_uid);
  broker_states_.emplace(location_uid, BrokerState::DisConnected);
  ready_md_locations_.erase(location_uid);
  ready_td_locations_.erase(location_uid);
}

AutoClient::AutoClient(apprentice &app) : Client(app) {}

const ResumePolicy &AutoClient::get_resume_policy() const { return resume_policy_; }

bool AutoClient::is_fully_subscribed(uint32_t md_location_uid) const { return false; }

bool AutoClient::should_connect_md(const location_ptr &md_location) const { return true; }

bool AutoClient::should_connect_td(const location_ptr &td_location) const { return true; }

bool AutoClient::should_connect_strategy(const location_ptr &td_location) const { return true; }

SilentAutoClient::SilentAutoClient(practice::apprentice &app) : AutoClient(app) {}

bool SilentAutoClient::is_subscribed(const std::string &exchange_id, const std::string &instrument_id) const {
  return true;
}

void SilentAutoClient::renew(int64_t trigger_time, const location_ptr &md_location) {}

void SilentAutoClient::sync(int64_t trigger_time, const location_ptr &td_location) {}

PassiveClient::PassiveClient(apprentice &app) : Client(app) {}

const ResumePolicy &PassiveClient::get_resume_policy() const { return resume_policy_; }

bool PassiveClient::is_fully_subscribed(uint32_t md_location_uid) const {
  return should_connect_md(app_.get_location(md_location_uid)) and enrolled_md_locations_.at(md_location_uid);
}

void PassiveClient::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                             const std::string &instrument_id) {
  if (not is_fully_subscribed(md_location->uid)) {
    enrolled_md_locations_.emplace(md_location->uid, false);
  }
  Client::subscribe(md_location, exchange_id, instrument_id);
}

void PassiveClient::subscribe_all(const location_ptr &md_location) {
  enrolled_md_locations_.emplace(md_location->uid, true);
}

void PassiveClient::renew(int64_t trigger_time, const location_ptr &md_location) {
  if (is_fully_subscribed(md_location->uid)) {
    app_.get_writer(md_location->uid)->mark(trigger_time, SubscribeAll::tag);
  } else {
    Client::renew(trigger_time, md_location);
  }
}

void PassiveClient::sync(int64_t trigger_time, const location_ptr &td_location) {}

void PassiveClient::enroll_account(const location_ptr &td_location) {
  enrolled_td_locations_.emplace(td_location->uid, true);
}

bool PassiveClient::should_connect_md(const location_ptr &md_location) const {
  return enrolled_md_locations_.find(md_location->uid) != enrolled_md_locations_.end();
}

bool PassiveClient::should_connect_td(const location_ptr &td_location) const {
  return enrolled_td_locations_.find(td_location->uid) != enrolled_td_locations_.end();
}

bool PassiveClient::should_connect_strategy(const location_ptr &td_location) const { return false; }
} // namespace kungfu::wingchun::broker
