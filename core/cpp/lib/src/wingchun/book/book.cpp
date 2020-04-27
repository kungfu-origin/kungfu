//
// Created by qlu on 2019-10-14.
//

#include <kungfu/wingchun/book/book.h>

#include <utility>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
Book::Book(const CommissionMap &commissions_ref, const InstrumentMap &instruments_ref)
    : commissions(commissions_ref), instruments(instruments_ref) {}

double Book::get_frozen_price(uint64_t order_id) {
  if (orders.find(order_id) != orders.end()) {
    return orders.at(order_id).frozen_price;
  }
  return 0;
}

void Book::ensure_position(const InstrumentKey &instrument_key) {
  if (is_shortable(instrument_key.instrument_type)) {
    get_position_for(Direction::Short, instrument_key);
  }
  get_position_for(Direction::Long, instrument_key);
}


bool Book::has_long_position(const char *exchange_id, const char *instrument_id) const {
  auto position_id = hash_instrument(exchange_id, instrument_id);
  return long_positions.find(position_id) != long_positions.end();
}

bool Book::has_short_position( const char *exchange_id, const char *instrument_id) const {
  auto position_id = hash_instrument(exchange_id, instrument_id);
  return short_positions.find(position_id) != short_positions.end();
}

bool Book::has_position(const char *exchange_id, const char *instrument_id) const {
  return has_long_position(exchange_id, instrument_id) or has_short_position(exchange_id, instrument_id);
}

Position &Book::get_long_position(const char *exchange_id, const char *instrument_id) {
  return get_position(Direction::Long, exchange_id, instrument_id);
}

Position &Book::get_short_position(const char *exchange_id, const char *instrument_id) {
  return get_position(Direction::Short, exchange_id, instrument_id);
}

Position &Book::get_position(Direction direction, const char *exchange_id, const char *instrument_id) {
  assert(asset.holder_uid != 0);
  PositionMap &positions = direction == Direction::Long ? long_positions : short_positions;
  auto position_id = hash_instrument(exchange_id, instrument_id);
  auto pair = positions.try_emplace(position_id);
  auto &position = (*pair.first).second;
  if (pair.second) {
    position.trading_day = asset.trading_day;
    position.instrument_id = instrument_id;
    position.exchange_id = exchange_id;
    position.instrument_type = get_instrument_type(position.exchange_id, position.instrument_id);
    position.holder_uid = asset.holder_uid;
    position.ledger_category = asset.ledger_category;
    position.source_id = asset.source_id;
    position.account_id = asset.account_id;
    position.client_id = asset.client_id;
    position.direction = direction;
  }
  return position;
}

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
} // namespace kungfu::wingchun::book