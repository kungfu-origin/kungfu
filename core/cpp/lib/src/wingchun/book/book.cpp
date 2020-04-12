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
    get_position(Direction::Short, instrument_key);
  }
  get_position(Direction::Long, instrument_key);
}

void Book::update(int64_t update_time) {
  asset.update_time = update_time;
  asset.margin = 0;
  asset.market_value = 0;
  asset.unrealized_pnl = 0;
  asset.dynamic_equity = asset.avail;

  auto update_position = [&](Position &position) {
    position.update_time = update_time;
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