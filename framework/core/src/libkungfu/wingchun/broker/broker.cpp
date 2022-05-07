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
  auto &config_map = vendor_.get_state_bank()[boost::hana::type_c<Config>];
  auto &config_obj = config_map.at(vendor_.get_home_uid());
  return config_obj.data.value;
}

const std::string &BrokerService::get_risk_setting() {
  auto &risk_setting_map = vendor_.get_state_bank()[boost::hana::type_c<RiskSetting>];
  auto &risk_setting_obj = risk_setting_map.at(vendor_.get_home_uid());
  return risk_setting_obj.data.value;
}

std::string BrokerService::get_runtime_folder() {
  return vendor_.get_locator()->layout_dir(vendor_.get_home(), layout::LOG);
}

const location_ptr &BrokerService::get_home() const { return vendor_.get_home(); }

writer_ptr BrokerService::get_writer(uint32_t dest_id) const { return vendor_.get_writer(dest_id); }

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
