//
// Created by Keren Dong on 2019-06-20.
//

#include <utility>
#include <fmt/format.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::strategy
{
    Runner::Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name, longfist::enums::mode m,
                   bool low_latency)
            : apprentice(location::make_shared(m, category::STRATEGY, group, name, std::move(locator)), low_latency), started_(false)
    {}

    Context_ptr Runner::make_context()
    {
        return std::make_shared<Context>(*this, events_);
    }

    void Runner::add_strategy(const Strategy_ptr &strategy)
    {
        strategies_.push_back(strategy);
    }

    void Runner::on_trading_day(const event_ptr &event, int64_t daytime)
    {
        for (const auto &strategy : strategies_)
        {
            strategy->on_trading_day(context_, daytime);
        }
    }

    void Runner::on_start()
    {
        apprentice::on_start();

        context_ = make_context();
        context_->react();

        events_ | take_until(events_ | filter([&](const event_ptr &e)
                                              { return started_; })) |
        $([&](const event_ptr &e)
          {
              if (started_)
              {
                  return;
              }
              for (auto &account_location : context_->list_accounts())
              {
                  if (not has_writer(account_location->uid))
                  {
                      return;
                  }
              }
              started_ = true;
              SPDLOG_INFO("strategy {} started", get_io_device()->get_home()->name);
              for (const auto &strategy : strategies_)
              {
                  strategy->post_start(context_);
              }
          });

        events_ | is(Quote::tag) | filter([=](const event_ptr &event)
                                          { return context_->is_subscribed(event->data<Quote>()); }) |
        $([&](const event_ptr &event)
          {
              context_->book_context_->on_quote(event, event->data<Quote>());
              for (const auto &strategy : strategies_)
              {
                  strategy->on_quote(context_, event->data<Quote>());
              }
          });

        events_ | is(Bar::tag) | filter([=](const event_ptr &event)
                                        { return context_->is_subscribed(event->data<Bar>()); }) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_bar(context_, event->data<Bar>());
              }
          });

        events_ | is(Entrust::tag) | filter([=](const event_ptr &event)
                                            { return context_->is_subscribed(event->data<Entrust>()); }) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_entrust(context_, event->data<Entrust>());
              }
          });

        events_ | is(Transaction::tag) | filter([=](const event_ptr &event)
                                                { return context_->is_subscribed(event->data<Transaction>()); }) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_transaction(context_, event->data<Transaction>());
              }
          });

        events_ | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              context_->book_context_->on_order(event, event->data<Order>());
              for (const auto &strategy : strategies_)
              {
                  strategy->on_order(context_, event->data<Order>());
              }
          });

        events_ | is(OrderActionError::tag) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_order_action_error(context_, event->data<OrderActionError>());
              }
          });

        events_ | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              context_->book_context_->on_trade(event, event->data<Trade>());
              for (const auto &strategy : strategies_)
              {
                  strategy->on_trade(context_, event->data<Trade>());
              }
          });

        events_ | is(Asset::tag) |
        $([&](const event_ptr &event)
          {
              context_->book_context_->on_asset(event, event->data<Asset>());
          });

        events_ | is(TradingDay::tag) |
        $([&](const event_ptr &event)
          {
              context_->book_context_->on_trading_day(event, event->data<TradingDay>().timestamp);
          });

        for (const auto &strategy : strategies_)
        {
            strategy->pre_start(context_);
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
