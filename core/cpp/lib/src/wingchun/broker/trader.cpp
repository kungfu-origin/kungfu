//
// Created by Keren Dong on 2019-06-20.
//

#include <utility>

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/broker/trader.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker
{
    Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id) :
            apprentice(location::make_shared(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
            source_(source), account_id_(account_id)
    {
        log::copy_log_settings(get_io_device()->get_home(), account_id);
    }

    void Trader::on_start()
    {
        apprentice::on_start();

        events_ | is(OrderInput::tag) |
        $([&](const event_ptr &event)
          {
              insert_order(event);
          });

        events_ | is(OrderAction::tag) |
        $([&](const event_ptr &event)
          {
              cancel_order(event);
          });
    }

    void Trader::publish_state(longfist::enums::BrokerState state)
    {
        longfist::types::BrokerStateUpdate update{};
        update.state = state;
        write_to(0, update);
    }
}
