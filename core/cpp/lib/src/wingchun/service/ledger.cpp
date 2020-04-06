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
      broker_client_(*this), bookkeeper_(*this, broker_client_) {
  log::copy_log_settings(get_io_device()->get_home(), "ledger");
  if (m == mode::LIVE) {
    pub_sock_ = get_io_device()->bind_socket(nanomsg::protocol::PUBLISH);
  }
}

void Ledger::on_exit() { write_daily_assets(); }

void Ledger::on_trading_day(const event_ptr &event, int64_t daytime) {
  bookkeeper_.on_trading_day(daytime);
  write_daily_assets();
}

book::Bookkeeper &Ledger::get_bookkeeper() { return bookkeeper_; }

void Ledger::publish(const std::string &msg) {
  if (get_io_device()->get_home()->mode == mode::LIVE) {
    pub_sock_->send(msg);
  }
  SPDLOG_DEBUG("published {}", msg);
}

uint64_t Ledger::cancel_order(const event_ptr &event, uint32_t account_location_uid, uint64_t order_id) {
  if (has_writer(account_location_uid)) {
    SPDLOG_INFO("cancel order {}", order_id);
    auto writer = get_writer(account_location_uid);
    OrderAction &action = writer->open_data<OrderAction>(event->gen_time());
    action.order_action_id = writer->current_frame_uid();
    action.order_id = order_id;
    action.action_flag = OrderActionFlag::Cancel;
    writer->close_data();
    return action.order_action_id;
  }
  return 0;
}

std::vector<Position> Ledger::get_positions(const location_ptr &location) {
  std::vector<Position> res = {};
  for (const auto &pair : state_bank_[boost::hana::type_c<Position>]) {
    if (pair.second.data.holder_uid == location->uid) {
      res.push_back(pair.second.data);
    }
  }
  return res;
}

bool Ledger::has_asset(const location_ptr &location) { return assets_.find(location->uid) != assets_.end(); }

Asset Ledger::get_asset(const location_ptr &location) { return assets_.at(location->uid).data; }

const std::unordered_map<uint32_t, Instrument> &Ledger::get_instruments() const {
  return broker_client_.get_instruments();
}

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
      write_strategy_books(event->gen_time(), channel.source_id);
    }
  });

  events_ | is(OrderInput::tag) | $([&](const event_ptr &event) {
    const OrderInput &data = event->data<OrderInput>();
    write_book(event, data);

    OrderStat stat = {};
    stat.order_id = data.order_id;
    stat.md_time = event->trigger_time();
    stat.insert_time = event->gen_time();
    order_stats_.try_emplace(stat.order_id, get_home_uid(), event->dest(), event->gen_time(), stat);
    write_to(event->gen_time(), stat, event->dest());
  });

  events_ | is(Order::tag) | $([&](const event_ptr &event) {
    const Order &data = event->data<Order>();

    if (data.error_id == 0) {
      write_book(event, data);
    }

    if (order_stats_.find(data.order_id) == order_stats_.end()) {
      order_stats_.try_emplace(data.order_id, get_home_uid(), event->source(), event->gen_time(), OrderStat());
    }
    OrderStat &stat = order_stats_.at(data.order_id).data;
    stat.ack_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  });

  events_ | is(Trade::tag) | $([&](const event_ptr &event) {
    const Trade &data = event->data<Trade>();
    write_book(event, data);

    if (order_stats_.find(data.order_id) == order_stats_.end()) {
      order_stats_.try_emplace(data.order_id, get_home_uid(), event->source(), event->gen_time(), OrderStat());
    }
    OrderStat &stat = order_stats_.at(data.order_id).data;
    stat.trade_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
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

  events_ | is(InstrumentRequest::tag) | $([&](const event_ptr &event) {
    try {
      handle_instrument_request(event);
    } catch (const std::exception &e) {
      SPDLOG_ERROR("Unexpected exception {}", e.what());
    }
  });

  /**
   * process active query from clients
   */
  events_ | filter([&](const event_ptr &event) {
    return dynamic_cast<nanomsg::nanomsg_json *>(event.get()) != nullptr and event->source() == 0;
  }) | $([&](const event_ptr &event) {
    // let python do the actual job, we just operate the I/O part
    try {
      const nlohmann::json &cmd = event->data<nlohmann::json>();
      SPDLOG_INFO("handle command type {} data {}", event->msg_type(), cmd.dump());
      std::string response = handle_request(event, event->to_string());
      get_io_device()->get_rep_sock()->send(response);
    } catch (const std::exception &e) {
      SPDLOG_ERROR("Unexpected exception {}", e.what());
    }
  });

  add_time_interval(time_unit::NANOSECONDS_PER_MINUTE, [&](const event_ptr &event) {
    for (auto &pair : bookkeeper_.get_books()) {
      auto &book = pair.second;
      auto &asset = book->asset;
      if (has_writer(asset.holder_uid)) {
        book->update(event->gen_time());
        get_writer(asset.holder_uid)->write(event->gen_time(), AssetSnapshot::tag, asset);
      }
    }
  });
}

void Ledger::write_book_reset(int64_t trigger_time, uint32_t dest) {
  auto writer = get_writer(dest);

  writer->open_data<CacheReset>().msg_type = Asset::tag;
  writer->close_data();
  writer->mark(trigger_time, AssetRequest::tag);

  writer->open_data<CacheReset>().msg_type = Position::tag;
  writer->close_data();
  writer->mark(trigger_time, PositionRequest::tag);
  SPDLOG_INFO("reset book for {}", get_location_uname(dest));
}

void Ledger::write_strategy_books(int64_t trigger_time, uint32_t dest) {
  auto book = bookkeeper_.get_book(dest);
  auto writer = get_writer(dest);
  auto write_positions = [&](auto positions) {
    for (auto &pair : positions) {
      if (book->has_position(pair.second)) {
        writer->write(trigger_time, pair.second);
      }
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

void Ledger::write_daily_assets() {
  for (auto &pair : bookkeeper_.get_books()) {
    auto &book = pair.second;
    auto &asset = book->asset;
    if (has_writer(asset.holder_uid)) {
      auto writer = get_writer(asset.holder_uid);
      DailyAsset &daily = writer->open_data<DailyAsset>();
      memcpy(&daily, &asset, sizeof(daily));
      writer->close_data();
    }
  }
}
} // namespace kungfu::wingchun::service
