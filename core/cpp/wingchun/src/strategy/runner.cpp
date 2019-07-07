#include <utility>

//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            Runner::Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name, yijinjing::data::mode m,
                           bool low_latency)
                    : apprentice(location::make(m, category::STRATEGY, group, name, std::move(locator)), low_latency)
            {}

            void Runner::add_strategy(const Strategy_ptr &strategy)
            {
                strategies_.push_back(strategy);
            }

            void Runner::react(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::react(events);

                context_ = std::make_shared<Context>(*this, events);
                context_->react(events);

                events | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_quote(context_, event->data<Quote>());
                      }
                  });

                events | is(msg::type::Order) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_order(context_, event->data<Order>());
                      }
                  });

                events | is(msg::type::Trade) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_trade(context_, event->data<Trade>());
                      }
                  });

                events | is(msg::type::Entrust) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_entrust(context_, event->data<Entrust>());
                      }
                  });

                events | is(msg::type::Transaction) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_transaction(context_, event->data<Transaction>());
                      }
                  });
            }

            void Runner::on_start(const rx::observable<yijinjing::event_ptr> &events)
            {
                for (const auto &strategy : strategies_)
                {
                    strategy->pre_start(context_);
                }
            }

        }
    }
}
