//
// Created by Keren Dong on 2020/4/6.
//

#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
Bookkeeper::Bookkeeper(apprentice &app, broker::Client &broker_client)
    : app_(app), broker_client_(broker_client), instruments_(), books_() {
  book::AccountingMethod::setup_defaults(*this);
}

const CommissionMap &Bookkeeper::get_commissions() const { return commissions_; }

const InstrumentMap &Bookkeeper::get_instruments() const { return instruments_; }

const BookMap &Bookkeeper::get_books() const { return books_; }

Book_ptr Bookkeeper::get_book(uint32_t location_uid) {
  if (books_.find(location_uid) == books_.end()) {
    books_.emplace(location_uid, make_book(location_uid));
  }
  return books_.at(location_uid);
}

void Bookkeeper::set_accounting_method(InstrumentType instrument_type, const AccountingMethod_ptr &accounting_method) {
  accounting_methods_.emplace(instrument_type, accounting_method);
}

void Bookkeeper::on_trading_day(int64_t daytime) {
  auto trading_day = time::strftime(daytime, KUNGFU_TRADING_DAY_FORMAT).c_str();
  for (auto &book_pair : books_) {
    auto &book = book_pair.second;
    strcpy(book->asset.trading_day, trading_day);
    for (auto &pos_pair : book->long_positions) {
      pos_pair.second.trading_day = book->asset.trading_day;
    }
    for (auto &pos_pair : book->short_positions) {
      pos_pair.second.trading_day = book->asset.trading_day;
    }
  }
}

void Bookkeeper::on_start(const rx::connectable_observable<event_ptr> &events) {
  restore(app_.get_state_bank());
  on_trading_day(app_.get_trading_day());

  events | is_own<Quote>(broker_client_) | $([&](const event_ptr &event) {
    const Quote &data = event->data<Quote>();
    if (accounting_methods_.find(data.instrument_type) == accounting_methods_.end()) {
      return;
    }
    auto accounting_method = accounting_methods_.at(data.instrument_type);
    for (const auto &item : books_) {
      auto &book = item.second;
      auto has_long_position = book->has_long_position(data);
      auto has_short_position = book->has_short_position(data);
      if (has_long_position or has_short_position) {
        accounting_method->apply_quote(book, data);
        book->update(event->gen_time());
      }
      if (has_long_position) {
        book->get_long_position(data).update_time = event->gen_time();
      }
      if (has_short_position) {
        book->get_short_position(data).update_time = event->gen_time();
      }
    }
  });

  events | is(OrderInput::tag) |
      $([&](const event_ptr &event) { update_book<OrderInput>(event, &AccountingMethod::apply_order_input); });

  events | is(Order::tag) |
      $([&](const event_ptr &event) { update_book<Order>(event, &AccountingMethod::apply_order); });

  events | is(Trade::tag) |
      $([&](const event_ptr &event) { update_book<Trade>(event, &AccountingMethod::apply_trade); });

  events | is(Asset::tag) | $([&](const event_ptr &event) {
    const Asset &data = event->data<Asset>();
    try_update_asset(event->source(), data);
    try_update_asset(event->dest(), data);
  });

  events | is(Position::tag) | $([&](const event_ptr &event) {
    const Position &data = event->data<Position>();
    try_update_position(event->source(), data);
    try_update_position(event->dest(), data);
    try_subscribe_position(data);
  });

  events | is(PositionEnd::tag) | $([&](const event_ptr &event) {
    const PositionEnd &data = event->data<PositionEnd>();
    get_book(data.holder_uid)->update(event->gen_time());
  });

  events | is(Instrument::tag) | $([&](const event_ptr &event) {
    const Instrument &data = event->data<Instrument>();
    instruments_.emplace(hash_instrument(data.exchange_id, data.instrument_id), data);
  });

  events | is(TradingDay::tag) |
      $([&](const event_ptr &event) { on_trading_day(event->data<TradingDay>().timestamp); });
}

void Bookkeeper::restore(const cache::bank &state_bank) {
  for (auto &pair : state_bank[boost::hana::type_c<Position>]) {
    auto &state = pair.second;
    auto &position = state.data;
    auto book = get_book(position.holder_uid);
    auto is_long = position.direction == longfist::enums::Direction::Long;
    auto &positions = is_long ? book->long_positions : book->short_positions;
    positions[hash_instrument(position.exchange_id, position.instrument_id)] = position;
    try_subscribe_position(position);
  }
  for (auto &pair : state_bank[boost::hana::type_c<Asset>]) {
    auto &state = pair.second;
    auto &asset = state.data;
    auto book = get_book(asset.holder_uid);
    book->asset = asset;
    book->update(app_.now());
  }
  for (auto &pair : state_bank[boost::hana::type_c<Commission>]) {
    auto &state = pair.second;
    auto &commission = state.data;
    commissions_.emplace(yijinjing::util::hash_str_32(commission.product_id), commission);
  }
}

Book_ptr Bookkeeper::make_book(uint32_t location_uid) {
  auto location = app_.get_location(location_uid);
  auto book = std::make_shared<Book>(commissions_, instruments_);
  auto &asset = book->asset;
  asset.holder_uid = location_uid;
  asset.ledger_category = location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
  strcpy(asset.trading_day, time::strftime(app_.get_trading_day(), KUNGFU_TRADING_DAY_FORMAT).c_str());
  if (location->category == category::TD) {
    strcpy(asset.source_id, location->group.c_str());
    strcpy(asset.broker_id, location->group.c_str());
    strcpy(asset.account_id, location->name.c_str());
  }
  if (location->category == category::STRATEGY) {
    strcpy(asset.client_id, location->name.c_str());
  }
  return book;
}

void Bookkeeper::try_update_asset(uint32_t location_uid, const Asset &asset) {
  if (app_.has_location(location_uid) and app_.get_location(location_uid)->category == category::TD) {
    get_book(location_uid)->asset = asset;
  }
}

void Bookkeeper::try_update_position(uint32_t location_uid, const Position &position) {
  if (app_.has_location(location_uid) and app_.get_location(location_uid)->category == category::TD) {
    auto &target_position = get_book(location_uid)->get_position(position.direction, position);
    auto last_price = target_position.last_price;
    target_position = position;
    target_position.last_price = last_price;
  }
}

void Bookkeeper::try_subscribe_position(const Position &position) {
  auto holder_location = app_.get_location(position.holder_uid);
  if (holder_location->category == category::TD) {
    auto group = holder_location->group;
    auto md_location = location::make_shared(holder_location->mode, category::MD, group, group, app_.get_locator());
    broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
  }
}
} // namespace kungfu::wingchun::book