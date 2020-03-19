//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>

#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/utils.h>
#include <kungfu/yijinjing/log/setup.h>
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
    : app_(app), events_(events), started_(false), broker_client_(app_), bookkeeper_(app_, broker_client_),
      algo_context_(std::make_shared<algo::AlgoContext>(app_, events_)),
      quotes_(app.get_state_map()[boost::hana::type_c<longfist::types::Quote>]),
      ledger_location_(location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", app_.get_locator())) {
  log::copy_log_settings(app_.get_home(), app_.get_home()->name);
}

void Context::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);
  bookkeeper_.restore(app_.get_state_map());
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
  if (accounts_.find(account_id) != accounts_.end()) {
    throw wingchun_error(fmt::format("duplicated account {}@{}", account, source));
  }

  auto home = app_.get_io_device()->get_home();
  auto account_location = location::make_shared(mode::LIVE, category::TD, source, account, home->locator);
  if (home->mode == mode::LIVE and not app_.has_location(account_location->uid)) {
    throw wingchun_error(fmt::format("invalid account {}@{}", account, source));
  }

  accounts_.emplace(account_id, account_location);
  account_cash_limits_.emplace(account_id, cash_limit);
  account_location_ids_.emplace(account_id, account_location->uid);

  broker_client_.enroll_account(account_location);

  SPDLOG_INFO("added account {}@{} [{:08x}]", account, source, account_id);
}

std::vector<yijinjing::data::location_ptr> Context::list_accounts() {
  std::vector<yijinjing::data::location_ptr> acc_locations;
  for (auto &item : accounts_) {
    acc_locations.push_back(item.second);
  }
  return acc_locations;
}

double Context::get_account_cash_limit(const std::string &account) {
  uint32_t account_id = yijinjing::util::hash_str_32(account);
  if (account_cash_limits_.find(account_id) == account_cash_limits_.end()) {
    throw wingchun_error(fmt::format("invalid account {}", account));
  }
  return account_cash_limits_[account_id];
}

const location_ptr &Context::find_marketdata(const std::string &source) {
  if (market_data_.find(source) == market_data_.end()) {
    auto home = app_.get_home();
    auto md_location = source == "bar"
                           ? location::make_shared(mode::LIVE, category::SYSTEM, "service", source, home->locator)
                           : location::make_shared(mode::LIVE, category::MD, source, source, home->locator);
    if (not app_.has_location(md_location->uid)) {
      throw wingchun_error(fmt::format("invalid md {}", source));
    }
    market_data_.emplace(source, md_location);
    SPDLOG_INFO("added md {} at {} [{:08x}]", source, md_location->uname, md_location->uid);
  }
  return market_data_.at(source);
}

void Context::subscribe_all(const std::string &source) { broker_client_.subscribe_all(find_marketdata(source)); }

void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols,
                        const std::string &exchange) {
  auto md_location = find_marketdata(source);
  for (const auto &symbol : symbols) {
    broker_client_.subscribe(md_location, exchange, symbol);
  }
}

uint64_t Context::insert_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                               double limit_price, int64_t volume, PriceType type, Side side, Offset offset,
                               HedgeFlag hedge_flag) {
  auto account_location_uid = lookup_account_location_id(account);
  if (not broker_client_.is_ready(account_location_uid)) {
    SPDLOG_ERROR("account {} not ready", account);
    return 0;
  }
  auto instrument_type = get_instrument_type(symbol, exchange);
  if (instrument_type == InstrumentType::Unknown || instrument_type == InstrumentType::Repo) {
    SPDLOG_ERROR("unsupported instrument type {} of {}.{}", str_from_instrument_type(instrument_type), symbol,
                 exchange);
    return 0;
  }
  auto writer = app_.get_writer(account_location_uid);
  OrderInput &input = writer->open_data<OrderInput>(app_.real_now());
  input.order_id = writer->current_frame_uid();
  strcpy(input.instrument_id, symbol.c_str());
  strcpy(input.exchange_id, exchange.c_str());
  strcpy(input.account_id, account.c_str());
  input.instrument_type = instrument_type;
  input.limit_price = limit_price;
  input.frozen_price = limit_price;
  input.volume = volume;
  input.price_type = type;
  input.side = side;
  input.offset = offset;
  writer->close_data();
  return input.order_id;
}

uint64_t Context::cancel_order(uint64_t order_id) {
  uint32_t account_location_uid = (order_id >> 32) xor (app_.get_home_uid());
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

uint32_t Context::lookup_account_location_id(const std::string &account) {
  return account_location_ids_.at(hash_str_32(account));
}

void Context::connect_account(const longfist::types::Register &register_data) {
  auto app_location = app_.get_location(register_data.location_uid);
  auto key = hash_str_32(app_location->name);
  SPDLOG_WARN("app_location {} key {:08x}, {}", app_location->uname, key, accounts_.find(key) != accounts_.end());
  if (app_location->category == category::TD and accounts_.find(key) != accounts_.end()) {
    SPDLOG_INFO("connecting account {}", app_location->uname);
    app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
    app_.request_read_from(app_.now(), app_location->uid, register_data.checkin_time);
    app_.request_write_to(app_.now(), app_location->uid);
  }
}
} // namespace kungfu::wingchun::strategy
