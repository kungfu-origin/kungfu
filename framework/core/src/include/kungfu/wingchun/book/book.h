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
typedef std::unordered_map<uint64_t, longfist::types::OrderInput> OrderInputMap;

// key = order_id
typedef std::unordered_map<uint64_t, longfist::types::Order> OrderMap;

// key = trade_id
typedef std::unordered_map<uint64_t, longfist::types::Trade> TradeMap;

struct Book {
  const CommissionMap &commissions;
  const InstrumentMap &instruments;
  longfist::types::Asset asset = {};
  PositionMap long_positions = {};
  PositionMap short_positions = {};
  OrderInputMap order_inputs = {};
  OrderMap orders = {};
  TradeMap trades = {};

  Book(const CommissionMap &commissions_ref, const InstrumentMap &instruments_ref);

  double get_frozen_price(uint64_t order_id);

  void ensure_position(const longfist::types::InstrumentKey &instrument_key);

  [[nodiscard]] bool has_long_position(const char *exchange_id, const char *instrument_id) const;

  [[nodiscard]] bool has_long_position_volume(const char *exchange_id, const char *instrument_id) const;

  [[nodiscard]] bool has_short_position(const char *exchange_id, const char *instrument_id) const;

  [[nodiscard]] bool has_short_position_volume(const char *exchange_id, const char *instrument_id) const;

  [[nodiscard]] bool has_position(const char *exchange_id, const char *instrument_id) const;

  [[nodiscard]] longfist::types::Position &get_long_position(const char *exchange_id, const char *instrument_id);

  [[nodiscard]] longfist::types::Position &get_short_position(const char *exchange_id, const char *instrument_id);

  [[nodiscard]] longfist::types::Position &get_position(longfist::enums::Direction direction, const char *exchange_id,
                                                        const char *instrument_id);

  template <typename TradingData> [[nodiscard]] bool has_position_for(const TradingData &data) const {
    return has_position(data.exchange_id, data.instrument_id);
  }

  template <typename TradingData> [[nodiscard]] bool has_long_position_for(const TradingData &data) const {
    return has_long_position_volume(data.exchange_id, data.instrument_id);
  }

  template <typename TradingData> [[nodiscard]] bool has_short_position_for(const TradingData &data) const {
    return has_short_position_volume(data.exchange_id, data.instrument_id);
  }

  template <typename TradingData>
  [[nodiscard]] longfist::types::Position &get_position_for(longfist::enums::Direction direction,
                                                            const TradingData &data) {
    return get_position(direction, data.exchange_id, data.instrument_id);
  }

  template <typename TradingData> [[nodiscard]] longfist::types::Position &get_position_for(const TradingData &data) {
    auto direction = get_direction(data.instrument_type, data.side, data.offset);
    return get_position(direction, data.exchange_id, data.instrument_id);
  }

  void update(int64_t update_time);

  void replace(const longfist::types::OrderInput &input);

  void replace(const longfist::types::Order &order);

  void replace(const longfist::types::Trade &trade);

  const InstrumentMap &get_instruments() const { return instruments; }

  const CommissionMap &get_commissions() const { return commissions; }
};
} // namespace kungfu::wingchun::book

#endif // WINGCHUN_BOOK_H
