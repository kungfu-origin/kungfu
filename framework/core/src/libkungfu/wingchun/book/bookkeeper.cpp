//
// Created by Keren Dong on 2020/4/6.
//

#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::broker;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::book {
Bookkeeper::Bookkeeper(apprentice &app, broker::Client &broker_client) : app_(app), broker_client_(broker_client) {
  book::AccountingMethod::setup_defaults(*this);
}

bool Bookkeeper::has_book(uint32_t location_uid) { return books_.find(location_uid) != books_.end(); }

void Bookkeeper::drop_book(uint32_t uid) { books_.erase(uid); }

Book_ptr Bookkeeper::get_book(uint32_t location_uid) {
  if (books_.find(location_uid) == books_.end()) {
    books_.emplace(location_uid, make_book(location_uid));
  }
  return books_.at(location_uid);
}

const BookMap &Bookkeeper::get_books() const { return books_; }

void Bookkeeper::set_accounting_method(InstrumentType instrument_type, const AccountingMethod_ptr &accounting_method) {
  accounting_methods_.emplace(instrument_type, accounting_method);
}

void Bookkeeper::on_trading_day(int64_t daytime) {
  auto trading_day = time::strftime(daytime, KUNGFU_TRADING_DAY_FORMAT);
  for (auto &book_pair : books_) {
    auto &book = book_pair.second;
    strcpy(book->asset.trading_day, trading_day.c_str());
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

  events | is(Instrument::tag) | $$(update_instrument(event->data<Instrument>()));
  events | is_own<Quote>(broker_client_) | $$(update_book(event, event->data<Quote>()));
  events | is(InstrumentKey::tag) | $$(update_book(event, event->data<InstrumentKey>()));
  events | is(OrderInput::tag) | $$(update_book<OrderInput>(event, &AccountingMethod::apply_order_input));
  events | is(Order::tag) | $$(update_book<Order>(event, &AccountingMethod::apply_order));
  events | is(Trade::tag) | $$(update_book<Trade>(event, &AccountingMethod::apply_trade));
  events | is(Asset::tag) | $$(try_update_asset(event->data<Asset>()));
  events | is(Position::tag) | $$(try_update_position(event->data<Position>()));
  events | is(PositionEnd::tag) | $$(get_book(event->data<PositionEnd>().holder_uid)->update(event->gen_time()));
  events | is(TradingDay::tag) | $$(on_trading_day(event->data<TradingDay>().timestamp));
  events | is(ResetBookRequest::tag) | $$(drop_book(event->source()));
}

void Bookkeeper::on_order_input(int64_t update_time, uint32_t source, uint32_t dest, const OrderInput &input) {
  update_book<OrderInput>(update_time, source, dest, input, &AccountingMethod::apply_order_input);
}

void Bookkeeper::restore(const cache::bank &state_bank) {
  for (auto &pair : state_bank[boost::hana::type_c<Instrument>]) {
    update_instrument(pair.second.data);
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
  }
  for (auto &pair : state_bank[boost::hana::type_c<Asset>]) {
    auto &state = pair.second;
    auto &asset = state.data;
    auto book = get_book(asset.holder_uid);
    book->asset = asset;
    book->update(app_.now());
  }
}

void Bookkeeper::guard_positions() { positions_guarded_ = true; }

Book_ptr Bookkeeper::make_book(uint32_t location_uid) {
  auto location = app_.get_location(location_uid);
  auto book = std::make_shared<Book>(commissions_, instruments_);
  auto &asset = book->asset;
  asset.holder_uid = location_uid;
  asset.ledger_category = location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
  strcpy(asset.trading_day, time::strftime(app_.get_trading_day(), KUNGFU_TRADING_DAY_FORMAT).c_str());
  if (asset.ledger_category == LedgerCategory::Account) {
    strcpy(asset.source_id, location->group.c_str());
    strcpy(asset.broker_id, location->group.c_str());
    strcpy(asset.account_id, location->name.c_str());
  }
  if (asset.ledger_category == LedgerCategory::Strategy) {
    strcpy(asset.client_id, location->name.c_str());
  }
  return book;
}

void Bookkeeper::update_instrument(const longfist::types::Instrument &instrument) {
  instruments_.emplace(hash_instrument(instrument.exchange_id, instrument.instrument_id), instrument);
}

void Bookkeeper::update_book(const event_ptr &event, const InstrumentKey &instrument_key) {
  broker_client_.subscribe(instrument_key);
  get_book(event->source())->ensure_position(instrument_key);
}

void Bookkeeper::update_book(const event_ptr &event, const Quote &quote) {
  if (accounting_methods_.find(quote.instrument_type) == accounting_methods_.end()) {
    return;
  }
  auto accounting_method = accounting_methods_.at(quote.instrument_type);
  for (auto &item : books_) {
    auto &book = item.second;
    auto has_long_position = book->has_long_position_for(quote);
    auto has_short_position = book->has_short_position_for(quote);
    if (has_long_position or has_short_position) {
      accounting_method->apply_quote(book, quote);
      book->update(event->gen_time());
    }
    if (has_long_position) {
      book->get_position_for(Direction::Long, quote).update_time = event->gen_time();
    }
    if (has_short_position) {
      book->get_position_for(Direction::Short, quote).update_time = event->gen_time();
    }
  }
}

void Bookkeeper::update_book(const Quote &quote) {
  if (accounting_methods_.find(quote.instrument_type) == accounting_methods_.end()) {
    return;
  }
  auto accounting_method = accounting_methods_.at(quote.instrument_type);
  for (auto &item : books_) {
    auto &book = item.second;
    auto has_long_position = book->has_long_position_for(quote);
    auto has_short_position = book->has_short_position_for(quote);

    if (has_long_position or has_short_position) {
      accounting_method->apply_quote(book, quote);
      book->update(app_.now());
    }
    if (has_long_position) {
      book->get_position_for(Direction::Long, quote).update_time = app_.now();
    }
    if (has_short_position) {
      book->get_position_for(Direction::Short, quote).update_time = app_.now();
    }
  }
}

void Bookkeeper::try_update_asset(const Asset &asset) {
  if (app_.has_location(asset.holder_uid)) {
    get_book(asset.holder_uid)->asset = asset;
  }
}

void Bookkeeper::try_update_position(const Position &position) {
  if (not app_.has_location(position.holder_uid)) {
    return;
  }
  auto book = get_book(position.holder_uid);
  auto &target_position = book->get_position_for(position.direction, position);
  if (positions_guarded_ and target_position.update_time >= position.update_time) {
    return;
  }
  auto last_price = target_position.last_price;
  target_position = position;
  target_position.last_price = std::max(last_price, target_position.last_price);
  if (accounting_methods_.find(target_position.instrument_type) == accounting_methods_.end()) {
    return;
  }
  accounting_methods_.at(target_position.instrument_type)->update_position(book, target_position);
}
} // namespace kungfu::wingchun::book