//
// Created by Keren Dong on 2019-06-20.
//

#include <utility>

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/trader.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace broker
        {
            Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id) :
                    apprentice(location::make(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
                    source_(source), account_id_(account_id)
            {
                log::copy_log_settings(get_io_device()->get_home(), account_id);
            }

            void Trader::on_start()
            {
                apprentice::on_start();

                events_ | is(msg::type::OrderInput) |
                $([&](event_ptr event)
                  {
                      insert_order(event);
                  });

                events_ | is(msg::type::OrderAction) |
                $([&](event_ptr event)
                  {
                      cancel_order(event);
                  });
            }
        }
    }
}
