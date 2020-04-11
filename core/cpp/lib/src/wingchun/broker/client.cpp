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
  if (app.get_last_active_time() == 0) {
    return broker.checkin_time;
  }
  if (app.get_last_active_time() == INT64_MAX) {
    return app.get_checkin_time();
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
  return get_resmue_time(app, broker);
}

int64_t ContinuousResumePolicy::get_resmue_time(const apprentice &app, const Register &broker) const {
  return app.get_last_active_time();
}

int64_t IntradayResumePolicy::get_resmue_time(const apprentice &app, const Register &broker) const {
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

bool Client::is_subscribed(uint32_t md_location_uid, const std::string &exchange_id,
                           const std::string &instrument_id) const {
  return instrument_keys_.find(hash_instrument(exchange_id.c_str(), instrument_id.c_str())) != instrument_keys_.end();
}

void Client::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                       const std::string &instrument_id) {
  uint32_t key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  if (instrument_keys_.find(key) != instrument_keys_.end()) {
    return;
  }
  InstrumentKey instrument_key = {};
  instrument_key.key = key;
  strcpy(instrument_key.instrument_id, instrument_id.c_str());
  strcpy(instrument_key.exchange_id, exchange_id.c_str());
  instrument_keys_.emplace(key, instrument_key);
  instrument_md_locations_.emplace(key, md_location);
}

void Client::on_start(const rx::connectable_observable<event_ptr> &events) {
  events | is(Register::tag) | $$(connect(event->data<Register>()));

  events | is(BrokerStateUpdate::tag) | $([&](const event_ptr &event) {
    auto state = event->data<BrokerStateUpdate>().state;
    auto broker_location = app_.get_location(event->source());

    bool state_ready = state == BrokerState::LoggedIn or state == BrokerState::Ready;
    bool state_reset = state == BrokerState::Connected or state == BrokerState::DisConnected;

    auto switch_broker_state = [&](category broker_category, LocationMap &ready_locations, auto on_broker_ready) {
      bool ready_recorded = ready_locations.find(broker_location->uid) != ready_locations.end();
      if (state_ready and app_.has_writer(broker_location->uid) and not ready_recorded) {
        ready_locations.emplace(broker_location->uid, broker_location);
        SPDLOG_INFO("{} {} ready, state {}", get_category_name(broker_category), broker_location->uname, (int)state);
        on_broker_ready();
      }
      if (state_reset and ready_recorded) {
        ready_locations.erase(broker_location->uid);
        SPDLOG_INFO("{} {} reset, state {}", get_category_name(broker_category), broker_location->uname, (int)state);
      }
    };

    if (broker_location->category == category::MD) {
      switch_broker_state(category::MD, ready_md_locations_,
                          [&]() { subscribe_instruments(event->gen_time(), broker_location); });
    }
    if (broker_location->category == category::TD) {
      switch_broker_state(category::TD, ready_td_locations_, [&]() {});
    }

    broker_states_.emplace(broker_location->uid, state);
  });

  events | is(Deregister::tag) | $([&](const event_ptr &event) {
    auto location_uid = event->data<Deregister>().location_uid;
    auto broker_location = app_.get_location(location_uid);
    broker_states_.emplace(location_uid, BrokerState::DisConnected);
    ready_md_locations_.erase(location_uid);
    ready_td_locations_.erase(location_uid);
    SPDLOG_INFO("{} {} reset", get_category_name(broker_location->category), broker_location->uname);
  });
}

void Client::subscribe_instruments(int64_t trigger_time, const location_ptr &md_location) {
  auto writer = app_.get_writer(md_location->uid);
  for (auto &pair : instrument_keys_) {
    if (md_location->uid == instrument_md_locations_.at(pair.second.key)->uid) {
      writer->write(trigger_time, pair.second);
    }
  }
}

void Client::connect(const Register &register_data) {
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
    app_.request_read_from(app_.now(), app_uid, app_.get_last_active_time());
    app_.request_read_from_public(app_.now(), app_uid, resume_time_point);
    SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
  }
}

AutoClient::AutoClient(apprentice &app) : Client(app) {}

const ResumePolicy &AutoClient::get_resume_policy() const { return resume_policy_; }

bool AutoClient::should_connect_md(const location_ptr &md_location) const { return true; }

bool AutoClient::should_connect_td(const location_ptr &td_location) const { return true; }

ManualClient::ManualClient(apprentice &app) : Client(app) {}

const ResumePolicy &ManualClient::get_resume_policy() const { return resume_policy_; }

bool ManualClient::is_subscribed(uint32_t md_location_uid, const std::string &exchange_id,
                                 const std::string &instrument_id) const {
  return is_all_subscribed(md_location_uid) or Client::is_subscribed(md_location_uid, exchange_id, instrument_id);
}

void ManualClient::subscribe(const location_ptr &md_location, const std::string &exchange_id,
                             const std::string &instrument_id) {
  if (not is_all_subscribed(md_location->uid)) {
    enrolled_md_locations_.emplace(md_location->uid, false);
  }
  Client::subscribe(md_location, exchange_id, instrument_id);
}

bool ManualClient::is_all_subscribed(uint32_t md_location_uid) const {
  return should_connect_md(app_.get_location(md_location_uid)) and enrolled_md_locations_.at(md_location_uid);
}

void ManualClient::subscribe_all(const location_ptr &md_location) {
  enrolled_md_locations_.emplace(md_location->uid, true);
}

void ManualClient::enroll_account(const location_ptr &td_location) {
  enrolled_td_locations_.emplace(td_location->uid, true);
}

bool ManualClient::should_connect_md(const location_ptr &md_location) const {
  return enrolled_md_locations_.find(md_location->uid) != enrolled_md_locations_.end();
}

bool ManualClient::should_connect_td(const location_ptr &td_location) const {
  return enrolled_td_locations_.find(td_location->uid) != enrolled_td_locations_.end();
}

void ManualClient::subscribe_instruments(int64_t trigger_time, const location_ptr &md_location) {
  if (is_all_subscribed(md_location->uid)) {
    app_.get_writer(md_location->uid)->mark(trigger_time, SubscribeAll::tag);
  } else {
    Client::subscribe_instruments(trigger_time, md_location);
  }
}
} // namespace kungfu::wingchun::broker
