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
  bookkeeper_.guard_positions();

  events_ | is(OrderInput::tag) | $$(update_order_stat(event, event->data<OrderInput>()));
  events_ | is(Order::tag) | $$(update_order_stat(event, event->data<Order>()));
  events_ | is(Trade::tag) | $$(update_order_stat(event, event->data<Trade>()));
  events_ | is(Channel::tag) | $$(inspect_channel(event->gen_time(), event->data<Channel>()));
  events_ | is(MirrorPositionsRequest::tag) | $$(mirror_positions(event->gen_time(), event->source()));
  events_ | is(PositionRequest::tag) | $$(write_strategy_data(event->gen_time(), event->source()));
  events_ | is(AssetRequest::tag) | $$(write_book_reset(event->gen_time(), event->source()));
  events_ | is(PositionEnd::tag) | filter([&](const event_ptr &event) {
    SPDLOG_WARN(" Ledger::on_start Position event->source() : {}, event->dest() : {}",
                get_location_uname(event->source()), get_location_uname(event->dest()));
    return event->dest() != location::UPDATE;
  }) | $$(update_account_book(event->gen_time(), event->data<PositionEnd>().holder_uid););

  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE, [&](auto e) { write_asset_snapshots(AssetSnapshot::tag); });
  add_time_interval(time_unit::NANOSECONDS_PER_HOUR, [&](auto e) { write_asset_snapshots(DailyAsset::tag); });

  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE, [&](auto e) { write_asset_update(e); });
  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE, [&](auto e) { write_position_update(e); });

  refresh_books();
}

void Ledger::refresh_books() {
  for (const auto &pair : bookkeeper_.get_books()) {
    if (pair.second->asset.ledger_category == LedgerCategory::Account) {
      refresh_account_book(now(), pair.first);
    }
    request_write_to(now(), pair.first);
  }
}

void Ledger::refresh_account_book(int64_t trigger_time, uint32_t account_uid) {
  auto account_location = get_location(account_uid);
  auto group = account_location->group;
  auto md_location = location::make_shared(account_location->mode, category::MD, group, group, get_locator());
  auto book = bookkeeper_.get_book(account_uid);
  auto subscribe_positions = [&](auto positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
    }
  };
  subscribe_positions(book->long_positions);
  subscribe_positions(book->short_positions);
  broker_client_.try_renew(trigger_time, md_location);
  book->update(trigger_time);
}

OrderStat &Ledger::get_order_stat(uint64_t order_id, const event_ptr &event) {
  if (order_stats_.find(order_id) == order_stats_.end()) {
    order_stats_.try_emplace(order_id, get_home_uid(), event->source(), event->gen_time(), OrderStat());
  }
  return order_stats_.at(order_id).data;
}

void Ledger::update_order_stat(const event_ptr &event, const OrderInput &data) {
  write_book(event->gen_time(), event->dest(), event->source(), data);
  auto &stat = get_order_stat(data.order_id, event);
  stat.order_id = data.order_id;
  stat.md_time = event->trigger_time();
  stat.input_time = event->gen_time();
}

