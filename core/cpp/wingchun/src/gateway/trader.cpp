#include <utility>

//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/trader.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

typedef rxcpp::schedulers::scheduler::clock_type::time_point::duration duration_type;

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id)
                    :
                    apprentice(location::make(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
                    source_(source), account_id_(account_id),
                    calendar_(nullptr)
            {
                log::copy_log_settings(get_io_device()->get_home(), account_id);
            }

            void Trader::react(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::react(events);

                events | is(msg::type::OrderInput) |
                $([&](event_ptr event)
                  {
                      insert_order(event);
                  });

                events | is(msg::type::OrderAction) |
                $([&](event_ptr event)
                  {
                      cancel_order(event);
                  });
            }

            void Trader::on_start(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::on_start(events);
                auto home = get_io_device()->get_home();
                auto md_location = location::make(home->mode, category::MD, source_, source_, home->locator);
                request_read_from(time::now_in_nano(), md_location->uid, true);
            }
        }
    }
}
