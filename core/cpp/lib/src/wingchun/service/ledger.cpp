//
// Created by Keren Dong on 2019-06-28.
//

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/service/ledger.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::cache;

namespace kungfu::wingchun::service {
Ledger::Ledger(locator_ptr locator, mode m, bool low_latency)
    : apprentice(location::make_shared(m, category::SYSTEM, "service", "ledger", std::move(locator)), low_latency),
      broker_client_(*this), bookkeeper_(*this, broker_client_) {}

void Ledger::on_exit() { write_asset_snapshots(DailyAsset::tag); }

void Ledger::on_trading_day(const event_ptr &event, int64_t daytime) {
  bookkeeper_.on_trading_day(daytime);
  write_asset_snapshots(DailyAsset::tag);
}

book::Bookkeeper &Ledger::get_bookkeeper() { return bookkeeper_; }

void Ledger::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);
  restore_subscriptions();

  events_ | is(OrderInput::tag) | $$(update_order_stat(event, event->data<OrderInput>()));
  events_ | is(Order::tag) | $$(update_order_stat(event, event->data<Order>()));
  events_ | is(Trade::tag) | $$(update_order_stat(event, event->data<Trade>()));
  events_ | is(Channel::tag) | $$(inspect_channel(event->gen_time(), event->data<Channel>()));
  events_ | is(PositionRequest::tag) | $$(write_strategy_data(event->gen_time(), event->source()));
  events_ | is(AssetRequest::tag) | $$(write_book_reset(event->gen_time(), event->source()));
  events_ | is(PositionEnd::tag) | $$(update_account_book(event->gen_time(), event->data<PositionEnd>().holder_uid););

  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE, [&](auto e) { write_asset_snapshots(AssetSnapshot::tag); });
}

void Ledger::restore_subscriptions() {
  for (const auto &pair : bookkeeper_.get_books()) {
    if (pair.second->asset.ledger_category == LedgerCategory::Account) {
      refresh_account_book(now(), pair.first);
    }
  }
}

void Ledger::refresh_account_book(int64_t trigger_time, uint32_t account_uid) {
  auto account_location = get_location(account_uid);
  auto group = account_location->group;
  auto md_location = location::make_shared(account_location->mode, category::MD, group, group, get_locator());
  auto book = get_bookkeeper().get_book(account_uid);
  auto subscribe_positions = [&](auto positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
    }
  };
  subscribe_positions(book->long_positions);
  subscribe_positions(book->short_positions);
  broker_client_.try_subscribe(trigger_time, md_location);
  book->update(trigger_time);
}

OrderStat &Ledger::get_order_stat(uint64_t order_id, const event_ptr &event) {
  if (order_stats_.find(order_id) == order_stats_.end()) {
    order_stats_.try_emplace(order_id, get_home_uid(), event->source(), event->gen_time(), OrderStat());
  }
  return order_stats_.at(order_id).data;
}

void Ledger::update_order_stat(const event_ptr &event, const longfist::types::OrderInput &data) {
  write_book(event, data);
  auto &stat = get_order_stat(data.order_id, event);
  stat.order_id = data.order_id;
  stat.md_time = event->trigger_time();
  stat.input_time = event->gen_time();
}

void Ledger::update_order_stat(const event_ptr &event, const longfist::types::Order &data) {
  if (data.error_id == 0) {
    write_book(event, data);
  }
  auto &stat = get_order_stat(data.order_id, event);
  auto inserted = stat.insert_time != 0;
  auto acked = stat.ack_time != 0;
  if (not inserted) {
    stat.insert_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  }
  if (inserted and not acked) {
    stat.ack_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  }
}

void Ledger::update_order_stat(const event_ptr &event, const longfist::types::Trade &data) {
  write_book(event, data);
  auto &stat = get_order_stat(data.order_id, event);
  if (stat.trade_time == 0) {
    stat.trade_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  }
}

void Ledger::update_account_book(int64_t trigger_time, uint32_t account_uid) {
  refresh_account_book(trigger_time, account_uid);
  write_to(trigger_time, get_bookkeeper().get_book(account_uid)->asset, account_uid);
}

void Ledger::inspect_channel(int64_t trigger_time, const longfist::types::Channel &channel) {
  auto source_location = get_location(channel.source_id);
  if (channel.source_id != get_home_uid() and channel.dest_id != get_home_uid()) {
    reader_->join(source_location, channel.dest_id, trigger_time);
  }
  bool writable = channel.dest_id == get_home_uid() and has_writer(channel.source_id);
  if (writable and source_location->category == category::TD) {
    write_book_reset(trigger_time, channel.source_id);
  }
}

void Ledger::write_book_reset(int64_t trigger_time, uint32_t dest) {
  auto writer = get_writer(dest);

  writer->open_data<CacheReset>().msg_type = Asset::tag;
  writer->close_data();
  writer->mark(trigger_time, AssetRequest::tag);

  writer->open_data<CacheReset>().msg_type = Position::tag;
  writer->close_data();
  writer->mark(trigger_time, PositionRequest::tag);
}

void Ledger::write_strategy_data(int64_t trigger_time, uint32_t strategy_uid) {
  auto strategy_book = bookkeeper_.get_book(strategy_uid);
  auto writer = get_writer(strategy_uid);
  auto write_positions = [&](const auto &positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      if (strategy_book->has_position(position)) {
        Position &strategy_position = writer->open_data<Position>(trigger_time);
        longfist::copy(strategy_position, position);
        strategy_position.holder_uid = strategy_uid;
        strategy_position.client_id = strategy_book->asset.client_id;
        strategy_position.ledger_category = LedgerCategory::Strategy;
        writer->close_data();
      }
      writer->write(trigger_time, position);
    }
  };
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = *pair.second;
    auto holder_uid = book.asset.holder_uid;
    if (pair.second->asset.ledger_category == LedgerCategory::Account and has_channel(holder_uid, strategy_uid)) {
      writer->write(trigger_time, book.asset);
      write_positions(book.long_positions);
      write_positions(book.short_positions);
    }
  }
  PositionEnd &end = writer->open_data<PositionEnd>(trigger_time);
  end.holder_uid = strategy_uid;
  writer->close_data();
}

void Ledger::write_asset_snapshots(int32_t msg_type) {
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (has_writer(asset.holder_uid)) {
      book->update(now());
      get_writer(asset.holder_uid)->write(now(), msg_type, asset);
    }
  }
}
} // namespace kungfu::wingchun::service
