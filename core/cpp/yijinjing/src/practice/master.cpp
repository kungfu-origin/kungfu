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
            writers_[0] = get_io_device()->open_writer(0);
            writers_[0]->open_session();
        }

        void master::react(const observable<event_ptr> &events)
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
                  if (has_location(app_location->uid))
                  {
                      SPDLOG_ERROR("location {} has already been registered", app_location->uname);
                  } else
                  {
                      auto now = time::now_in_nano();
                      auto uid_str = fmt::format("{:08x}", app_location->uid);
                      auto home = get_io_device()->get_home();
                      auto master_location = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, home->locator);

                      register_location(app_location);
                      register_location(master_location);

                      auto writer = get_io_device()->open_writer_at(master_location, app_location->uid);
                      writers_[app_location->uid] = writer;

                      reader_->join(app_location, 0, now);
                      request_publish(app_location->uid, e->gen_time(), 0);

                      reader_->join(app_location, master_location->uid, now);
                      request_publish(app_location->uid, e->gen_time(), master_location->uid);

                      nlohmann::json register_msg;
                      register_msg["msg_type"] = msg::type::Register;
                      register_msg["gen_time"] = now;
                      register_msg["trigger_time"] = e->gen_time();
                      register_msg["source"] = app_location->uid;
                      register_msg["data"] = request_loc;
                      get_io_device()->get_publisher()->publish(register_msg.dump());

                      writer->open_frame(e->gen_time(), msg::type::RequestStart);
                      writer->close_frame(1);
                  }
              });

            events | is(msg::type::RequestPublish) |
            $([&](event_ptr e)
              {
                  auto request = e->data<msg::data::RequestPublish>();
                  if (not has_location(request.dest_id))
                  {
                      SPDLOG_ERROR("Request publish to unknown location {:08x}", request.dest_id);
                      return;
                  }
                  reader_->join(get_location(e->source()), request.dest_id, e->gen_time());
                  request_publish(e->source(), e->gen_time(), request.dest_id);
                  request_subscribe(request.dest_id, e->gen_time(), e->source());
              });

            events | is(msg::type::RequestSubscribe) |
            $([&](event_ptr e)
              {
                  auto request = e->data<msg::data::RequestSubscribe>();
                  if (not has_location(request.source_id))
                  {
                      SPDLOG_ERROR("Request subscribe to unknown location {:08x}", request.source_id);
                      return;
                  }
                  reader_->join(get_location(request.source_id), e->source(), e->gen_time());
                  request_publish(request.source_id, e->gen_time(), e->source());
                  request_subscribe(e->source(), e->gen_time(), request.source_id);
              });
        }
    }
}
