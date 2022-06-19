//
// Created by Keren Dong on 2020/3/10.
//

#include <kungfu/common.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/time.h>

#include <utility>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;

namespace kungfu::wingchun::broker {
BrokerVendor::BrokerVendor(location_ptr location, bool low_latency) : apprentice(std::move(location), low_latency) {
  log::copy_log_settings(get_home(), get_home()->name);
}

void BrokerVendor::on_start() {
  events_ | is(RequestWriteTo::tag, RequestReadFrom::tag, RequestReadFromPublic::tag, RequestReadFromSync::tag) |
      $$(notify_broker_state());
}

void BrokerVendor::on_exit() {
  auto service = get_service();
  service->on_exit();
}

void BrokerVendor::notify_broker_state() {
  auto service = get_service();
  service->update_broker_state(service->get_state());
}

BrokerService::BrokerService(BrokerVendor &vendor) : vendor_(vendor), state_(BrokerState::Pending) {}

void BrokerService::on_start() {}

void BrokerService::on_exit() {}

void BrokerService::on_trading_day(const event_ptr &event, int64_t daytime) {}

int64_t BrokerService::now() const { return vendor_.now(); }

BrokerState BrokerService::get_state() { return state_; }

const std::string &BrokerService::get_config() {
  auto &config_map = get_state_bank()[boost::hana::type_c<Config>];
  auto &config_obj = config_map.at(get_home_uid());
  return config_obj.data.value;
}

const std::string &BrokerService::get_risk_setting() {
  auto &risk_setting_map = get_state_bank()[boost::hana::type_c<RiskSetting>];
  auto &risk_setting_obj = risk_setting_map.at(get_home_uid());
  return risk_setting_obj.data.value;
}

std::string BrokerService::get_runtime_folder() { return vendor_.get_locator()->layout_dir(get_home(), layout::LOG); }

const location_ptr &BrokerService::get_home() const { return vendor_.get_home(); }

uint32_t BrokerService::get_home_uid() const { return vendor_.get_home_uid(); }

writer_ptr BrokerService::get_writer(uint32_t dest_id) const { return vendor_.get_writer(dest_id); }

bool has_writer(uint32_t dest_id) const { return vendor_.has_writer(dest_id); }

const cache::bank &BrokerService::get_state_bank() const { return vendor_.get_state_bank(); }

bool BrokerService::check_if_stored_instruments(const std::string &trading_day) {
  SPDLOG_INFO("CHECK_IF_STORED_INSTRUMENTS trading_day {}", trading_day);
  auto &time_key_value_map = get_state_bank()[boost::hana::type_c<TimeKeyValue>];
  for (auto &pair : time_key_value_map) {
    const TimeKeyValue &timeKeyValue = pair.second.data;
    if (timeKeyValue.key == "instrument_stored_trading_day" ||
        timeKeyValue.key == "instrument_stored_trading_day_next_day") {
      if (timeKeyValue.value == trading_day) {
        return true;
      }
    }
  }
  return false;
}

void BrokerService::record_instruments_stored_trading_day(const std::string &trading_day) {
  auto writer = get_writer(location::PUBLIC);
  TimeKeyValue instrument_stored_trading_day_tkv = {};
  instrument_stored_trading_day_tkv.update_time = now();
  instrument_stored_trading_day_tkv.key = "instrument_stored_trading_day";
  instrument_stored_trading_day_tkv.value = trading_day;
  writer->write(now(), instrument_stored_trading_day_tkv);

  //为了解决夜盘的问题
  TimeKeyValue instrument_stored_trading_day_next_day_tkv = {};
  instrument_stored_trading_day_next_day_tkv.update_time = time::next_trading_day_end(now());
  instrument_stored_trading_day_next_day_tkv.key = "instrument_stored_trading_day_next_day";
  instrument_stored_trading_day_next_day_tkv.value = trading_day;
  writer->write(now(), instrument_stored_trading_day_next_day_tkv);

  SPDLOG_INFO("INSTRUMENT_STORED_TRADING_DAY {}", trading_day);
}

void BrokerService::add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback) {
  vendor_.add_timer(nanotime, callback);
}

void BrokerService::add_time_interval(int64_t nanotime, const std::function<void(const event_ptr &)> &callback) {
  vendor_.add_time_interval(nanotime, callback);
}

void BrokerService::update_broker_state(BrokerState state) {
  state_ = state;
  auto writer = get_writer(location::PUBLIC);
  BrokerStateUpdate &update = writer->open_data<BrokerStateUpdate>();
  update.state = state_;
  writer->close_data();
}
} // namespace kungfu::wingchun::broker
