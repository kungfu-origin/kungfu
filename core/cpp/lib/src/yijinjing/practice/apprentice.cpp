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
            master_start_time_(0), timer_usage_count_(0),
            state_map_(build_state_map(longfist::StateDataTypes)), recover_state(state_map_)
    {
        auto uid_str = fmt::format("{:08x}", get_live_home_uid());
        auto locator = get_io_device()->get_home()->locator;
        master_home_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", "master", locator);
        master_commands_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, locator);
        config_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "etc", "kungfu", locator);
        locations_[master_commands_location_->uid] = master_commands_location_;
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

    void apprentice::add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback)
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

    void apprentice::add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback)
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

    void apprentice::react()
    {
        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            events_ | skip_until(events_ | is(Register::tag) | from(master_home_location_->uid)) | first() |
            rx::timeout(seconds(10), observe_on_new_thread()) |
            $([&](const event_ptr &e)
              {
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
              },
              [&]()
              {
                  // once registered this subscriber finished, no worry for performance.
              });

            events_ | skip_until(events_ | is(Register::tag) | from(master_home_location_->uid)) | first() |
            $([&](const event_ptr &e)
              {
                  reader_->join(master_commands_location_, get_live_home_uid(), e->gen_time());
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
        } else if (get_io_device()->get_home()->mode == mode::REPLAY)
        {
            reader_->join(master_commands_location_, get_live_home_uid(), begin_time_);
        } else if (get_io_device()->get_home()->mode == mode::BACKTEST)
        {
            // dest_id 0 should be configurable TODO
            reader_->join(std::make_shared<location>(mode::BACKTEST, category::MD, get_io_device()->get_home()->group,
                                                     get_io_device()->get_home()->name, get_io_device()->get_home()->locator), 0, begin_time_);
        }

        events_ | is(Location::tag) | $$(register_location_from_event<Location>);
        events_ | is(Register::tag) | $$(register_location_from_event<Register>);
        events_ | is(Deregister::tag) | $$(deregister_location_from_event);

        events_ | is(RequestReadFrom::tag) | $$(on_read_from);
        events_ | is(RequestReadFromPublic::tag) | $$(on_read_from_public);
        events_ | is(RequestWriteTo::tag) | $$(on_write_to);

        events_ | take_until(events_ | is(RequestStart::tag)) |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, recover_state);
          });

        events_ | is(Channel::tag) |
        $([&](const event_ptr &e)
          {
              register_channel(e->gen_time(), e->data<Channel>());
          });

        events_ | is(TradingDay::tag) |
        $([&](const event_ptr &e)
          {
              on_trading_day(e, e->data<TradingDay>().timestamp);
          });


        if (get_io_device()->get_home()->mode != mode::BACKTEST)
        {
            reader_->join(master_home_location_, 0, begin_time_);
            events_ | is(RequestStart::tag) | first() |
            $([&](const event_ptr &e)
              {
                  master_start_time_ = e->trigger_time();
                  on_start();
              },
              [&](const std::exception_ptr &e)
              {
                  try
                  { std::rethrow_exception(e); }
                  catch (const rx::empty_error &ex)
                  { SPDLOG_WARN("{}", ex.what()); }
                  catch (const std::exception &ex)
                  { SPDLOG_WARN("Unexpected exception before start {}", ex.what()); }
              });
        } else
        {
            on_start();
        }

        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            checkin();
        }
    }

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
        if (writers_.find(request.dest_id) == writers_.end())
        {
            writers_[request.dest_id] = get_io_device()->open_writer(request.dest_id);
        }
        SPDLOG_INFO("{} requires {} write to {}",
                    get_location(event->source())->uname,
                    get_location(event->dest())->uname,
                    request.dest_id == 0 ? "public" : get_location(request.dest_id)->uname);
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
        data["uid"] = home->uid;
        data["pid"] = GETPID();
        request["data"] = data;

        SPDLOG_DEBUG("checkin request: {}", request.dump());
        get_io_device()->get_publisher()->publish(request.dump());
    }

    void apprentice::deregister_location_from_event(const event_ptr &event)
    {
        uint32_t location_uid = location::make_shared(event->data<Deregister>(), get_io_device()->get_home()->locator)->uid;
        reader_->disjoin(location_uid);
        deregister_channel_by_source(location_uid);
        deregister_location(event->trigger_time(), location_uid);
    }

}
