//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/common.h>
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::strategy {
Runner::Runner(locator_ptr locator, const std::string &group, const std::string &name, mode m, bool low_latency)
    : apprentice(location::make_shared(m, category::STRATEGY, group, name, std::move(locator)), low_latency),
      position_set_(m == mode::BACKTEST) {}

Context_ptr Runner::get_context() const { return context_; }

Context_ptr Runner::make_context() { return std::make_shared<Context>(*this, events_); }

void Runner::add_strategy(const Strategy_ptr &strategy) { strategies_.push_back(strategy); }

void Runner::on_trading_day(const event_ptr &event, int64_t daytime) {
  if (context_) {
    context_->bookkeeper_.on_trading_day(daytime);
  }
  for (const auto &strategy : strategies_) {
    strategy->on_trading_day(context_, daytime);
  }
}

void Runner::on_react() {
  context_ = make_context();
}

void Runner::on_start() {
  context_->on_start();

  pre_start();

  events_ | take_until(events_ | filter([&](const event_ptr &e) { return started_; })) | $([&](const event_ptr &event) {
    if (event->msg_type() == PositionEnd::tag and get_location(event->source())->category == category::SYSTEM) {
      position_set_ = true;
    }
    if (not position_set_) {
      return;
    }
    for (const auto &pair : context_->list_accounts()) {
      if (not context_->broker_client_.is_ready(pair.second->uid)) {
        return;
      }
    }
    started_ = true;
    post_start();
  });
}

void Runner::on_exit() {
  for (const auto &strategy : strategies_) {
    strategy->pre_stop(context_);
  }

  apprentice::on_exit();

  for (const auto &strategy : strategies_) {
    strategy->post_stop(context_);
  }
}

void Runner::pre_start() {
  for (const auto &strategy : strategies_) {
    strategy->pre_start(context_);
  }
}

void Runner::post_start() {
  for (const auto &strategy : strategies_) {
    strategy->post_start(context_);
  }

  SPDLOG_INFO("strategy {} started", get_io_device()->get_home()->name);

  events_ | is_own<Quote>(context_) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_quote(context_, event->data<Quote>());
    }
  });

  events_ | is_own<Bar>(context_) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_bar(context_, event->data<Bar>());
    }
  });

  events_ | is_own<Entrust>(context_) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_entrust(context_, event->data<Entrust>());
    }
  });

  events_ | is_own<Transaction>(context_) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_transaction(context_, event->data<Transaction>());
    }
  });

  events_ | is(Order::tag) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_order(context_, event->data<Order>());
    }
  });

  events_ | is(OrderActionError::tag) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_order_action_error(context_, event->data<OrderActionError>());
    }
  });

  events_ | is(Trade::tag) | $([&](const event_ptr &event) {
    for (const auto &strategy : strategies_) {
      strategy->on_trade(context_, event->data<Trade>());
    }
  });
}

} // namespace kungfu::wingchun::strategy
