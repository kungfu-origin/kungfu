//
// Created by qlu on 2019-10-14.
//

#include <kungfu/wingchun/book/book.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
Position &Book::get_long_position(const Quote &quote) { return get_position(Direction::Long, quote); }

Position &Book::get_short_position(const Quote &quote) { return get_position(Direction::Short, quote); }

void Book::update(int64_t update_time) {
  asset.update_time = update_time;
  asset.margin = 0;
  asset.market_value = 0;
  asset.unrealized_pnl = 0;
  asset.dynamic_equity = asset.avail;

  auto update_position = [&](Position &position) {
    auto is_stock =
        position.instrument_type == InstrumentType::Stock or position.instrument_type == InstrumentType::Bond or
        position.instrument_type == InstrumentType::Fund or position.instrument_type == InstrumentType::StockOption or
        position.instrument_type == InstrumentType::TechStock or position.instrument_type == InstrumentType::Index or
        position.instrument_type == InstrumentType::Repo;
    auto is_future = position.instrument_type == InstrumentType::Future;
    auto position_market_value =
        position.volume * (position.last_price > 0 ? position.last_price : position.avg_open_price);
    asset.margin += position.margin;
    asset.market_value += position_market_value;
    asset.unrealized_pnl += position.unrealized_pnl;
    if (is_stock) {
      asset.dynamic_equity += position_market_value;
    }
    if (is_future) {
      asset.dynamic_equity += position.margin + position.position_pnl;
    }
  };

  for (auto &pair : long_positions) {
    update_position(pair.second);
  }
  for (auto &pair : short_positions) {
    update_position(pair.second);
  }
}

Bookkeeper::Bookkeeper(apprentice &app, broker::Client &broker_client)
    : app_(app), broker_client_(broker_client), instruments_(), books_() {}

const BookMap &Bookkeeper::get_books() const { return books_; }

Book_ptr Bookkeeper::get_book(uint32_t location_uid) {
  if (books_.find(location_uid) == books_.end()) {
    books_.emplace(location_uid, make_book(location_uid));
  }
  return books_.at(location_uid);
}

void Bookkeeper::set_accounting_method(longfist::enums::InstrumentType instrument_type,
                                       AccountingMethod_ptr accounting_method) {
  accounting_methods_.emplace(instrument_type, accounting_method);
}

void Bookkeeper::on_trading_day(int64_t daytime) {
  auto trading_day = time::strftime(daytime, "%Y%m%d").c_str();
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
  events | is_own<Quote>(broker_client_) | $([&](const event_ptr &event) {
    const Quote &data = event->data<Quote>();
    auto accounting_method = accounting_methods_.at(data.instrument_type);
    for (const auto &item : books_) {
      auto &book = item.second;
      if (book->has_position(data)) {
        accounting_method->apply_quote(book, data);
      }
    }
  });

  events | is(OrderInput::tag) | $([&](const event_ptr &event) {
    const OrderInput &data = event->data<OrderInput>();
    auto accounting_method = accounting_methods_.at(data.instrument_type);
    accounting_method->apply_order_input(get_book(event->source()), data);
    accounting_method->apply_order_input(get_book(event->dest()), data);
  });

  events | is(Order::tag) | $([&](const event_ptr &event) {
    const Order &data = event->data<Order>();
    auto accounting_method = accounting_methods_.at(data.instrument_type);
    accounting_method->apply_order(get_book(event->source()), data);
    accounting_method->apply_order(get_book(event->dest()), data);
  });

  events | is(Trade::tag) | $([&](const event_ptr &event) {
    const Trade &data = event->data<Trade>();
    auto accounting_method = accounting_methods_.at(data.instrument_type);
    accounting_method->apply_trade(get_book(event->source()), data);
    accounting_method->apply_trade(get_book(event->dest()), data);
  });

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
}

void Bookkeeper::restore(const yijinjing::cache::bank &state_bank) {
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
  auto book = std::make_shared<Book>();
  auto &asset = book->asset;
  asset.holder_uid = location_uid;
  asset.ledger_category = location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
  strcpy(asset.trading_day, time::strftime(app_.get_trading_day(), "%Y%m%d").c_str());
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