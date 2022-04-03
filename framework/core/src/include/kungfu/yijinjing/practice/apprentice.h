/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by Keren Dong on 2019-06-01.
//

#ifndef KUNGFU_APPRENTICE_H
#define KUNGFU_APPRENTICE_H

#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/hero.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::practice {
class apprentice : public hero {
public:
  explicit apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

  bool is_started() const;

  uint32_t get_master_commands_uid() const;

  int64_t get_last_active_time() const;

  int64_t get_checkin_time() const;

  int64_t get_trading_day() const;

  const cache::bank &get_state_bank() const;

  void request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time);

  void request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time);

  void request_write_to(int64_t trigger_time, uint32_t dest_id);

  void request_cached_reader_writer();

  void request_cached(uint32_t source_id);

  void add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  void add_time_interval(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  virtual void on_trading_day(const event_ptr &event, int64_t daytime);

  template <typename DataType>
  void write_to(int64_t trigger_time, DataType &data, uint32_t dest_id = yijinjing::data::location::PUBLIC) {
    get_writer(dest_id)->write(trigger_time, data);
  }

protected:
  const data::location_ptr master_home_location_;
  const data::location_ptr master_cmd_location_;
  const data::location_ptr cached_home_location_;
  cache::bank state_bank_;

  void react() final;

  void on_active() override;

  virtual void on_react();

  virtual void on_start();

  void on_register(int64_t trigger_time, const longfist::types::Register &register_data);

  void on_deregister(const event_ptr &event);

  void on_read_from(const event_ptr &event);

  void on_cached_ready_to_read();

  void on_read_from_public(const event_ptr &event);

  void on_write_to(const event_ptr &event);

  std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> timer(int64_t nanotime) {
    auto writer = get_writer(master_cmd_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    int64_t duration_ns = nanotime - now();
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](const rx::observable<event_ptr> &src) {
      return events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &event) {
               return (event->msg_type() == longfist::types::Time::tag &&
                       event->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns);
             }) |
             rx::first();
    };
  }

  template <typename Duration, typename Enabled = rx::is_duration<Duration>>
  std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> time_interval(Duration &&d) {
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    auto writer = get_writer(master_cmd_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](const rx::observable<event_ptr> &src) {
      return events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &event) {
               if (event->msg_type() == longfist::types::Time::tag &&
                   event->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns) {
                 auto writer = get_writer(master_cmd_location_->uid);
                 longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
                 r.id = timer_usage_count;
                 r.duration = duration_ns;
                 r.repeat = 1;
                 writer->close_data();
                 timer_checkpoints_[timer_usage_count] = now();
                 return true;
               } else {
                 return false;
               }
             });
    };
  }

  template <typename Duration, typename Enabled = rx::is_duration<Duration>>
  std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> timeout(Duration &&d) {
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    auto writer = get_writer(master_cmd_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](const rx::observable<event_ptr> &src) {
      return (src | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &event) {
                if (event->msg_type() != longfist::types::Time::tag) {
                  auto writer = get_writer(master_cmd_location_->uid);
                  longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
                  r.id = timer_usage_count;
                  r.duration = duration_ns;
                  r.repeat = 1;
                  writer->close_data();
                  timer_checkpoints_[timer_usage_count] = now();
                  return true;
                } else {
                  return false;
                }
              }))
          .merge(events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &event) {
                   if (event->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns) {
                     throw rx::timeout_error("timeout");
                   }
                   return false;
                 }));
    };
  }

private:
  bool started_ = false;
  int64_t last_active_time_ = INT64_MIN;
  int64_t checkin_time_ = INT64_MIN;
  int64_t trading_day_ = 0;
  int32_t timer_usage_count_ = 0;
  std::unordered_map<int, int64_t> timer_checkpoints_ = {};

  void checkin();

  void expect_start();

  template <typename DataType> void do_read_from(const event_ptr &event, uint32_t dest_id) {
    const DataType &request = event->data<DataType>();
    reader_->join(get_location(request.source_id), dest_id, request.from_time);
  }

  static void reset_time(const longfist::types::TimeReset &time_reset);
};

DECLARE_PTR(apprentice)
} // namespace kungfu::yijinjing::practice

#endif // KUNGFU_APPRENTICE_H
