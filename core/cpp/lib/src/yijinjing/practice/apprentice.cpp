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

#include <csignal>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/practice/apprentice.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;

namespace kungfu::yijinjing::practice
{
    apprentice::apprentice(location_ptr home, bool low_latency) :
            hero(std::make_shared<io_device_client>(home, low_latency)),
            master_start_time_(0), timer_usage_count_(0), started_(false),
            state_map_(build_state_map(longfist::StateDataTypes)), recover_state(state_map_)
    {
        auto uid_str = fmt::format("{:08x}", get_live_home_uid());
        auto locator = get_locator();

        master_home_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", "master", locator);
        add_location(0, master_home_location_);

        master_commands_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, locator);
        add_location(0, master_commands_location_);

        config_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "etc", "kungfu", locator);
        add_location(0, config_location_);
    }

    bool apprentice::is_started() const
    {
        return started_;
    }

    uint32_t apprentice::get_master_commands_uid() const
    {
        return master_commands_location_->uid;
    }

    int64_t apprentice::get_master_start_time() const
    {
        return master_start_time_;
    }

    int64_t apprentice::get_trading_day() const
    {
        return trading_day_;
    }

    location_ptr apprentice::get_config_location() const
    {
        return config_location_;
    }

    const longfist::StateMapType &apprentice::get_state_map() const
    {
        return state_map_;
    }

    void apprentice::request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time)
    {
        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            require_read_from(trigger_time, master_commands_location_->uid, source_id, from_time);
        }
    }

    void apprentice::request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time)
    {
        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            require_read_from_public(trigger_time, master_commands_location_->uid, source_id, from_time);
        }
    }

    void apprentice::request_write_to(int64_t trigger_time, uint32_t dest_id)
    {
        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            require_write_to(trigger_time, master_commands_location_->uid, dest_id);
        }
    }

    void apprentice::add_timer(int64_t nanotime, const std::function<void(const event_ptr &)> &callback)
    {
        events_ | timer(nanotime) |
        $([&, callback](const event_ptr &e)
          {
              try
              { callback(e); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception by timer {}", e.what()); }
          },
          [&](std::exception_ptr e)
          {
              try
              { std::rethrow_exception(e); }
              catch (const rx::empty_error &ex)
              { SPDLOG_WARN("{}", ex.what()); }
              catch (const std::exception &ex)
              { SPDLOG_WARN("Unexpected exception by timer{}", ex.what()); }
          });
    }

    void apprentice::add_time_interval(int64_t duration, const std::function<void(const event_ptr &)> &callback)
    {
        events_ | time_interval(std::chrono::nanoseconds(duration)) |
        $([&, callback](const event_ptr &e)
          {
              try
              { callback(e); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception by time_interval {}", e.what()); }
          });
    }

    void apprentice::on_trading_day(const event_ptr &event, int64_t daytime)
    {}

    void apprentice::react()
    {
        events_ | is(Location::tag) |
        $([&](const event_ptr &event)
          {
              add_location(event->trigger_time(), location::make_shared(event->data<Location>(), get_locator()));
          });

        events_ | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              register_location(event->trigger_time(), event->data<Register>());
          });

        events_ | is(Deregister::tag) |
        $([&](const event_ptr &event)
          {
              uint32_t location_uid = data::location::make_shared(event->data<Deregister>(), get_locator())->uid;
              reader_->disjoin(location_uid);
              deregister_channel(location_uid);
              deregister_location(event->trigger_time(), location_uid);
          });

        events_ | is(RequestReadFrom::tag) | $$(on_read_from);
        events_ | is(RequestReadFromPublic::tag) | $$(on_read_from_public);
        events_ | is(RequestWriteTo::tag) | $$(on_write_to);

        events_ | is(Channel::tag) |
        $([&](const event_ptr &event)
          {
              register_channel(event->gen_time(), event->data<Channel>());
          });

        events_ | is(TradingDay::tag) |
        $([&](const event_ptr &event)
          {
              trading_day_ = event->data<TradingDay>().timestamp;
              SPDLOG_INFO("update trading day to {}", time::strftime(trading_day_, "%Y%m%d"));
              on_trading_day(event, trading_day_);
          });

        events_ | take_until(events_ | is(RequestStart::tag)) |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, recover_state);
          });

        SPDLOG_INFO("on ready");
        on_ready();

        switch (get_io_device()->get_home()->mode)
        {
            case mode::BACKTEST:
            {
                // dest_id 0 should be configurable TODO
                auto home = get_io_device()->get_home();
                auto backtest_location = std::make_shared<location>(mode::BACKTEST, category::MD, home->group, home->name, get_locator());
                reader_->join(backtest_location, 0, begin_time_);
                started_ = true;
                SPDLOG_INFO("on start");
                on_start();
                break;
            }
            case mode::LIVE:
            {
                auto self_register_event = events_ |
                                           skip_until(events_ | is(Register::tag) | filter([&](const event_ptr &event)
                                                                                           {
                                                                                               auto uid = event->data<Register>().location_uid;
                                                                                               return uid == get_home_uid();
                                                                                           })) |
                                           first();

                self_register_event | rx::timeout(seconds(10), observe_on_new_thread()) |
                $([&](const event_ptr &event)
                  {
                      // once registered this subscriber finished, no worry for performance.
                      // timeout happens on new thread, can not subscribe journal reader here
                      // TODO find a better approach to timeout (use timestamp in journal rather than rx scheduler)
                  },
                  [&](std::exception_ptr e)
                  {
                      try
                      { std::rethrow_exception(e); }
                      catch (const timeout_error &ex)
                      {
                          SPDLOG_ERROR("app register timeout");
                          hero::signal_stop();
                      }
                  });

                self_register_event |
                $([&](const event_ptr &event)
                  {
                      reader_->join(master_commands_location_, get_live_home_uid(), event->gen_time());
                  },
                  [&](std::exception_ptr e)
                  {
                      try
                      { std::rethrow_exception(e); }
                      catch (const std::exception &ex)
                      {
                          SPDLOG_ERROR("Register failed: {}", ex.what());
                          hero::signal_stop();
                      }
                  });

                checkin();
                expect_start();
                break;
            }
            case mode::REPLAY:
            {
                reader_->join(master_commands_location_, get_live_home_uid(), begin_time_);
                expect_start();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    void apprentice::on_active()
    {}

    void apprentice::on_ready()
    {}

    void apprentice::on_start()
    {}

    void apprentice::on_read_from(const event_ptr &event)
    {
        do_read_from<RequestReadFrom>(event, get_live_home_uid());
    }

    void apprentice::on_read_from_public(const event_ptr &event)
    {
        do_read_from<RequestReadFromPublic>(event, 0);
    }

    void apprentice::on_write_to(const event_ptr &event)
    {
        const RequestWriteTo &request = event->data<RequestWriteTo>();
        auto new_writer = writers_.find(request.dest_id) == writers_.end();
        if (new_writer)
        {
            writers_.emplace(request.dest_id, get_io_device()->open_writer(request.dest_id));
        }
        SPDLOG_INFO("{} requires {} write to {}, {}",
                    get_location(event->source())->uname, get_location(event->dest())->uname,
                    request.dest_id == 0 ? "public" : get_location(request.dest_id)->uname, new_writer ? "newly added" : "duplicated");
    }

    void apprentice::checkin()
    {
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
        request["data"] = data;

        SPDLOG_DEBUG("checkin request: {}", request.dump());
        get_io_device()->get_publisher()->publish(request.dump());
    }

    void apprentice::expect_start()
    {
        reader_->join(master_home_location_, 0, begin_time_);
        events_ | is(RequestStart::tag) | first() |
        $([&](const event_ptr &e)
          {
              master_start_time_ = e->trigger_time();
              started_ = true;
              SPDLOG_INFO("on start");
              on_start();
          },
          [&](const std::exception_ptr &e)
          {
              try
              { std::rethrow_exception(e); }
              catch (const rx::empty_error &ex)
              { SPDLOG_WARN("Unexpected rx empty {}", ex.what()); }
              catch (const std::exception &ex)
              { SPDLOG_WARN("Unexpected exception before start {}", ex.what()); }
          });
    }
}
