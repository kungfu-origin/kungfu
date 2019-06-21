//
// Created by Keren Dong on 2019-06-15.
//

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
        master::master(location_ptr home, bool low_latency) : hero(std::make_shared<io_device_master>(home, low_latency))
        {
            SPDLOG_INFO("{} call master", get_io_device()->get_home()->uname);
            writers_[0] = get_io_device()->open_writer(0);
            writers_[0]->open_session();
        }

        void master::react(observable<event_ptr> events)
        {
            events | is(msg::type::Register) |
            $([&](event_ptr e)
              {
                  auto request_loc = e->data<nlohmann::json>();
                  auto app_location = std::make_shared<location>(
                          static_cast<mode>(request_loc["mode"]),
                          static_cast<category>(request_loc["category"]),
                          request_loc["group"], request_loc["name"],
                          get_io_device()->get_home()->locator
                  );
                  if (writers_.find(app_location->uid) == writers_.end())
                  {
                      register_location(app_location);
                      auto now = time::now_in_nano();
                      auto uid_str = fmt::format("{:08x}", app_location->uid);
                      auto master_location = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str,
                                                                        get_io_device()->get_home()->locator);
                      reader_->join(app_location, 0, now);
                      reader_->join(app_location, master_location->uid, now);

                      auto writer = get_io_device()->open_writer_at(master_location, app_location->uid);
                      writers_[app_location->uid] = writer;

                      nlohmann::json register_msg;
                      register_msg["msg_type"] = msg::type::Register;
                      register_msg["gen_time"] = now;
                      register_msg["trigger_time"] = e->gen_time();
                      register_msg["source"] = app_location->uid;
                      register_msg["data"] = request_loc;
                      get_io_device()->get_publisher()->publish(register_msg.dump());

                      msg::data::RequestPublish msg;
                      msg.dest_id = 0;
                      writer->write(e->gen_time(), msg::type::RequestPublish, &msg);
                      msg.dest_id = master_location->uid;
                      writer->write(e->gen_time(), msg::type::RequestPublish, &msg);
                      SPDLOG_INFO("request publish {}", app_location->uname);
                  } else
                  {
                      SPDLOG_ERROR("location {} has already been registered", app_location->uname);
                  }
              });

            events | is(msg::type::RequestSubscribe) |
            $([&](event_ptr e)
              {
                  auto subscribe = e->data<msg::data::RequestSubscribe>();
                  msg::data::RequestPublish publish{};
                  publish.dest_id = e->source();
                  if (writers_.find(subscribe.source_id) == writers_.end())
                  {
                      SPDLOG_ERROR("Subscribe to unknown location");
                      return;
                  }
                  if (writers_.find(e->source()) == writers_.end())
                  {
                      SPDLOG_ERROR("Unregistered request");
                      return;
                  }
                  writers_[subscribe.source_id]->write(e->gen_time(), msg::type::RequestPublish, &publish);
                  writers_[e->source()]->write(e->gen_time(), msg::type::RequestSubscribe, &subscribe);
              });
        }
    }
}