void Ledger::update_order_stat(const event_ptr &event, const Order &data) {
  if (data.error_id == 0) {
    write_book(event->gen_time(), event->source(), event->dest(), data);
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

void Ledger::update_order_stat(const event_ptr &event, const Trade &data) {
  write_book(event->gen_time(), event->source(), event->dest(), data);
  auto &stat = get_order_stat(data.order_id, event);
  if (stat.trade_time == 0) {
    stat.trade_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  }
}

void Ledger::update_account_book(int64_t trigger_time, uint32_t account_uid) {
  refresh_account_book(trigger_time, account_uid);
  auto writer = get_writer(account_uid);
  auto book = bookkeeper_.get_book(account_uid);
  auto &asset = book->asset;
  write_positions(trigger_time, account_uid, book->long_positions);
  write_positions(trigger_time, account_uid, book->short_positions);
  writer->write(trigger_time, asset);
  write_asset_snapshot(trigger_time, writer, asset);
}

void Ledger::inspect_channel(int64_t trigger_time, const Channel &channel) {
  auto source_location = get_location(channel.source_id);
  auto is_from_account = source_location->category == category::TD;

  if (channel.source_id == cached_home_location_->uid or channel.dest_id == cached_home_location_->uid) {
    return;
  }
  if (channel.source_id != get_live_home_uid() and channel.dest_id != get_live_home_uid()) {
    reader_->join(source_location, channel.dest_id, trigger_time);
  }
  if (channel.dest_id == get_live_home_uid() and has_writer(channel.source_id) and is_from_account) {
    write_book_reset(trigger_time, channel.source_id);
  }
  if (channel.source_id == get_live_home_uid() and bookkeeper_.has_book(channel.dest_id)) {
    write_asset_snapshot(trigger_time, get_writer(channel.dest_id), bookkeeper_.get_book(channel.dest_id)->asset);
  }
}

void Ledger::mirror_positions(int64_t trigger_time, uint32_t strategy_uid) {
  auto strategy_book = bookkeeper_.get_book(strategy_uid);

  auto reset_positions = [trigger_time](auto &positions) {
    for (auto &item : positions) {
      auto &position = item.second;
      position.volume = 0;
      position.yesterday_volume = 0;
      position.frozen_total = 0;
      position.frozen_yesterday = 0;
      position.avg_open_price = 0;
      position.position_cost_price = 0;
      position.update_time = trigger_time;
    }
  };
  reset_positions(strategy_book->long_positions);
  reset_positions(strategy_book->short_positions);

  auto copy_positions = [&](const auto &positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      if (strategy_book->has_position_for(position)) {
        auto &strategy_position = strategy_book->get_position_for(position.direction, position);

        auto volume = strategy_position.volume;
        auto yesterday_volume = strategy_position.yesterday_volume;
        auto frozen_total = strategy_position.frozen_total;
        auto frozen_yesterday = strategy_position.frozen_yesterday;
        auto avg_open_price = strategy_position.avg_open_price;
        auto position_cost_price = strategy_position.position_cost_price;

        auto total_volume = strategy_position.volume + position.volume;

        longfist::copy(strategy_position, position);
        strategy_position.holder_uid = strategy_uid;
        strategy_position.client_id = strategy_book->asset.client_id;
        strategy_position.ledger_category = LedgerCategory::Strategy;
        strategy_position.update_time = trigger_time;

        if (volume > 0) {
          strategy_position.volume += volume;
          strategy_position.yesterday_volume += yesterday_volume;
          strategy_position.frozen_total += frozen_total;
          strategy_position.frozen_yesterday += frozen_yesterday;
          strategy_position.avg_open_price =
              ((position.avg_open_price * position.volume) + (avg_open_price * volume)) / total_volume;
          strategy_position.position_cost_price =
              ((position.position_cost_price * position.volume) + (position_cost_price * volume)) / total_volume;
        }
      }
    }
  };
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto holder_uid = book->asset.holder_uid;
    if (book->asset.ledger_category == LedgerCategory::Account and has_channel(strategy_uid, holder_uid)) {
      copy_positions(book->long_positions);
      copy_positions(book->short_positions);
    }
  }
  strategy_book->update(trigger_time);
}

void Ledger::write_book_reset(int64_t trigger_time, uint32_t book_uid) {
  auto writer = get_writer(book_uid);
  writer->open_data<CacheReset>(trigger_time).msg_type = Position::tag;
  writer->close_data();
  writer->open_data<CacheReset>(trigger_time).msg_type = Asset::tag;
  writer->close_data();
  writer->mark(trigger_time, ResetBookRequest::tag);
}

void Ledger::write_strategy_data(int64_t trigger_time, uint32_t strategy_uid) {
  auto strategy_book = bookkeeper_.get_book(strategy_uid);
  auto writer = get_writer(strategy_uid);
  writer->open_data<CacheReset>(trigger_time).msg_type = Position::tag;
  writer->close_data();
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    auto book_uid = asset.holder_uid;
    bool has_account = asset.ledger_category == LedgerCategory::Account and has_channel(book_uid, strategy_uid);
    bool is_strategy = asset.ledger_category == LedgerCategory::Strategy and book_uid == strategy_uid;
    if (has_account or is_strategy) {
      write_positions(trigger_time, strategy_uid, book->long_positions);
      write_positions(trigger_time, strategy_uid, book->short_positions);
      writer->write(trigger_time, asset);
    }
  }
  writer->open_data<PositionEnd>(trigger_time).holder_uid = strategy_uid;
  writer->close_data();
  write_asset_snapshot(trigger_time, writer, strategy_book->asset);
}

void Ledger::write_positions(int64_t trigger_time, uint32_t dest, book::PositionMap &positions) {
  auto writer = get_writer(dest);
  for (const auto &pair : positions) {
    if (pair.second.volume > 0 or pair.second.direction == Direction::Long) {
      writer->write_as(trigger_time, pair.second, get_home_uid(), pair.second.holder_uid);
    }
  }
}

void Ledger::write_asset_snapshots(int32_t msg_type) {
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (has_writer(asset.holder_uid)) {
      book->update(now());
      write_asset_snapshot(now(), get_writer(asset.holder_uid), asset);
    }
  }
}

void Ledger::write_asset_update(const event_ptr &event) {
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (asset.ledger_category == LedgerCategory::Account and has_writer(asset.holder_uid)) {
      get_writer(asset.holder_uid)->mark(event->gen_time(), AssetUpdate::tag);
    }
  }
}

void Ledger::write_position_update(const event_ptr &event) {
  for (const auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (asset.ledger_category == LedgerCategory::Account and has_writer(asset.holder_uid)) {
      get_writer(asset.holder_uid)->mark(event->gen_time(), PositionUpdate::tag);
    }
  }
}

} // namespace kungfu::wingchun::service
