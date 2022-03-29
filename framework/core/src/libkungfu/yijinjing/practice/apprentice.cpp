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

#include <kungfu/common.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;

namespace kungfu::yijinjing::practice {
inline location_ptr make_master_location(const std::string &name, const locator_ptr &locator) {
  return location::make_shared(mode::LIVE, category::SYSTEM, "master", name, locator);
}

inline location_ptr maske_cached_location(const locator_ptr &locator) {
  return location::make_shared(mode::LIVE, category::SYSTEM, "service", "cached", locator);
}

apprentice::apprentice(location_ptr home, bool low_latency)
    : hero(std::make_shared<io_device_client>(home, low_latency)),
      master_home_location_(make_master_location("master", get_locator())),
      master_cmd_location_(make_master_location(fmt::format("{:08x}", get_live_home_uid()), get_locator())),
      cached_home_location_(maske_cached_location(get_locator())), state_bank_(), trading_day_(time::today_start()) {
  add_location(0, master_home_location_);
  add_location(0, master_cmd_location_);
  add_location(0, cached_home_location_);
}

bool apprentice::is_started() const { return started_; }

uint32_t apprentice::get_master_commands_uid() const { return master_cmd_location_->uid; }

int64_t apprentice::get_checkin_time() const { return checkin_time_; }

int64_t apprentice::get_last_active_time() const { return last_active_time_; }

int64_t apprentice::get_trading_day() const { return trading_day_; }

const cache::bank &apprentice::get_state_bank() const { return state_bank_; }

void apprentice::request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    require_read_from(trigger_time, master_cmd_location_->uid, source_id, from_time);
  }
}

void apprentice::request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    require_read_from_public(trigger_time, master_cmd_location_->uid, source_id, from_time);
  }
}

void apprentice::request_read_from_update(int64_t trigger_time, uint32_t source_id, int64_t from_time) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    require_read_from_update(trigger_time, master_cmd_location_->uid, source_id, from_time);
  }
}

void apprentice::request_write_to(int64_t trigger_time, uint32_t dest_id) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    require_write_to(trigger_time, master_cmd_location_->uid, dest_id);
  }
}

void apprentice::request_cached_reader_writer(const event_ptr &event) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    if (writers_.find(master_cmd_location_->uid) == writers_.end()) {
      SPDLOG_ERROR("no writer for {}", get_location_uname(master_cmd_location_->uid));
      return;
    }

    if (get_live_home_uid() != cached_home_location_->uid) {
      if (registry_.find(cached_home_location_->uid) == registry_.end()) {
        SPDLOG_ERROR("no register in registry_ {}", get_location_uname(master_cmd_location_->uid));
        return;
      }

      request_write_to(now(), cached_home_location_->uid);
      request_read_from(now(), cached_home_location_->uid, now());

    } else {
      // At cached case, pass the restore, start directly
      auto writer = get_writer(master_cmd_location_->uid);
      RequestCachedDone &rcd = writer->open_data<RequestCachedDone>();
      rcd.dest_id = get_io_device()->get_live_home()->uid;
      writer->close_data();
    }
  }
}

void apprentice::request_cached(uint32_t source_id) {
  auto writer = get_writer(source_id);
  writer->mark(now(), RequestCached::tag);
}

void apprentice::add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback) {
  events_ | timer(nanotime) |
      $(
          [&, callback](const event_ptr &event) {
            try {
              callback(event);
            } catch (const std::exception &e) {
              SPDLOG_ERROR("Unexpected exception by timer {}", e.what());
            }
          },
          [&](std::exception_ptr e) {
            try {
              std::rethrow_exception(e);
            } catch (const rx::empty_error &ex) {
              SPDLOG_WARN("{}", ex.what());
            } catch (const std::exception &ex) {
              SPDLOG_WARN("Unexpected exception by timer{}", ex.what());
            }
          });
}

void apprentice::add_time_interval(int64_t duration, const std::function<void(const event_ptr &)> &callback) {
  events_ | time_interval(std::chrono::nanoseconds(duration)) | $([&, callback](const event_ptr &event) {
    try {
      callback(event);
    } catch (const std::exception &e) {
      SPDLOG_ERROR("Unexpected exception by time_interval {}", e.what());
    }
  });
}

void apprentice::on_trading_day(const event_ptr &event, int64_t daytime) {}

