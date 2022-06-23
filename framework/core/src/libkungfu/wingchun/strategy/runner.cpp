//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::broker;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::practice;

namespace kungfu::wingchun::strategy {
Runner::Runner(locator_ptr locator, const std::string &group, const std::string &name, mode m, bool low_latency)
    : apprentice(location::make_shared(m, category::STRATEGY, group, name, std::move(locator)), low_latency),
      positions_set_(m == mode::BACKTEST), started_(m == mode::BACKTEST) {}

RuntimeContext_ptr Runner::get_context() const { return context_; }

RuntimeContext_ptr Runner::make_context() { return std::make_shared<RuntimeContext>(*this, events_); }

void Runner::add_strategy(const Strategy_ptr &strategy) { strategies_.push_back(strategy); }

void Runner::on_exit() { post_stop(); }

void Runner::on_trading_day(const event_ptr &event, int64_t daytime) {
  if (context_) {
    context_->get_bookkeeper().on_trading_day(daytime);
  }
  invoke(&Strategy::on_trading_day, daytime);
}

void Runner::on_react() {
  context_ = make_context();
  events_ | is(Channel::tag) | $$(inspect_channel(event));
}

void Runner::inspect_channel(const event_ptr &event) {
  auto channel = event->data<Channel>();
  if (has_location(channel.source_id) and has_location(channel.dest_id)) {
    auto source_location = get_location(channel.source_id);
    auto dest_location = get_location(channel.dest_id);
    if (ledger_home_location_->uid == channel.source_id and dest_location->category == category::TD and
        context_->get_broker_client().should_connect_td(dest_location)) {
      reader_->join(source_location, channel.dest_id, event->gen_time());
    }
  }
}

void Runner::on_start() {
  enable(*context_);
  context_->get_bookkeeper().add_book_listener(std::make_shared<BookListener>(*this));
  pre_start();
  events_ | take_until(events_ | filter([&](auto e) { return started_; })) | $$(prepare(event));
  post_start();
}

void Runner::on_active() {
  if (not is_live()) {
    pre_stop();
  }
}

void Runner::pre_start() { invoke(&Strategy::pre_start); }

void Runner::post_start() {
  if (not started_) {
    return; // safe guard for live mode, in that case we will run truly when prepare process is done.
  }

  events_ | is_own<Quote>(context_->get_broker_client()) | $$(invoke(&Strategy::on_quote, event->data<Quote>()));
  events_ | is_own<Bar>(context_->get_broker_client()) | $$(invoke(&Strategy::on_bar, event->data<Bar>()));
  events_ | is_own<TopOfBook>(context_->get_broker_client()) |
      $$(invoke(&Strategy::on_top_of_book, event->data<TopOfBook>()));
  events_ | is(Order::tag) | $$(invoke(&Strategy::on_order, event->data<Order>(), event->source()));
  events_ | is(Trade::tag) | $$(invoke(&Strategy::on_trade, event->data<Trade>()));
  events_ | is(HistoryOrder::tag) | $$(invoke(&Strategy::on_history_order, event->data<HistoryOrder>()));
  events_ | is(HistoryTrade::tag) | $$(invoke(&Strategy::on_history_trade, event->data<HistoryTrade>()));
  events_ | is(Entrust::tag) | $$(invoke(&Strategy::on_entrust, event->data<Entrust>()));
  events_ | is_own<Transaction>(context_->get_broker_client()) |
      $$(invoke(&Strategy::on_transaction, event->data<Transaction>()));
  events_ | is(OrderActionError::tag) | $$(invoke(&Strategy::on_order_action_error, event->data<OrderActionError>()));
  events_ | is_own_reg<Deregister>(context_->get_broker_client()) |
      $$(invoke(&Strategy::on_deregister, event->data<Deregister>()));
  events_ | is_own_updata_state(context_->get_broker_client()) |
      $$(invoke(&Strategy::on_broker_state_change, event->data<BrokerStateUpdate>(), get_location(event->source())));

  invoke(&Strategy::post_start);
  SPDLOG_INFO("strategy {} started", get_io_device()->get_home()->name);
}

void Runner::pre_stop() { invoke(&Strategy::pre_stop); }

void Runner::post_stop() { invoke(&Strategy::post_stop); }

void Runner::prepare(const event_ptr &event) {
  if (event->msg_type() == Position::tag) {
    const Position &position = event->data<Position>();
    if (position.holder_uid == get_home_uid()) {
      context_->get_broker_client().subscribe(position.exchange_id, position.instrument_id);
    }
  }
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
  auto ledger_uid = ledger_home_location_->uid;
  if (not positions_requested_ and has_writer(ledger_uid)) {
    auto writer = get_writer(ledger_uid);

    if (not context_->is_book_held()) {
      // Start - Let ledger prepare book for strategy
      writer->mark(now(), KeepPositionsRequest::tag);
      writer->mark(now(), ResetBookRequest::tag);
    }

    for (const auto &pair : context_->get_broker_client().get_instrument_keys()) {
      writer->write(now(), pair.second);
    }
    if (context_->is_positions_mirrored()) {
      writer->mark(now(), MirrorPositionsRequest::tag);
    }
    // End - Let ledger prepare book for strategy
    if (not context_->is_book_held() and not context_->is_positions_mirrored()) {
      writer->mark(now(), RebuildPositionsRequest::tag);
    }
    // Request ledger to recover book for strategy
    writer->mark(now(), AssetRequest::tag);
    writer->mark(now(), PositionRequest::tag);
    positions_requested_ = true;
    return;
  }
  if (event->msg_type() == PositionEnd::tag and event->source() == ledger_uid) {
    positions_set_ = true;
  }
  if (not positions_set_) {
    return;
  }
  context_->get_bookkeeper().guard_positions();
  started_ = true;
  post_start();
}

Runner::BookListener::BookListener(Runner &runner) : runner_(runner) {}

void Runner::BookListener::on_book_sync_reset(const book::Book &old_book, const book::Book &new_book) {
  auto context = std::dynamic_pointer_cast<Context>(runner_.context_);
  for (const auto &strategy : runner_.strategies_) {
    strategy->on_book_sync_reset(context, old_book, new_book);
  }
}

void Runner::BookListener::on_asset_sync_reset(const longfist::types::Asset &old_asset,
                                               const longfist::types::Asset &new_asset) {
  auto context = std::dynamic_pointer_cast<Context>(runner_.context_);
  for (const auto &strategy : runner_.strategies_) {
    strategy->on_asset_sync_reset(context, old_asset, new_asset);
  }
}

} // namespace kungfu::wingchun::strategy
