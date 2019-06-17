//
// Created by Keren Dong on 2019-06-15.
//

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/master.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu
{
    namespace practice
    {
        master::master(location_ptr home, bool low_latency) : hero(std::make_shared<io_device_master>(home, low_latency))
        {
            add_event_handler(shared_from_this());
        }

        void master::handle(const event_ptr e)
        {
            switch (e->msg_type())
            {
                case MsgType::Register:
                {
                    auto app_location = get_location(e->source());
                    if (writers_.find(app_location->uid) == writers_.end())
                    {
                        auto now = time::now_in_nano();
                        auto uid_str = fmt::format("{:08x}", app_location->uid);
                        auto master_location = std::make_shared<location>(mode::LIVE, category::SYSTEM, "app", uid_str, get_io_device()->get_home()->locator);
                        reader_->subscribe(app_location, 0, now);
                        reader_->subscribe(app_location, master_location->uid, now);

                        auto writer = get_io_device()->open_writer_at(master_location, app_location->uid);
                        writers_[app_location->uid] = writer;

                        action::RequestPublish msg;
                        msg.dest_id = 0;
                        writer->write(e->gen_time(), MsgType::RequestPublish, master_location->uid, &msg);
                        msg.dest_id = master_location->uid;
                        writer->write(e->gen_time(), MsgType::RequestPublish, master_location->uid, &msg);
                    }
                    break;
                }
                case MsgType::RequestSubscribe:
                {
                    auto subscribe = e->data<action::RequestSubscribe>();
                    action::RequestPublish publish{};
                    publish.dest_id = e->source();
                    if (writers_.find(subscribe.source_id) == writers_.end())
                    {
                        SPDLOG_ERROR("Subscribe to unknown location");
                        break;
                    }
                    if (writers_.find(e->source()) == writers_.end())
                    {
                        SPDLOG_ERROR("Unregistered request");
                        break;
                    }
                    writers_[subscribe.source_id]->write(e->gen_time(), MsgType::RequestPublish, get_home_uid(), &publish);
                    writers_[e->source()]->write(e->gen_time(), MsgType::RequestSubscribe, get_home_uid(), &subscribe);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}
