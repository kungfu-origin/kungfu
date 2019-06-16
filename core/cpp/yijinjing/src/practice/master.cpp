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

namespace kungfu {
    namespace practice {

            master::master(location_ptr home, bool low_latency) : home_(home)
            {
                io_device_ = io_device::create_io_device(home_, low_latency);
                reader_ = io_device_->open_reader_to_subscribe();
            }

            void master::register_event_source(uint32_t source_id)
            {
                reader_->subscribe(get_location(source_id), 0, 0);
            }

            void master::try_once()
            {
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
            }
    }
}
