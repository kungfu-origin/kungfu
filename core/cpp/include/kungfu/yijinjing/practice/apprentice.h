/*****************************************************************************
 * Copyright [taurus.ai]
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

#include <unordered_map>

#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/hero.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::practice {
class apprentice : public hero {
public:
  explicit apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

  index::session_finder &get_session_finder();

  bool is_started() const;

  uint32_t get_master_commands_uid() const;

  int64_t get_master_start_time() const;

  int64_t get_trading_day() const;

  yijinjing::data::location_ptr get_config_location() const;

  const cache::bank &get_state_bank() const;

  void request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time);

  void request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time);

  void request_write_to(int64_t trigger_time, uint32_t dest_id);

  void add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  void add_time_interval(int64_t nanotime, const std::function<void(const event_ptr &)> &callback);

  virtual void on_trading_day(const event_ptr &event, int64_t daytime);

  template <typename HandlerType> void for_each_live_location(HandlerType handler) {
    for (const auto &pair : registry_) {
      handler(pair.second);
    }
  }

  template <typename DataType>
  void write_to(int64_t trigger_time, DataType &data, uint32_t dest_id = yijinjing::data::location::PUBLIC) {
    get_writer(dest_id)->write(trigger_time, data);
  }

protected:
  cache::bank state_bank_;
  const data::location_ptr config_location_;
  const data::location_ptr master_home_location_;
  const data::location_ptr master_commands_location_;

  void react() final;

  void on_active() override;

  virtual void on_ready();

  virtual void on_start();

  void on_read_from(const event_ptr &event);

  void on_read_from_public(const event_ptr &event);

  void on_write_to(const event_ptr &event);

  std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> timer(int64_t nanotime) {
    auto writer = get_writer(master_commands_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    int64_t duration_ns = nanotime - now();
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src) {
      return events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e) {
               return (e->msg_type() == longfist::types::Time::tag &&
                       e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns);
             }) |
             rx::first();
    };
  }

  template <typename Duration, typename Enabled = rx::is_duration<Duration>>
  std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> time_interval(Duration &&d) {
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    auto writer = get_writer(master_commands_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src) {
      return events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e) {
               if (e->msg_type() == longfist::types::Time::tag &&
                   e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns) {
                 auto writer = get_writer(master_commands_location_->uid);
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
    auto writer = get_writer(master_commands_location_->uid);
    int32_t timer_usage_count = timer_usage_count_;
    longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
    r.id = timer_usage_count;
    r.duration = duration_ns;
    r.repeat = 1;
    writer->close_data();
    timer_checkpoints_[timer_usage_count] = now();
    timer_usage_count_++;
    return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src) {
      return (src | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e) {
                if (e->msg_type() != longfist::types::Time::tag) {
                  auto writer = get_writer(master_commands_location_->uid);
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
          .merge(events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e) {
                   if (e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns) {
                     throw rx::timeout_error("timeout");
                   }
                   return false;
                 }));
    };
  }

private:
  bool started_;
  int64_t master_start_time_;
  int64_t trading_day_;
  int32_t timer_usage_count_;
  std::unordered_map<int, int64_t> timer_checkpoints_ = {};
  index::session_finder session_finder_;

  void checkin();

  void expect_start();

  template <typename DataType> void do_read_from(const event_ptr &event, uint32_t dest_id) {
    const DataType &request = event->data<DataType>();
    reader_->join(get_location(request.source_id), dest_id, request.from_time);
  }
};

DECLARE_PTR(apprentice)
} // namespace kungfu::yijinjing::practice

#endif // KUNGFU_APPRENTICE_H
