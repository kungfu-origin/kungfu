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

namespace kungfu::wingchun::broker {
Broker::Broker(yijinjing::data::location_ptr location, bool low_latency)
    : apprentice(std::move(location), low_latency), state_(BrokerState::Unknown) {
  log::copy_log_settings(get_home(), get_home()->name);
}

void Broker::on_start() {
  events_ | is(RequestWriteTo::tag, RequestReadFrom::tag, RequestReadFromPublic::tag) |
      $([&](const event_ptr &event) { update_broker_state(state_); });
}

std::string Broker::get_runtime_folder() { return get_locator()->layout_dir(get_home(), layout::LOG); }

void Broker::update_broker_state(BrokerState state) {
  state_ = state;
  auto writer = get_writer(location::PUBLIC);
  BrokerStateUpdate &update = writer->open_data<BrokerStateUpdate>();
  update.state = state_;
  writer->close_data();
}
} // namespace kungfu::wingchun::broker
