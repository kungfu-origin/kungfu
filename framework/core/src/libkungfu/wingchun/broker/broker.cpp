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
BrokerVendor::BrokerVendor(location_ptr location, bool low_latency)
    : apprentice(std::move(location), low_latency) {
  log::copy_log_settings(get_home(), get_home()->name);
}

void BrokerVendor::on_start() {
  events_ | is(RequestWriteTo::tag, RequestReadFrom::tag, RequestReadFromPublic::tag) | $$(notify_broker_state());
}

void BrokerVendor::notify_broker_state() {
  auto service = get_service();
  service->update_broker_state(service->get_state());
}

BrokerService::BrokerService(BrokerVendor &vendor) : vendor_(vendor), state_(BrokerState::Pending) {}

void BrokerService::on_start() {}

std::string BrokerService::get_runtime_folder() {
  return vendor_.get_locator()->layout_dir(vendor_.get_home(), layout::LOG);
}

BrokerState BrokerService::get_state() { return state_; }

void BrokerService::update_broker_state(BrokerState state) {
  state_ = state;
  auto writer = get_writer(location::PUBLIC);
  BrokerStateUpdate &update = writer->open_data<BrokerStateUpdate>();
  update.state = state_;
  writer->close_data();
}

writer_ptr BrokerService::get_writer(uint32_t dest_id) const { return vendor_.get_writer(dest_id); }
} // namespace kungfu::wingchun::broker
