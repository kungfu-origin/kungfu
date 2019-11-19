//
// Created by Keren Dong on 2019-06-15.
//

#include <typeinfo>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/master.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu
{
    namespace practice
    {

        master::master(location_ptr home, bool low_latency) : hero(std::make_shared<io_device_master>(home, low_latency)), last_check_(0)
        {
            writers_[0] = get_io_device()->open_writer(0);
            writers_[0]->mark(time::now_in_nano(), msg::type::SessionStart);
        }

        void master::on_exit()
        {
            writers_[0]->mark(time::now_in_nano(), msg::type::SessionEnd);
        }

        void master::on_notify()
        {
            get_io_device()->get_publisher()->notify();
        }

        void master::register_app(const event_ptr &e)
        {
            auto request_loc = e->data<nlohmann::json>();
            auto app_location = std::make_shared<location>(
                    static_cast<mode>(request_loc["mode"]),
                    static_cast<category>(request_loc["category"]),
                    request_loc["group"], request_loc["name"],
                    get_io_device()->get_home()->locator
            );

            if (has_location(app_location->uid))
            {
                SPDLOG_ERROR("location {} has already been registered", app_location->uname);
                return;
            }

            auto now = time::now_in_nano();
            auto uid_str = fmt::format("{:08x}", app_location->uid);
            auto home = get_io_device()->get_home();

            if (app_locations_.find(app_location->uid) == app_locations_.end())
            {
                auto master_location = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, home->locator);
                register_location(e->gen_time(), master_location);
                app_locations_[app_location->uid] = master_location->uid;
            }

            register_location(e->gen_time(), app_location);

            auto master_location = get_location(app_locations_[app_location->uid]);
            writers_[app_location->uid] = get_io_device()->open_writer_at(master_location, app_location->uid);

            reader_->join(app_location, 0, now);
            reader_->join(app_location, master_location->uid, now);
                
            auto &writer = writers_[app_location->uid];

            {
                auto msg = request_loc.dump();
                auto frame = writers_[0]->open_frame(e->gen_time(), msg::type::Register, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writers_[0]->close_frame(msg.length());
            }

            writer->mark(e->gen_time(), msg::type::SessionStart);

            require_write_to(app_location->uid, e->gen_time(), 0);
            require_write_to(app_location->uid, e->gen_time(), master_location->uid);

            for (const auto &item : locations_)
            {
                nlohmann::json location;
                location["mode"] = item.second->mode;
                location["category"] = item.second->category;
                location["group"] = item.second->group;
                location["name"] = item.second->name;
                auto msg = location.dump();
                SPDLOG_DEBUG("adding location {}", msg);
                auto &&frame = writer->open_frame(e->gen_time(), msg::type::Location, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
            }

            for(const auto& item: channels_)
            {
                writer->write(e->gen_time(), msg::type::Channel, item.second);
            }

            on_register(e, app_location);

            writer->mark(e->gen_time(), msg::type::RequestStart);
        }

        void master::deregister_app(int64_t trigger_time, uint32_t app_location_uid)
        {
            auto location = get_location(app_location_uid);
            nlohmann::json location_desc{};
            location_desc["mode"] = location->mode;
            location_desc["category"] = location->category;
            location_desc["group"] = location->group;
            location_desc["name"] = location->name;
            location_desc["uname"] = location->uname;
            location_desc["uid"] = app_location_uid;

            writers_[app_location_uid]->mark(trigger_time, msg::type::SessionEnd);

            deregister_channel_by_source(app_location_uid);

            deregister_location(trigger_time, app_location_uid);
            reader_->disjoin(app_location_uid);
            writers_.erase(app_location_uid);
            timer_tasks_.erase(app_location_uid);

            auto msg = location_desc.dump();
            auto &&frame = writers_[0]->open_frame(trigger_time, msg::type::Deregister, msg.length());
            memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
            writers_[0]->close_frame(msg.length());

        }

        void master::publish_time(int32_t msg_type, int64_t nanotime)
        {
            writers_[0]->write(0, msg_type, nanotime);
        }

        void master::send_time(uint32_t dest, int32_t msg_type, int64_t nanotime)
        {
            if (has_location(dest))
            {
                writers_[dest]->write(0, msg_type, nanotime);
            } else
            {
                SPDLOG_ERROR("Can not send time to {:08x}", dest);
            }
        }

        void master::register_channel(int64_t trigger_time, const yijinjing::msg::data::Channel &channel)
        {
            hero::register_channel(trigger_time, channel);
            writers_[0]->write(trigger_time, msg::type::Channel, channel);
        }

        bool master::produce_one(const rx::subscriber<yijinjing::event_ptr> &sb)
        {
            auto now = time::now_in_nano();

            for (auto &app : timer_tasks_)
            {
                uint32_t app_id = app.first;
                auto &app_tasks = app.second;
                for (auto it = app_tasks.begin(); it != app_tasks.end();)
                {
                    auto &task = it->second;
                    if (task.checkpoint <= now)
                    {
                        writers_[app_id]->mark(0, msg::type::Time);
                        SPDLOG_DEBUG("sent time event to {}", get_location(app_id)->uname);
                        task.checkpoint += task.duration;
                        task.repeat_count++;
                        if (task.repeat_count >= task.repeat_limit)
                        {
                            it = app_tasks.erase(it);
                            continue;
                        }
                    }
                    it++;
                }
            }

            if (last_check_ + time_unit::NANOSECONDS_PER_SECOND < now)
            {
                on_interval_check(now);
                last_check_ = now;
            }

            return hero::produce_one(sb);
        }

        void master::react()
        {
            events_ | is(msg::type::Register) |
            $([&](event_ptr e)
              {
                  register_app(e);
              });

            events_ | is(msg::type::RequestWriteTo) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestWriteTo &request = e->data<msg::data::RequestWriteTo>();
                  if (has_location(request.dest_id))
                  {
                      reader_->join(get_location(e->source()), request.dest_id, e->gen_time());
                      require_write_to(e->source(), e->gen_time(), request.dest_id);
                      require_read_from(request.dest_id, e->gen_time(), e->source(), false);
                      msg::data::Channel channel = {};
                      channel.source_id = e->source();
                      channel.dest_id = request.dest_id;
                      register_channel(e->gen_time(), channel);
                  } else
                  {
                      SPDLOG_ERROR("Request write to unknown location {:08x}", request.dest_id);
                  }
              });

            events_ | is(msg::type::RequestReadFrom) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestReadFrom &request = e->data<msg::data::RequestReadFrom>();
                  if (has_location(request.source_id))
                  {
                      reader_->join(get_location(request.source_id), e->source(), e->gen_time());
                      require_write_to(request.source_id, e->gen_time(), e->source());
                      require_read_from(e->source(), e->gen_time(), request.source_id, false);
                      msg::data::Channel channel = {};
                      channel.source_id = request.source_id;
                      channel.dest_id = e->source();
                      register_channel(e->gen_time(), channel);
                  } else
                  {
                      SPDLOG_ERROR("Request read from unknown location {:08x}", request.source_id);
                  }
              });

            events_ | is(msg::type::RequestReadFromPublic) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestReadFrom &request = e->data<msg::data::RequestReadFrom>();
                  if (has_location(request.source_id))
                  {
                      require_read_from(e->source(), e->gen_time(), request.source_id, true);
                  } else
                  {
                      SPDLOG_ERROR("Request read public from unknown location {:08x}", request.source_id);
                  }
              });

            events_ | is(msg::type::TimeRequest) |
            $([&](event_ptr e)
              {
                  const msg::data::TimeRequest &request = e->data<msg::data::TimeRequest>();
                  if (timer_tasks_.find(e->source()) == timer_tasks_.end())
                  {
                      timer_tasks_[e->source()] = std::unordered_map<int32_t, TimerTask>();
                  }
                  std::unordered_map<int32_t, TimerTask> &app_tasks = timer_tasks_[e->source()];
                  if (app_tasks.find(request.id) == app_tasks.end())
                  {
                      app_tasks[request.id] = TimerTask();
                  }
                  TimerTask &task = app_tasks[request.id];
                  task.checkpoint = time::now_in_nano() + request.duration;
                  task.duration = request.duration;
                  task.repeat_count = 0;
                  task.repeat_limit = request.repeat;
                  SPDLOG_DEBUG("time request from {} duration {} repeat {}", get_location(e->source())->uname, request.duration, request.repeat);
              });
        }
    }
}
