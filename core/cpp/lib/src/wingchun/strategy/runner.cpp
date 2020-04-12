//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::strategy {
Runner::Runner(locator_ptr locator, const std::string &group, const std::string &name, mode m, bool low_latency)
    : apprentice(location::make_shared(m, category::STRATEGY, group, name, std::move(locator)), low_latency),
      ledger_location_(mode::LIVE, category::SYSTEM, "service", "ledger", get_locator()),
      positions_set_(m == mode::BACKTEST) {}

Context_ptr Runner::get_context() const { return context_; }

Context_ptr Runner::make_context() { return std::make_shared<Context>(*this, events_); }

void Runner::add_strategy(const Strategy_ptr &strategy) { strategies_.push_back(strategy); }

void Runner::on_trading_day(const event_ptr &event, int64_t daytime) {
  if (context_) {
    context_->get_bookkeeper().on_trading_day(daytime);
  }
  invoke(&Strategy::on_trading_day, daytime);
}

void Runner::on_react() { context_ = make_context(); }

void Runner::on_start() {
  enable(*context_);
  pre_start();
  events_ | take_until(events_ | filter([this](auto e) { return started_; })) | $$(prepare(event));
}

void Runner::on_active() {
  if (not is_live()) {
    pre_stop();
  }
}

void Runner::on_exit() { post_stop(); }

void Runner::pre_start() { invoke(&Strategy::pre_start); }

void Runner::post_start() {
  events_ | is(Quote::tag) | $$(invoke(&Strategy::on_quote, event->data<Quote>()));
  events_ | is(Bar::tag) | $$(invoke(&Strategy::on_bar, event->data<Bar>()));
  events_ | is(Order::tag) | $$(invoke(&Strategy::on_order, event->data<Order>()));
  events_ | is(Trade::tag) | $$(invoke(&Strategy::on_trade, event->data<Trade>()));
  events_ | is(Entrust::tag) | $$(invoke(&Strategy::on_entrust, event->data<Entrust>()));
  events_ | is(Transaction::tag) | $$(invoke(&Strategy::on_transaction, event->data<Transaction>()));
  events_ | is(OrderActionError::tag) | $$(invoke(&Strategy::on_order_action_error, event->data<OrderActionError>()));

  invoke(&Strategy::post_start);
  SPDLOG_INFO("strategy {} started", get_io_device()->get_home()->name);
}

void Runner::pre_stop() { invoke(&Strategy::pre_stop); }

void Runner::post_stop() { invoke(&Strategy::post_stop); }

void Runner::prepare(const event_ptr &event) {
  auto ready_test = [&](auto &locations) {
    for (const auto &pair : locations) {
      if (not context_->get_broker_client().is_ready(pair.second->uid)) {
        return false;
      }
    }
    return true;
  };
  if (not ready_test(context_->list_accounts()) or not ready_test(context_->list_md())) {
    return;
  }
  if (not positions_requested_ and has_channel(get_home_uid(), ledger_location_.uid)) {
    auto writer = get_writer(ledger_location_.uid);
    for (const auto &pair : context_->get_broker_client().get_instrument_keys()) {
      writer->write(now(), pair.second);
    }
    writer->mark(now(), PositionRequest::tag);
    positions_requested_ = true;
    return;
  }
  if (event->msg_type() == PositionEnd::tag and event->source() == ledger_location_.uid) {
    positions_set_ = true;
  }
  if (not positions_set_) {
    return;
  }
  started_ = true;
  post_start();
}
} // namespace kungfu::wingchun::strategy
