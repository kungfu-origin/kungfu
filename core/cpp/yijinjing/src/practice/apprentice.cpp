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

#include <signal.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/apprentice.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;

namespace kungfu
{
    namespace practice
    {
        apprentice::apprentice(location_ptr home, bool low_latency) :
                hero(std::make_shared<io_device_client>(home, low_latency)), timer_usage_count_(0)
        {
            auto uid_str = fmt::format("{:08x}", get_live_home_uid());
            auto locator = get_io_device()->get_home()->locator;
            master_home_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", "master", locator);
            master_commands_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, locator);
            config_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "etc", "kungfu", locator);
        }

        void apprentice::request_write_to(int64_t trigger_time, uint32_t dest_id)
        {
            require_write_to(master_commands_location_->uid, trigger_time, dest_id);
        }

        void apprentice::request_read_from(int64_t trigger_time, uint32_t source_id, bool pub)
        {
            require_read_from(master_commands_location_->uid, trigger_time, source_id, pub);
        }

        void apprentice::add_timer(int64_t nanotime, const std::function<void(event_ptr)>& callback)
        {
            events_ | timer(nanotime) |
            $([&, callback](event_ptr e)
            {
                callback(e);
            });
        }

        void apprentice::add_time_interval(int64_t duration, const std::function<void(event_ptr)>& callback)
        {
            events_ | time_interval(std::chrono::nanoseconds(duration)) |
            $([&, callback](event_ptr e)
              {
                  callback(e);
              });
        }

        void apprentice::react()
        {
            if (get_io_device()->get_home()->mode == mode::LIVE)
            {
                events_ | skip_until(events_ | is(msg::type::Register) | from(master_home_location_->uid)) | first() |
                rx::timeout(seconds(1), observe_on_new_thread()) |
                $([&](event_ptr e)
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

                events_ | skip_until(events_ | is(msg::type::Register) | from(master_home_location_->uid)) | first() |
                $([&](event_ptr e)
                  {
                      reader_->join(master_commands_location_, get_live_home_uid(), e->gen_time());
                  });
            } else
            {
                reader_->join(master_commands_location_, get_live_home_uid(), begin_time_);
            }

            events_ |
            $([&](event_ptr event)
              {
                  now_ = event->gen_time();
              });

            events_ | is(msg::type::Location) |
            $([&](event_ptr e)
              {
                  register_location_from_event(e);
              });

            events_ | is(msg::type::Register) |
            $([&](event_ptr e)
              {
                  register_location_from_event(e);
              });

            events_ | is(msg::type::Deregister) |
            $([&](event_ptr e)
              {
                  reader_->disjoin(e->source());
                  writers_.erase(e->source());
                  deregister_location(e->gen_time(), e->source());
              });

            events_ | is(msg::type::RequestWriteTo) |
            $([&](event_ptr e)
              {
                  on_write_to(e);
              });

            events_ | filter([&](event_ptr e)
                            {
                                return e->msg_type() == msg::type::RequestReadFromPublic or e->msg_type() == msg::type::RequestReadFrom;
                            }) |
            $([&](event_ptr e)
              {
                  on_read_from(e);
              });

            events_ | is(msg::type::RequestStart) | first() |
            $([&](event_ptr e)
              {
                  on_start();
              });

            events_ | is(msg::type::TradingDay) |
            $([&](event_ptr e)
              {
                  on_trading_day(e, e->data<int64_t>());
              });

            reader_->join(master_home_location_, 0, begin_time_);
            if (get_io_device()->get_home()->mode == mode::LIVE)
            {
                checkin();
            }
        }

        void apprentice::on_write_to(const yijinjing::event_ptr &event)
        {
            const msg::data::RequestWriteTo &request = event->data<msg::data::RequestWriteTo>();
            if (writers_.find(request.dest_id) == writers_.end())
            {
                writers_[request.dest_id] = get_io_device()->open_writer(request.dest_id);
                if (request.dest_id == 0)
                {
                    writers_[request.dest_id]->mark(event->trigger_time(), msg::type::SessionStart);
                }
            } else
            {
                SPDLOG_ERROR("{} [{:08x}] asks publish to {} [{:08x}] for more than once",
                             get_location(event->source())->uname, event->source(),
                             get_location(request.dest_id)->uname, request.dest_id);
            }
        }

        void apprentice::on_read_from(const yijinjing::event_ptr &event)
        {
            const msg::data::RequestReadFrom &request = event->data<msg::data::RequestReadFrom>();
            SPDLOG_INFO("{} [{:08x}] asks observe at {} [{:08x}] {} from {}", get_location(event->source())->uname, event->source(),
                        get_location(request.source_id)->uname, request.source_id, time::strftime(event->gen_time()),
                        time::strftime(request.from_time));
            uint32_t dest_id = event->msg_type() == msg::type::RequestReadFromPublic ? 0 : get_live_home_uid();
            reader_->join(get_location(request.source_id), dest_id, request.from_time);
        }

        void apprentice::checkin()
        {
            auto now = time::now_in_nano();
            nlohmann::json request;
            request["msg_type"] = msg::type::Register;
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
#ifdef _WINDOWS
            data["pid"] = _getpid();
#else
            data["pid"] = getpid();
#endif

            request["data"] = data;

            SPDLOG_DEBUG("request {}", request.dump());
            get_io_device()->get_publisher()->publish(request.dump());
        }

        void apprentice::register_location_from_event(const yijinjing::event_ptr &event)
        {
            const char *buffer = &(event->data<char>());
            std::string json_str{};
            json_str.assign(buffer, event->data_length());
            nlohmann::json location_json = nlohmann::json::parse(json_str);
            auto app_location = std::make_shared<location>(
                    static_cast<mode>(location_json["mode"]),
                    static_cast<category>(location_json["category"]),
                    location_json["group"], location_json["name"],
                    get_io_device()->get_home()->locator
            );
            register_location(event->trigger_time(), app_location);
        }
    }
}

