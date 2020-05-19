//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>

#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::yijinjing::practice;
using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::strategy {
Context::Context(apprentice &app, const rx::connectable_observable<event_ptr> &events)
    : app_(app), events_(events), broker_client_(app_), bookkeeper_(app_, broker_client_) {
  log::copy_log_settings(app_.get_home(), app_.get_home()->name);
}

void Context::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);
}

int64_t Context::now() const { return app_.now(); }

void Context::add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback) {
  app_.add_timer(nanotime, callback);
}

void Context::add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback) {
  app_.add_time_interval(duration, callback);
}

void Context::add_account(const std::string &source, const std::string &account, double cash_limit) {
  uint32_t account_id = hash_str_32(account);
  if (td_locations_.find(account_id) != td_locations_.end()) {
    throw wingchun_error(fmt::format("duplicated account {}@{}", account, source));
  }

  auto home = app_.get_io_device()->get_home();
  auto account_location = location::make_shared(mode::LIVE, category::TD, source, account, home->locator);
  if (home->mode == mode::LIVE and not app_.has_location(account_location->uid)) {
    throw wingchun_error(fmt::format("invalid account {}@{}", account, source));
  }

  td_locations_.emplace(account_id, account_location);
  account_cash_limits_.emplace(account_id, cash_limit);
  account_location_ids_.emplace(account_id, account_location->uid);

  broker_client_.enroll_account(account_location);
  td_locations_.emplace(account_location->uid, account_location);
}

void Context::subscribe(const std::string &source, const std::vector<std::string> &instrument_ids,
                        const std::string &exchange_ids) {
  auto md_location = find_md_location(source);
  for (const auto &instrument_id : instrument_ids) {
    broker_client_.subscribe(md_location, exchange_ids, instrument_id);
  }
  md_locations_.emplace(md_location->uid, md_location);
}

void Context::subscribe_all(const std::string &source) { broker_client_.subscribe_all(find_md_location(source)); }

uint64_t Context::insert_order(const std::string &instrument_id, const std::string &exchange_id,
                               const std::string &account, double limit_price, int64_t volume, PriceType type,
                               Side side, Offset offset, HedgeFlag hedge_flag) {
  auto account_location_uid = lookup_account_location_id(account);
  if (not broker_client_.is_ready(account_location_uid)) {
    SPDLOG_ERROR("account {} not ready", account);
    return 0;
  }
  auto instrument_type = get_instrument_type(exchange_id, instrument_id);
  if (instrument_type == InstrumentType::Unknown || instrument_type == InstrumentType::Repo) {
    SPDLOG_ERROR("unsupported instrument type {} of {}.{}", str_from_instrument_type(instrument_type), instrument_id,
                 exchange_id);
    return 0;
  }
  auto writer = app_.get_writer(account_location_uid);
  OrderInput &input = writer->open_data<OrderInput>(app_.now());
  input.order_id = writer->current_frame_uid();
  strcpy(input.instrument_id, instrument_id.c_str());
  strcpy(input.exchange_id, exchange_id.c_str());
  strcpy(input.account_id, account.c_str());
  input.instrument_type = instrument_type;
  input.limit_price = limit_price;
  input.frozen_price = limit_price;
  input.volume = volume;
  input.price_type = type;
  input.side = side;
  input.offset = offset;
  input.hedge_flag = hedge_flag;
  writer->close_data();
  bookkeeper_.on_order_input(app_.now(), app_.get_home_uid(), account_location_uid, input);
  return input.order_id;
}

uint64_t Context::cancel_order(uint64_t order_id) {
  uint32_t account_location_uid = (order_id >> 32u) xor (app_.get_home_uid());
  if (not broker_client_.is_ready(account_location_uid)) {
    SPDLOG_ERROR("invalid order_id {:16x}", order_id);
    return 0;
  }
  auto account_location = app_.get_location(account_location_uid);
  auto writer = app_.get_writer(account_location_uid);
  OrderAction &action = writer->open_data<OrderAction>(0);

  action.order_action_id = writer->current_frame_uid();
  action.order_id = order_id;
  action.action_flag = OrderActionFlag::Cancel;

  writer->close_data();
  return action.order_action_id;
}

bool Context::is_book_held() const { return book_held_; }

bool Context::is_positions_mirrored() const { return positions_mirrored_; }

void Context::hold_book() { book_held_ = true; }

void Context::hold_positions() { positions_mirrored_ = false; }

const location_map &Context::list_md() const { return md_locations_; }

const location_map &Context::list_accounts() const { return td_locations_; }

double Context::get_account_cash_limit(const std::string &account) const {
  uint32_t account_id = yijinjing::util::hash_str_32(account);
  if (account_cash_limits_.find(account_id) == account_cash_limits_.end()) {
    throw wingchun_error(fmt::format("invalid account {}", account));
  }
  return account_cash_limits_.at(account_id);
}

int64_t Context::get_trading_day() const { return app_.get_trading_day(); }

broker::Client &Context::get_broker_client() { return broker_client_; }

book::Bookkeeper &Context::get_bookkeeper() { return bookkeeper_; }

uint32_t Context::lookup_account_location_id(const std::string &account) const {
  return account_location_ids_.at(hash_str_32(account));
}

const location_ptr &Context::find_md_location(const std::string &source) {
  if (market_data_.find(source) == market_data_.end()) {
    auto home = app_.get_home();
    auto md_location = location::make_shared(mode::LIVE, category::MD, source, source, home->locator);
    if (not app_.has_location(md_location->uid)) {
      throw wingchun_error(fmt::format("invalid md {}", source));
    }
    market_data_.emplace(source, md_location);
  }
  return market_data_.at(source);
}
} // namespace kungfu::wingchun::strategy