void apprentice::react() {
  events_ | is(TimeReset::tag) | first() | $$(reset_time(event->data<TimeReset>()));
  events_ | is(Location::tag) | $$(add_location(event->gen_time(), event->data<Location>()));
  events_ | is(Register::tag) | $$(on_register(event->trigger_time(), event->data<Register>()));
  events_ | is(Deregister::tag) | $$(on_deregister(event));
  events_ | is(RequestReadFrom::tag) | $$(on_read_from(event));
  events_ | is(RequestReadFromPublic::tag) | $$(on_read_from_public(event));
  events_ | is(RequestReadFromUpdate::tag) | $$(on_read_from_update(event));
  events_ | is(RequestWriteTo::tag) | $$(on_write_to(event));
  events_ | is(Channel::tag) | $$(register_channel(event->gen_time(), event->data<Channel>()));
  events_ | is(TradingDay::tag) | $$(on_trading_day(event, event->data<TradingDay>().timestamp));
  events_ | is(RequestStop::tag) | to(get_home_uid()) | $$(signal_stop());
  events_ | take_until(events_ | is(RequestStart::tag)) | $$(feed_state_data(event, state_bank_));

  SPDLOG_TRACE("building reactive event handlers");
  on_react();

  if (get_io_device()->get_home()->mode == mode::LIVE) {
    auto self_register_event = events_ | skip_until(events_ | is(Register::tag) | filter([&](const event_ptr &event) {
                                                      auto uid = event->data<Register>().location_uid;
                                                      return uid == get_home_uid();
                                                    })) |
                               first();

    self_register_event | rx::timeout(seconds(180), observe_on_new_thread()) |
        $(
            [&](const event_ptr &event) {
              // this subscriber will quit when register is done, no worry for performance.
            },
            [&](std::exception_ptr e) {
              try {
                std::rethrow_exception(e);
              } catch (const timeout_error &ex) {
                SPDLOG_ERROR("app register timeout");
                hero::signal_stop();
              }
            });

    self_register_event | $([&](const event_ptr &event) {
      auto data = event->data<Register>();
      last_active_time_ = data.last_active_time;
      checkin_time_ = data.checkin_time;
      reader_->join(master_cmd_location_, get_live_home_uid(), event->gen_time());
    });

    auto cached_register_event = events_ | is(Register::tag) | filter([&](const event_ptr &event) {
                                   auto register_data = event->data<Register>();
                                   return register_data.location_uid == cached_home_location_->uid;
                                 }) |
                                 filter([&](const event_ptr &event) {
                                   if (writers_.find(master_cmd_location_->uid) != writers_.end()) {
                                     return true;
                                   }
                                   return false;
                                 }) |
                                 first();

    cached_register_event | $$(request_cached_reader_writer(event));

    checkin();
    expect_start();
  }
  if (get_io_device()->get_home()->mode == mode::REPLAY) {
    reader_->join(master_cmd_location_, get_live_home_uid(), begin_time_);
    expect_start();
  }
  if (get_io_device()->get_home()->mode == mode::BACKTEST) {
    // dest_id 0 should be configurable TODO
    auto home = get_io_device()->get_home();
    auto bt_location = location::make_shared(mode::BACKTEST, category::MD, home->group, home->name, get_locator());
    reader_->join(bt_location, 0, begin_time_);
    started_ = true;
    on_start();
  }
}

void apprentice::on_active() {}

void apprentice::on_react() {}

void apprentice::on_start() {}

void apprentice::on_register(int64_t trigger_time, const Register &register_data) {
  register_location(trigger_time, register_data);
}

void apprentice::on_deregister(const event_ptr &event) {
  uint32_t location_uid = data::location::make_shared(event->data<Deregister>(), get_locator())->uid;
  reader_->disjoin(location_uid);
  deregister_channel(location_uid);
  deregister_location(event->trigger_time(), location_uid);
}

void apprentice::on_read_from(const event_ptr &event) {
  do_read_from<RequestReadFrom>(event, get_live_home_uid());

  // In on read to ensure the write in cached established
  auto source_id = event->data<RequestReadFrom>().source_id;
  if (source_id == cached_home_location_->uid) {
    request_cached(source_id);
  }
}

void apprentice::on_read_from_public(const event_ptr &event) { do_read_from<RequestReadFromPublic>(event, 0); }

void apprentice::on_read_from_update(const event_ptr &event) { do_read_from<RequestReadFromUpdate>(event, location::UPDATE); }

void apprentice::on_write_to(const event_ptr &event) {
  auto dest_id = event->data<RequestWriteTo>().dest_id;
  if (writers_.find(dest_id) == writers_.end()) {
    writers_.emplace(dest_id, get_io_device()->open_writer(dest_id));
  }
}

void apprentice::checkin() {
  auto now = time::now_in_nano();
  nlohmann::json request;
  request["msg_type"] = Register::tag;
  request["gen_time"] = now;
  request["trigger_time"] = now;
  request["source"] = get_home_uid();
  request["dest"] = master_home_location_->uid;

  auto home = get_io_device()->get_home();
  nlohmann::json data;
  data["mode"] = home->mode;
  data["category"] = home->category;
  data["group"] = home->group;
  data["name"] = home->name;
  data["location_uid"] = home->uid;
  data["pid"] = GETPID();
  data["checkin_time"] = now;
  data["last_active_time"] = now;
  request["data"] = data;

  get_io_device()->get_publisher()->publish(request.dump());
}

void apprentice::expect_start() {
  reader_->join(master_home_location_, 0, begin_time_);
  events_ | is(RequestStart::tag) | first() |
      $(
          [&](const event_ptr &event) {
            started_ = true;
            SPDLOG_INFO("ready to start");
            on_start();
          },
          [&](const std::exception_ptr &e) {
            try {
              std::rethrow_exception(e);
            } catch (const rx::empty_error &ex) {
              SPDLOG_WARN("Unexpected rx empty {}", ex.what());
            } catch (const std::exception &ex) {
              SPDLOG_WARN("Unexpected exception before start {}", ex.what());
            }
          });
}

void apprentice::reset_time(const longfist::types::TimeReset &time_reset) {
  time::reset(time_reset.system_clock_count, time_reset.steady_clock_count);
}
} // namespace kungfu::yijinjing::practice
