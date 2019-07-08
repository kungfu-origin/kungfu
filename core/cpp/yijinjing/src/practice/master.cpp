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

        void master::on_notify()
        {
            get_io_device()->get_publisher()->notify();
        }

        void master::register_app(event_ptr e)
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
            } else
            {
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
                auto writer = get_io_device()->open_writer_at(master_location, app_location->uid);
                writers_[app_location->uid] = writer;

                {
                    auto msg = request_loc.dump();
                    auto frame = writers_[0]->open_frame(e->gen_time(), msg::type::Register, msg.length());
                    SPDLOG_DEBUG("register to {}/{} location {}", writers_[0]->get_location()->uname, writers_[0]->get_dest(), msg);
                    memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                    writers_[0]->close_frame(msg.length());
                }

                reader_->join(app_location, 0, now);
                require_write_to(app_location->uid, e->gen_time(), 0);

                reader_->join(app_location, master_location->uid, now);
                require_write_to(app_location->uid, e->gen_time(), master_location->uid);

                for (const auto& item : locations_)
                {
                    nlohmann::json location;
                    location["mode"] = item.second->mode;
                    location["category"] = item.second->category;
                    location["group"] = item.second->group;
                    location["name"] = item.second->name;
                    auto msg = location.dump();
                    auto frame = writer->open_frame(e->gen_time(), msg::type::Location, msg.length());
                    SPDLOG_DEBUG("adding location {}", msg);
                    memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                    writer->close_frame(msg.length());
                }

                writer->mark(e->gen_time(), msg::type::RequestStart);
            }
        }

        void master::deregister_app(int64_t trigger_time, uint32_t app_location_uid)
        {
            deregister_location(trigger_time, app_location_uid);
            reader_->disjoin(app_location_uid);
            nlohmann::json msg{};
            auto now = time::now_in_nano();
            msg["gen_time"] = now;
            msg["trigger_time"] = now;
            msg["msg_type"] = msg::type::Deregister;
            msg["source"] = app_location_uid;
            get_io_device()->get_publisher()->publish(msg.dump());
        }

        void master::react(const observable<event_ptr> &events)
        {
            events | is(msg::type::Register) |
            $([&](event_ptr e)
              {
                  register_app(e);
              });

            events | is(msg::type::RequestWriteTo) |
            $([&](event_ptr e)
              {
                  auto request = e->data<msg::data::RequestWriteTo>();
                  if (not has_location(request.dest_id))
                  {
                      SPDLOG_ERROR("Request publish to unknown location {:08x}", request.dest_id);
                      return;
                  }
                  reader_->join(get_location(e->source()), request.dest_id, e->gen_time());
                  require_write_to(e->source(), e->gen_time(), request.dest_id);
                  require_read_from(request.dest_id, e->gen_time(), e->source(), false);
              });

            events | filter([&](event_ptr e){
                return e->msg_type() == msg::type::RequestReadFromPublic or e->msg_type() == msg::type::RequestReadFrom;
            }) |
            $([&](event_ptr e)
              {
                  auto request = e->data<msg::data::RequestReadFrom>();
                  if (not has_location(request.source_id))
                  {
                      SPDLOG_ERROR("Request subscribe to unknown location {:08x}", request.source_id);
                      return;
                  }
                  if (e->msg_type() == msg::type::RequestReadFrom)
                  {
                      reader_->join(get_location(request.source_id), e->source(), e->gen_time());
                  }
                  require_write_to(request.source_id, e->gen_time(), e->source());
                  require_read_from(e->source(), e->gen_time(), request.source_id, e->msg_type() == msg::type::RequestReadFromPublic);
              });

            events |
            filter([=](yijinjing::event_ptr e)
                   {
                       return dynamic_cast<nanomsg::nanomsg_json *>(e.get()) != nullptr;
                   }) |
            $([&](event_ptr e)
              {
                  on_notice(e);
              });
        }
    }
}
