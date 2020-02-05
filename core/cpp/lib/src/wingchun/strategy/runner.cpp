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

    void Runner::on_trading_day(const event_ptr &event, int64_t daytime)
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

        events_ | timer(now() + yijinjing::time_unit::NANOSECONDS_PER_SECOND * 2) |
        $([&](const event_ptr &e)
          {
              auto book_ctx = context_->get_book_context();
              auto books = book_ctx->get_books();
              bool ready = std::all_of(books.begin(), books.end(), [](book::Book_ptr book)
              { return book->is_ready(); });
              if (ready)
              {
                  for (const auto &strategy : strategies_)
                  {
                      strategy->post_start(context_);
                  }
              } else
              {
                  SPDLOG_WARN("failed to initialize books");
                  signal_stop();
              }
          },
          [&](std::exception_ptr e)
          {
              try
              { std::rethrow_exception(e); }
              catch (const rx::empty_error &ex)
              {
                  SPDLOG_WARN("{}", ex.what());
              }
              catch (const std::exception &ex)
              {
                  SPDLOG_WARN("Unexpected exception by timer{}", ex.what());
              }
          });

        for (const auto &strategy : strategies_)
        {
            strategy->pre_start(context_);
        }

        events_ | is(Quote::tag) | filter([=](const event_ptr &event)
                                     { return context_->is_subscribed(event->data<Quote>()); }) |
        $([&](const event_ptr &event)
          {
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

        events_ | is(Order::tag) | to(context_->app_.get_home_uid()) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_order(context_, event->data<Order>());
              }
          });

        events_ | is(OrderActionError::tag) | to(context_->app_.get_home_uid()) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_order_action_error(context_, event->data<OrderActionError>());
              }
          });

        events_ | is(Trade::tag) | to(context_->app_.get_home_uid()) |
        $([&](const event_ptr &event)
          {
              for (const auto &strategy : strategies_)
              {
                  strategy->on_trade(context_, event->data<Trade>());
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

        apprentice::on_start();

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