class app
{
private:
    int64_t now_;
    int64_t start_time_;
    int64_t last_event_time_;
    connectable_observable<event_ptr> events_;
    observable<int64_t> time_events_;
public:
    app() : now_(0), last_event_time_(0)
    {}

    template<typename Duration,
            typename Enabled = rxu::enable_if_all_true_type_t<rxu::is_duration<Duration>>>
    std::function<observable<event_ptr>(observable<event_ptr>)> jtimeout(Duration &&d)
    {
        auto ns = duration_cast<nanoseconds>(d).count();
        return [&, ns](observable<event_ptr> src)
        {
            events_ | first() |
            $([&](event_ptr e)
              {
                  start_time_ = e->gen_time();
                  std::cout << "check   = " << time::strftime(start_time_) << std::endl;
                  std::cout << "timeout = " << ns << std::endl;
                  std::cout << "------------ start ------------" << std::endl;
              });
            return (src | filter([&, ns](event_ptr e)
                                 {
                                     start_time_ = e->gen_time();
                                     std::cout << "check   = " << time::strftime(start_time_) << std::endl;
                                     return true;
                                 })).merge(events_ | filter([&, ns](event_ptr e)
                                                            {
                                                                std::cout << "now_    = " << time::strftime(e->gen_time()) << std::endl;
                                                                if (e->gen_time() > start_time_ + ns)
                                                                {
                                                                    throw timeout_error("timeout");
                                                                }
                                                                return false;
                                                            }));
        };
    }

    void run()
    {
        events_ = observable<>::create<event_ptr>(
                [&](subscriber<event_ptr> sb)
                {
                    auto now = time::now_in_nano();
                    nlohmann::json e1{
                            {"gen_time", now},
                            {"msg_type", 1},
                            {"source",   1},
                            {"data",     {
                                                 {"dest_id", 2}
                                         }}
                    };
                    nlohmann::json e2{
                            {"gen_time", now + duration_cast<nanoseconds>(std::chrono::milliseconds(500)).count()},
                            {"msg_type", 2},
                            {"source",   1},
                            {"data",     {
                                                 {"dest_id", 2}
                                         }}
                    };
                    nlohmann::json t1{
                            {"gen_time", now + duration_cast<nanoseconds>(std::chrono::milliseconds(1000)).count()},
                            {"msg_type", 9},
                            {"source",   1}
                    };
                    nlohmann::json e3{
                            {"gen_time", now + duration_cast<nanoseconds>(std::chrono::milliseconds(1000)).count() + 1},
                            {"msg_type", 3},
                            {"source",   1},
                            {"data",     {
                                                 {"dest_id", 2}
                                         }}
                    };
                    sb.on_next(std::make_shared<nanomsg::nanomsg_json>(e1.dump()));
                    sb.on_next(std::make_shared<nanomsg::nanomsg_json>(e2.dump()));
                    sb.on_next(std::make_shared<nanomsg::nanomsg_json>(t1.dump()));
                    sb.on_next(std::make_shared<nanomsg::nanomsg_json>(e3.dump()));
                }).publish();

        time_events_ = events_ | map([&](event_ptr e)
                                     { return e->gen_time(); });

        events_ |
        $([&](event_ptr e)
          {
              now_ = e->gen_time();
          });

        events_ | is(3) | jtimeout(std::chrono::seconds(1)) |
        $([&](event_ptr e)
          {
              std::cout << "good" << std::endl;
          },
          [&](std::exception_ptr e)
          {
              try
              { std::rethrow_exception(e); }
              catch (const timeout_error &ex)
              {
                  SPDLOG_ERROR("timeout");
              }
          });

        std::cout << sizeof(journal::frame_header) << std::endl;

        events_.connect();

        std::shared_ptr<std::vector<char>> vp;
        vp->insert(vp->end(), 0);
    }
};

int main(int argc, char **argv)
{
    app a{};
    a.run();
}