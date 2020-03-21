//
// Created by Keren Dong on 2020/3/10.
//

#ifndef WINGCHUN_BROKER_H
#define WINGCHUN_BROKER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {
class Broker : public yijinjing::practice::apprentice {
public:
  explicit Broker(yijinjing::data::location_ptr location, bool low_latency);

  ~Broker() override = default;

  void on_start() override;

  std::string get_runtime_folder();

  void update_broker_state(longfist::enums::BrokerState state);

private:
  volatile longfist::enums::BrokerState state_;
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_BROKER_H
