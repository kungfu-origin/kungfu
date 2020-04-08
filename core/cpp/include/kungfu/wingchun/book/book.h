//
// Created by qlu on 2019-10-14.
//

#ifndef WINGCHUN_BOOK_H
#define WINGCHUN_BOOK_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/common.h>

namespace kungfu::wingchun::book {
FORWARD_DECLARE_PTR(Book)
FORWARD_DECLARE_PTR(Bookkeeper)

// key = hash_str_32(product_id)
typedef std::unordered_map<uint32_t, longfist::types::Commission> CommissionMap;

// key = hash_instrument(exchange_id, instrument_id)
typedef std::unordered_map<uint32_t, longfist::types::Instrument> InstrumentMap;

// key = hash_instrument(exchange_id, instrument_id)
typedef std::unordered_map<uint32_t, longfist::types::Position> PositionMap;

// key = hash_instrument(exchange_id, instrument_id)
typedef std::unordered_map<uint32_t, longfist::types::PositionDetail> PositionDetailMap;

// key = order_id
typedef std::unordered_map<uint64_t, kungfu::longfist::types::Order> OrderMap;

struct Book {
  const CommissionMap &commissions;
  const InstrumentMap &instruments;

  longfist::types::Asset asset = {};

  PositionMap long_positions = {};
  PositionDetailMap long_position_details = {};

  PositionMap short_positions = {};
  PositionDetailMap short_position_details = {};

  OrderMap orders = {};

  Book(const CommissionMap &commissions_ref, const InstrumentMap &instruments_ref);

  double get_frozen_price(uint64_t order_id);

  template <typename DataType> bool has_position(const DataType &data) {
    return has_long_position(data) or has_short_position(data);
  }

  template <typename DataType> bool has_long_position(const DataType &data) {
    auto position_id = hash_instrument(data.exchange_id, data.instrument_id);
    return long_positions.find(position_id) != long_positions.end();
  }

  template <typename DataType> bool has_short_position(const DataType &data) {
    auto position_id = hash_instrument(data.exchange_id, data.instrument_id);
    return short_positions.find(position_id) != short_positions.end();
  }

  longfist::types::Position &get_long_position(const longfist::types::Quote &quote);

  longfist::types::Position &get_short_position(const longfist::types::Quote &quote);

  template <typename DataType>
  longfist::types::Position &get_position(longfist::enums::Direction direction, const DataType &data) {
    assert(asset.holder_uid != 0);
    PositionMap &positions = direction == longfist::enums::Direction::Long ? long_positions : short_positions;
    auto position_id = hash_instrument(data.exchange_id, data.instrument_id);
    auto pair = positions.try_emplace(position_id);
    auto &position = (*pair.first).second;
    if (pair.second) {
      position.trading_day = asset.trading_day;
      position.instrument_id = data.instrument_id;
      position.exchange_id = data.exchange_id;
      position.instrument_type = get_instrument_type(position.exchange_id, position.instrument_id);
      position.holder_uid = asset.holder_uid;
      position.ledger_category = asset.ledger_category;
      position.direction = direction;
    }
    return position;
  }

  template <typename DataType> longfist::types::Position &get_position(const DataType &data) {
    return get_position(get_direction(data.instrument_type, data.side, data.offset), data);
  }

  void update(int64_t update_time);
};
} // namespace kungfu::wingchun::book

#endif // WINGCHUN_BOOK_H
