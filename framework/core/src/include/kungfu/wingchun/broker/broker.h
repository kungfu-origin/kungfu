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

FORWARD_DECLARE_CLASS_PTR(BrokerVendor)
FORWARD_DECLARE_CLASS_PTR(BrokerService)

class BrokerVendor : public yijinjing::practice::apprentice {
public:
  typedef yijinjing::data::locator_ptr locator_ptr;
  typedef yijinjing::data::location_ptr location_ptr;
  typedef longfist::enums::BrokerState BrokerState;

  BrokerVendor(location_ptr location, bool low_latency);

  void on_exit() override;

protected:
  virtual BrokerService_ptr get_service() = 0;

  void on_start() override;

private:
  void notify_broker_state();
};

class BrokerService {
public:
  typedef longfist::enums::BrokerState BrokerState;

  explicit BrokerService(BrokerVendor &vendor);

  virtual ~BrokerService() = default;

  virtual void on_start();

  virtual void on_exit();

  virtual void on_trading_day(const event_ptr &event, int64_t daytime);

  [[nodiscard]] int64_t now() const;

  [[nodiscard]] BrokerState get_state();

  [[nodiscard]] std::string get_runtime_folder();

  [[nodiscard]] const std::string &get_config();

  [[nodiscard]] const std::string &get_risk_setting();

  [[nodiscard]] const yijinjing::data::location_ptr &get_home() const;

  [[nodiscard]] uint32_t get_home_uid() const;

  [[nodiscard]] const yijinjing::io_device_ptr get_io_device() const;

  [[nodiscard]] yijinjing::journal::writer_ptr get_writer(uint32_t dest_id) const;

  [[nodiscard]] bool has_writer(uint32_t dest_id) const;

  template <typename DataType>
  void write_to(int64_t trigger_time, DataType &data, uint32_t dest_id = yijinjing::data::location::PUBLIC) {
    vendor_.write_to(trigger_time, data, dest_id);
  }

  [[nodiscard]] const yijinjing::cache::bank &get_state_bank() const;

  [[nodiscard]] bool check_if_stored_instruments(const std::string &trading_day);

  void record_instruments_stored_trading_day(const std::string &trading_day);

  void add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  void add_time_interval(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  void update_broker_state(BrokerState state);

protected:
  BrokerVendor &vendor_;

  volatile BrokerState state_;
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_BROKER_H
