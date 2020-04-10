//
// Created by Keren Dong on 2019-06-28.
//

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/service/ledger.h>
#include <kungfu/wingchun/utils.h>
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

  events_ | is(Register::tag) | $([&](const event_ptr &event) {
    auto register_data = event->data<Register>();
    auto app_location = get_location(register_data.location_uid);
    auto resume_time_point = broker_client_.get_resume_policy().get_connect_time(*this, register_data);
    if (app_location->category == category::STRATEGY) {
      request_write_to(event->gen_time(), app_location->uid);
      request_read_from(event->gen_time(), app_location->uid, resume_time_point);
      request_read_from_public(event->gen_time(), app_location->uid, resume_time_point);
    }
  });

  events_ | is(Channel::tag) | $([&](const event_ptr &event) {
    const Channel &channel = event->data<Channel>();
    auto source_location = get_location(channel.source_id);
    if (channel.source_id != get_home_uid() and channel.dest_id != get_home_uid()) {
      reader_->join(source_location, channel.dest_id, event->gen_time());
    }
    bool writable = channel.dest_id == get_home_uid() and has_writer(channel.source_id);
    if (writable and source_location->category == category::TD) {
      write_book_reset(event->gen_time(), channel.source_id);
    }
    if (writable and source_location->category == category::STRATEGY) {
      write_book_reset(event->gen_time(), channel.source_id);
      write_strategy_data(event->gen_time(), channel.source_id);
    }
  });

  events_ | is(OrderInput::tag) | $([&](const event_ptr &event) {
    const OrderInput &data = event->data<OrderInput>();
    write_book(event, data);

    auto &stat = get_order_stat(data.order_id, event);
    stat.order_id = data.order_id;
    stat.md_time = event->trigger_time();
    stat.input_time = event->gen_time();
  });

  events_ | is(Order::tag) | $([&](const event_ptr &event) {
    const Order &data = event->data<Order>();

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
  });

  events_ | is(Trade::tag) | $([&](const event_ptr &event) {
    const Trade &data = event->data<Trade>();
    write_book(event, data);

    auto &stat = get_order_stat(data.order_id, event);
    if (stat.trade_time == 0) {
      stat.trade_time = event->gen_time();
      write_to(event->gen_time(), stat, event->source());
    }
  });

  events_ | is(Position::tag) | $([&](const event_ptr &event) {
    const Position &position = event->data<Position>();
    auto holder_location = get_location(position.holder_uid);
    if (holder_location->category == category::TD) {
      auto group = holder_location->group;
      auto md_location = location::make_shared(holder_location->mode, category::MD, group, group, get_locator());
      broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
    }
  });

  events_ | is(PositionEnd::tag) | $([&](const event_ptr &event) {
    const PositionEnd &data = event->data<PositionEnd>();
    auto book = bookkeeper_.get_book(data.holder_uid);
    write_to(event->gen_time(), book->asset, data.holder_uid);
  });

  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE,
                    [&](const event_ptr &event) { write_asset_snapshots(AssetSnapshot::tag); });
}

OrderStat &Ledger::get_order_stat(uint64_t order_id, const event_ptr &event) {
  if (order_stats_.find(order_id) == order_stats_.end()) {
    order_stats_.try_emplace(order_id, get_home_uid(), event->source(), event->gen_time(), OrderStat());
  }
  return order_stats_.at(order_id).data;
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

void Ledger::write_strategy_data(int64_t trigger_time, uint32_t dest) {
  auto strategy_book = bookkeeper_.get_book(dest);
  auto writer = get_writer(dest);
  auto write_positions = [&](auto positions) {
    for (auto &pair : positions) {
      auto &account_position = pair.second;
      if (strategy_book->has_position(account_position)) {
        auto &strategy_position = writer->open_data<Position>(trigger_time);
        memcpy(&strategy_position, &account_position, sizeof(Position));
        strategy_position.holder_uid = dest;
        writer->close_data();
      }
      writer->write(trigger_time, account_position);
    }
  };
  for (auto &pair : bookkeeper_.get_books()) {
    auto &account_book = *pair.second;
    writer->write(trigger_time, account_book.asset);
    write_positions(account_book.long_positions);
    write_positions(account_book.short_positions);
  }
  PositionEnd &end = writer->open_data<PositionEnd>(trigger_time);
  end.holder_uid = dest;
  writer->close_data();
}

void Ledger::write_asset_snapshots(int32_t msg_type) {
  for (auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (has_writer(asset.holder_uid)) {
      book->update(now());
      get_writer(asset.holder_uid)->write(now(), msg_type, asset);
    }
  }
}
} // namespace kungfu::wingchun::service
