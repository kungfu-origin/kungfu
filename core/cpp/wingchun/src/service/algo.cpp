//
// Created by qlu on 2019-10-31.
//

#include <utility>
#include <sstream>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/utils.h>
#include <kungfu/wingchun/service/algo.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;


namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            Algo::Algo(locator_ptr locator, mode m, bool low_latency) :
                apprentice(location::make(m, category::SYSTEM, "service", "algo", std::move(locator)),low_latency)
                {
                    log::copy_log_settings(get_io_device()->get_home(), "algo");
                }

                void Algo::on_start()
                {
                    context_ = std::make_shared<algo::AlgoContext>(*this, events_);
                    context_->react();

                    events_ | is(msg::type::AlgoOrderInput) |
                    $([&](event_ptr event)
                    {
                        insert_order(event, event->data_as_string());
                    });

                    events_ | is(msg::type::OrderAction) |
                    $([&](event_ptr event)
                    {
                        cancel_order(event, event->data<msg::data::OrderAction>());
                    });

                    events_ | is(msg::type::AlgoOrderModify) |
                    $([&](event_ptr event)
                    {
                        modify_order(event, event->data_as_string());
                    });

                    apprentice::on_start();
                }

                void Algo::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location)
                {
                    if (has_location(location->uid))
                    {
                        // bypass location events from others master cmd journal
                        return;
                    }
                    apprentice::register_location(trigger_time, location);

                    switch (location->category)
                    {
                        case category::MD: {
                            request_read_from(trigger_time, location->uid, true);
                            request_write_to(trigger_time, location->uid);
                            break;
                        }
                        case category::TD: {
                            request_read_from(trigger_time, location->uid);
                            request_write_to(trigger_time, location->uid);
                            break;
                        }
                        case category::STRATEGY:
                        {
                            request_read_from(trigger_time, location->uid);
                            request_write_to(trigger_time, location->uid);
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
}
