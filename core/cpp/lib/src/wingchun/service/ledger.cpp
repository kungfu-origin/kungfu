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

std::vector<longfist::types::Position> Ledger::get_positions(const yijinjing::data::location_ptr &location) {
  std::vector<longfist::types::Position> res = {};
  for (const auto &pair : state_bank_[boost::hana::type_c<longfist::types::Position>]) {
    if (pair.second.data.holder_uid == location->uid) {
      res.push_back(pair.second.data);
    }
  }
  return res;
}

bool Ledger::has_asset(const yijinjing::data::location_ptr &location) {
  return assets_.find(location->uid) != assets_.end();
}

longfist::types::Asset Ledger::get_asset(const yijinjing::data::location_ptr &location) {
  return assets_.at(location->uid).data;
}

const std::unordered_map<uint32_t, longfist::types::Instrument> &Ledger::get_instruments() const {
  return broker_client_.get_instruments();
}

void Ledger::on_ready() {
  events_ | is(Position::tag) | $([&](const event_ptr &event) {
    const Position &position = event->data<Position>();
    auto holder_location = get_location(position.holder_uid);
    if (holder_location->category == category::TD) {
      auto group = holder_location->group;
      auto md_location = location::make_shared(holder_location->mode, category::MD, group, group, get_locator());
      broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
    }
  });
}

void Ledger::on_start() {
  events_ | is(Register::tag) | $([&](const event_ptr &event) {
    auto register_data = event->data<Register>();
    auto app_location = get_location(register_data.location_uid);
    if (app_location->category == category::STRATEGY) {
      request_read_from_public(event->gen_time(), app_location->uid, get_last_active_time());
      request_read_from(event->gen_time(), app_location->uid, get_last_active_time());
      request_write_to(event->gen_time(), app_location->uid);
    }
  });

  events_ | is(Channel::tag) | $([&](const event_ptr &event) {
    const Channel &channel = event->data<Channel>();
    auto source_location = get_location(channel.source_id);
    if (channel.source_id != get_home_uid() and channel.dest_id != get_home_uid()) {
      reader_->join(source_location, channel.dest_id, event->gen_time());
    }
    if (source_location->category == category::TD and channel.dest_id == get_home_uid() and
        has_writer(channel.source_id)) {
      auto writer = get_writer(channel.source_id);

      CleanCacheRequest request = {};
      request.msg_type = Asset::tag;
      writer->write(event->gen_time(), request);
      request.msg_type = Position::tag;
      writer->write(event->gen_time(), request);

      writer->mark(event->gen_time(), AssetRequest::tag);
      writer->mark(event->gen_time(), PositionRequest::tag);
    }
  });

  events_ | is(Quote::tag) | $([&](const event_ptr &event) { const Quote &data = event->data<Quote>(); });

  events_ | is(OrderInput::tag) | $([&](const event_ptr &event) {
    const OrderInput &data = event->data<OrderInput>();
    write_book(event, data);

    OrderStat stat = {};
    stat.order_id = data.order_id;
    stat.md_time = event->trigger_time();
    stat.insert_time = event->gen_time();
    order_stats_.emplace(stat.order_id, state<OrderStat>(get_home_uid(), event->dest(), event->gen_time(), stat));
    write_to(event->gen_time(), stat, event->dest());
  });

  events_ | is(Order::tag) | $([&](const event_ptr &event) {
    const Order &data = event->data<Order>();
    write_book(event, data);

    if (order_stats_.find(data.order_id) == order_stats_.end()) {
      order_stats_.emplace(data.order_id,
                           state<OrderStat>(get_home_uid(), event->source(), event->gen_time(), OrderStat()));
    }
    OrderStat &stat = order_stats_.at(data.order_id).data;
    stat.ack_time = event->gen_time();
    write_to(event->gen_time(), stat, event->source());
  });

  events_ | is(Trade::tag) | $([&](const event_ptr &event) {
    const Trade &data = event->data<Trade>();
    write_book(event, data);
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
      pair.second->update(event->gen_time());
      get_writer(location::PUBLIC)->write(event->gen_time(), AssetSnapshot::tag, pair.second->asset);
    }
  });

  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);
  bookkeeper_.restore(state_bank_);

  state_bank_ >> get_writer(location::PUBLIC);
}

void Ledger::write_daily_assets() {
  for (auto &pair : bookkeeper_.get_books()) {
    DailyAsset daily = {};
    memcpy(&daily, &(pair.second->asset), sizeof(daily));
    get_writer(location::PUBLIC)->write(now(), daily);
  }
}
} // namespace kungfu::wingchun::service
