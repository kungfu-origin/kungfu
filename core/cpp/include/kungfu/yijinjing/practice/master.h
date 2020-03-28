//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_MASTER_H
#define KUNGFU_MASTER_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/hero.h>
#include <kungfu/yijinjing/practice/profile.h>

namespace kungfu::yijinjing::practice {

struct timer_task {
  int64_t checkpoint;
  int64_t duration;
  int64_t repeat_limit;
  int64_t repeat_count;
};

class master : public hero {
public:
  explicit master(yijinjing::data::location_ptr home, bool low_latency = false);

  index::session_builder &get_session_builder();

  void on_exit() override;

  void on_notify() override;

  virtual void on_register(const event_ptr &event, const data::location_ptr &app_location) = 0;

  virtual void on_interval_check(int64_t nanotime) = 0;

  virtual int64_t acquire_trading_day() = 0;

  void publish_trading_day();

  void register_app(const event_ptr &e);

  void deregister_app(int64_t trigger_time, uint32_t app_location_uid);

protected:
  void react() final;

  void on_active() final;

private:
  int64_t start_time_;
  int64_t last_check_;
  index::session_builder session_builder_;
  profile profile_;

  std::unordered_map<uint32_t, uint32_t> app_locations_ = {};
  std::unordered_map<uint32_t, cache::shift> app_cache_shift_ = {};
  std::unordered_map<uint32_t, longfist::types::Session> app_sessions_ = {};
  std::unordered_map<uint32_t, std::unordered_map<int32_t, timer_task>> timer_tasks_ = {};

  void write_trading_day(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_locations(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_registers(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_channels(int64_t trigger_time, const journal::writer_ptr &writer);
};
} // namespace kungfu::yijinjing::practice
#endif // KUNGFU_MASTER_H
