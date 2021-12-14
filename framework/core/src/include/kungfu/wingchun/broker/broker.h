//
// Created by Keren Dong on 2020/3/10.
//

#ifndef WINGCHUN_BROKER_H
#define WINGCHUN_BROKER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {

FORWARD_DECLARE_PTR(BrokerVendor)
FORWARD_DECLARE_PTR(BrokerService)

class BrokerVendor : public yijinjing::practice::apprentice {
public:
  typedef yijinjing::data::locator_ptr locator_ptr;
  typedef yijinjing::data::location_ptr location_ptr;
  typedef longfist::enums::BrokerState BrokerState;

  BrokerVendor(location_ptr location, bool low_latency);

protected:
  void on_start() override;

  virtual BrokerService_ptr get_service() = 0;

private:
  void notify_broker_state();
};

class BrokerService {
public:
  typedef longfist::enums::BrokerState BrokerState;

  explicit BrokerService(BrokerVendor &vendor);

  virtual ~BrokerService() = default;

  virtual void on_start();

  std::string get_runtime_folder();

  BrokerState get_state();

  void update_broker_state(BrokerState state);

  [[nodiscard]] yijinjing::journal::writer_ptr get_writer(uint32_t dest_id) const;

protected:
  BrokerVendor &vendor_;

  volatile BrokerState state_;
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_BROKER_H
