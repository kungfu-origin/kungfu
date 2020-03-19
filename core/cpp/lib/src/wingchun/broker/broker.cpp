//
// Created by Keren Dong on 2020/3/10.
//

#include <kungfu/common.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker {
Broker::Broker(yijinjing::data::location_ptr location, bool low_latency)
    : apprentice(location, low_latency), state_(BrokerState::Unknown) {
  auto home = get_io_device()->get_home();
  log::copy_log_settings(home, home->name);
}

void Broker::on_start() {
  events_ | is(RequestWriteTo::tag, RequestReadFrom::tag, RequestReadFromPublic::tag) | $([&](const event_ptr &event) {
    longfist::types::BrokerStateUpdate update = {};
    update.state = state_;
    write_to(event->gen_time(), update);
  });
}

std::string Broker::get_runtime_folder() {
  auto home = get_io_device()->get_home();
  return home->locator->layout_dir(home, longfist::enums::layout::LOG);
}

void Broker::update_broker_state(longfist::enums::BrokerState state) {
  state_ = state;
  longfist::types::BrokerStateUpdate update = {};
  update.state = state_;
  write_to(0, update);
}
} // namespace kungfu::wingchun::broker
