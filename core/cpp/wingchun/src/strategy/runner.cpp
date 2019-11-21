//
// Created by Keren Dong on 2019-06-20.
//

#include <utility>
#include <fmt/format.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

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

            Context_ptr Runner::make_context()
            {
                return std::make_shared<Context>(*this, events_);
            }

            void Runner::add_strategy(const Strategy_ptr &strategy)
            {
                strategies_.push_back(strategy);
            }

            void Runner::on_trading_day(const yijinjing::event_ptr &event, int64_t daytime)
            {
                for (const auto &strategy : strategies_)
                {
                    strategy->on_trading_day(context_, daytime);
                }
            }

            void Runner::on_start()
            {
                context_ = make_context();
                context_->react();

                for (const auto &strategy : strategies_)
                {
                    strategy->pre_start(context_);
                }

                events_ | is(msg::type::Quote) | filter([=](event_ptr event) { return context_->is_subscribed(event->data<Quote>());}) |
                $([&](event_ptr event)
                {
                    for (const auto &strategy : strategies_)
                    {
                        strategy->on_quote(context_, event->data<Quote>());
                    }
                });

                events_ | is(msg::type::Bar) | filter([=](event_ptr event) { return context_->is_subscribed(event->data<Bar>());}) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_bar(context_, event->data<Bar>());
                      }
                  });

                events_ | is(msg::type::Order) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_order(context_, event->data<Order>());
                      }
                  });

                events_ | is(msg::type::OrderActionError) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_order_action_error(context_, event->data<OrderActionError>());
                      }
                  });

                events_ | is(msg::type::Trade) | to(context_->app_.get_home_uid()) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_trade(context_, event->data<Trade>());
                      }
                  });

                events_ | is(msg::type::Entrust) | filter([=](event_ptr event) { return context_->is_subscribed(event->data<Entrust>());}) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_entrust(context_, event->data<Entrust>());
                      }
                  });

                events_ | is(msg::type::Transaction) | filter([=](event_ptr event) { return context_->is_subscribed(event->data<Transaction>());}) |
                $([&](event_ptr event)
                  {
                      for (const auto &strategy : strategies_)
                      {
                          strategy->on_transaction(context_, event->data<Transaction>());
                      }
                  });

                apprentice::on_start();

                for (const auto &strategy : strategies_)
                {
                    strategy->post_start(context_);
                }
            }

            void Runner::on_exit()
            {
                for (const auto &strategy : strategies_)
                {
                    strategy->pre_stop(context_);
                }

                apprentice::on_exit();

                for (const auto &strategy : strategies_)
                {
                    strategy->post_stop(context_);
                }
            }

        }
    }
}
