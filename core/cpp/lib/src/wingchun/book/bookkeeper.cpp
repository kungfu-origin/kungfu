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
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::book {
Bookkeeper::Bookkeeper(apprentice &app, broker::Client &broker_client)
    : app_(app), broker_client_(broker_client), instruments_(), books_() {
  book::AccountingMethod::setup_defaults(*this);
}

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

  events | is_own<Quote>(broker_client_) | $$$(update_book(event, event->data<Quote>()));
  events | is(OrderInput::tag) | $$$(update_book<OrderInput>(event, &AccountingMethod::apply_order_input));
  events | is(Order::tag) | $$$(update_book<Order>(event, &AccountingMethod::apply_order));
  events | is(Trade::tag) | $$$(update_book<Trade>(event, &AccountingMethod::apply_trade));
  events | is(Asset::tag) | $$$(try_update_asset(event->data<Asset>()));
  events | is(Position::tag) | $$$(try_update_position(event->data<Position>()));
  events | is(PositionEnd::tag) | $$$(get_book(event->data<PositionEnd>().holder_uid)->update(event->gen_time()));
  events | is(TradingDay::tag) | $$$(on_trading_day(event->data<TradingDay>().timestamp));
}

void Bookkeeper::restore(const cache::bank &state_bank) {
  for (auto &pair : state_bank[boost::hana::type_c<Instrument>]) {
    auto &state = pair.second;
    auto &instrument = state.data;
    instruments_.emplace(hash_instrument(instrument.exchange_id, instrument.instrument_id), instrument);
  }
  for (auto &pair : state_bank[boost::hana::type_c<Commission>]) {
    auto &state = pair.second;
    auto &commission = state.data;
    commissions_.emplace(hash_str_32(commission.product_id), commission);
  }
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

void Bookkeeper::try_update_asset(const Asset &asset) {
  if (app_.has_location(asset.holder_uid)) {
    get_book(asset.holder_uid)->asset = asset;
  }
}

void Bookkeeper::try_update_position(const Position &position) {
  if (app_.has_location(position.holder_uid)) {
    auto &target_position = get_book(position.holder_uid)->get_position(position.direction, position);
    auto last_price = target_position.last_price;
    target_position = position;
    target_position.last_price = last_price;
    try_subscribe_position(target_position);
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

void Bookkeeper::update_book(const event_ptr &event, const longfist::types::Quote &quote) {
  if (accounting_methods_.find(quote.instrument_type) == accounting_methods_.end()) {
    return;
  }
  auto accounting_method = accounting_methods_.at(quote.instrument_type);
  for (const auto &item : books_) {
    auto &book = item.second;
    auto has_long_position = book->has_long_position(quote);
    auto has_short_position = book->has_short_position(quote);
    if (has_long_position or has_short_position) {
      accounting_method->apply_quote(book, quote);
      book->update(event->gen_time());
    }
    if (has_long_position) {
      book->get_long_position(quote).update_time = event->gen_time();
    }
    if (has_short_position) {
      book->get_short_position(quote).update_time = event->gen_time();
    }
  }
}
} // namespace kungfu::wingchun::book