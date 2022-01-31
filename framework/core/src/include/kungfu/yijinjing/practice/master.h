//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_MASTER_H
#define KUNGFU_MASTER_H

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/journal/common.h>
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

  virtual void on_register(const event_ptr &event, const longfist::types::Register &register_data) = 0;

  virtual void on_interval_check(int64_t nanotime) = 0;

  virtual int64_t acquire_trading_day() = 0;

  void publish_trading_day();

  void register_app(const event_ptr &event);

  void deregister_app(int64_t trigger_time, uint32_t app_location_uid);

protected:
  void react() final;

  void on_active() final;

private:
  int64_t start_time_;
  int64_t last_check_;
  index::session_builder session_builder_;
  profile profile_;
  yijinjing::cache::bank feed_bank_;

  std::unordered_map<uint32_t, uint32_t> app_cmd_locations_ = {};
  std::unordered_map<uint32_t, cache::shift> app_cache_shift_ = {};
  std::unordered_map<uint32_t, std::unordered_map<int32_t, timer_task>> timer_tasks_ = {};

  void handle_timer_tasks();

  void handle_cached_feeds();

  void try_add_location(int64_t trigger_time, const data::location_ptr &app_location);

  void require_cached_write_to(int64_t trigger_time, uint32_t source_id, uint32_t dest_id);

  void feed(const event_ptr &event);

  void pong(const event_ptr &event);

  void on_cache_reset(const event_ptr &event);

  void on_request_write_to(const event_ptr &event);

  void on_request_read_from(const event_ptr &event);

  void on_request_read_from_public(const event_ptr &event);

  void on_channel_request(const event_ptr &event);

  void on_time_request(const event_ptr &event);

  void on_new_location(const event_ptr &event);

  void write_time_reset(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_trading_day(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_profile_data(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_registries(int64_t trigger_time, const journal::writer_ptr &writer);

  void write_channels(int64_t trigger_time, const journal::writer_ptr &writer);
};
} // namespace kungfu::yijinjing::practice
#endif // KUNGFU_MASTER_H