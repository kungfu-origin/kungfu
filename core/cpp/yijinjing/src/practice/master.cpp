//
// Created by Keren Dong on 2019-06-15.
//

#include <nlohmann/json.hpp>
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

        master::master(location_ptr home, bool low_latency) : hero(home, low_latency)
        {
            log::setup_log(home, home->name);
            io_device_ = std::make_shared<io_device_master>(get_home(), low_latency);
            reader_ = io_device_->open_reader_to_subscribe();
            os::handle_os_signals();
        }

        void master::register_event_source(uint32_t source_id)
        {
            reader_->subscribe(get_location(source_id), 0, 0);
        }

        void master::try_once()
        {
            if (io_device_->get_observer()->wait())
            {
                std::string notice = io_device_->get_observer()->get_notice();
                if (notice.length() > 2)
                {
                    io_device_->get_publisher()->publish(notice);
                }
            }
            if (reader_->data_available())
            {
                auto frame = reader_->current_frame();
                switch (frame.msg_type())
                {
                    case MsgType::Subscribe:
                    {
                        auto msg = frame.data<action::Subscribe>();
//                            reader_->subscribe(*get_location(msg.source_id), msg.from_time);
                        writers_[frame.source()]->write(frame.gen_time(), frame.msg_type(), 0, &msg);
                        break;
                    }
                    default:
                        break;
                }
                reader_->next();
            }
            if (io_device_->get_service_socket()->recv() > 0)
            {
                auto msg = io_device_->get_service_socket()->last_message();
                nlohmann::json request = nlohmann::json::parse(msg);
                int pid = request["pid"];
                data::mode mode = request["mode"];
                data::category category = request["category"];
                std::string group = request["group"];
                std::string name = request["name"];
                auto location = std::make_shared<data::location>(mode, category, group, name, get_home()->locator);
                if (writers_.find(location->uid) == writers_.end())
                {
                    auto now = time::now_in_nano();
                    writers_[location->uid] = io_device_->open_writer(location->uid);
                    reader_->subscribe(location, 0, now);
                    reader_->subscribe(location, get_home()->uid, now);

                    nlohmann::json response;
                    response["success"] = true;
                    response["uid"] = location->uid;
                    io_device_->get_service_socket()->send_json(response);
                } else
                {
                    nlohmann::json response;
                    response["success"] = false;
                    io_device_->get_service_socket()->send_json(response);
                }
            }
        }
    }
}
