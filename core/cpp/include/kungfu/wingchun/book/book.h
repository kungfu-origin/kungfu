//
// Created by qlu on 2019-10-14.
//

#ifndef WINGCHUN_BOOK_H
#define WINGCHUN_BOOK_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::book {
FORWARD_DECLARE_PTR(Book)
FORWARD_DECLARE_PTR(AccountingMethod)

typedef std::unordered_map<uint32_t, longfist::types::Position> PositionMap;
typedef std::unordered_map<uint32_t, longfist::types::PositionDetail> PositionDetailMap;
typedef std::unordered_map<uint32_t, Book_ptr> BookMap;
typedef std::unordered_map<uint32_t, longfist::types::Instrument> InstrumentMap;
typedef std::unordered_map<longfist::enums::InstrumentType, AccountingMethod_ptr> AccountingMethodMap;

struct Book {
  longfist::types::Asset asset = {};

  PositionMap long_positions = {};
  PositionDetailMap long_position_details = {};

  PositionMap short_positions = {};
  PositionDetailMap short_position_details = {};

  std::unordered_map<uint64_t, longfist::types::Order> orders = {};

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

class AccountingMethod {
public:
  AccountingMethod() = default;

  virtual ~AccountingMethod() = default;

  virtual void apply_trading_day(Book_ptr book, int64_t trading_day) = 0;

  virtual void apply_quote(Book_ptr book, const longfist::types::Quote &quote) = 0;

  virtual void apply_order_input(Book_ptr book, const longfist::types::OrderInput &input) = 0;

  virtual void apply_order(Book_ptr book, const longfist::types::Order &order) = 0;

  virtual void apply_trade(Book_ptr book, const longfist::types::Trade &trade) = 0;
};

class Bookkeeper {
public:
  explicit Bookkeeper(yijinjing::practice::apprentice &app, broker::Client &broker_client);

  virtual ~Bookkeeper() = default;

  [[nodiscard]] const BookMap &get_books() const;

  Book_ptr get_book(uint32_t uid);

  void set_accounting_method(longfist::enums::InstrumentType instrument_type, AccountingMethod_ptr accounting_method);

  void on_trading_day(int64_t daytime);

  void on_start(const rx::connectable_observable<event_ptr> &events);

  void restore(const yijinjing::cache::bank &state_bank);

private:
  yijinjing::practice::apprentice &app_;
  broker::Client &broker_client_;

  BookMap books_ = {};
  InstrumentMap instruments_ = {};
  AccountingMethodMap accounting_methods_ = {};

  Book_ptr make_book(uint32_t location_uid);

  void try_update_asset(uint32_t location_uid, const longfist::types::Asset &asset);

  void try_update_position(uint32_t location_uid, const longfist::types::Position &position);

  void try_subscribe_position(const longfist::types::Position &position);

  template <typename DataType>
  static constexpr auto is_own = [](const broker::Client &broker_client) {
    return rx::filter([&](const event_ptr &event) {
      const DataType &data = event->data<DataType>();
      return event->msg_type() == DataType::tag and
             broker_client.is_subscribed(event->source(), data.exchange_id, data.instrument_id);
    });
  };
};

DECLARE_PTR(Bookkeeper)
} // namespace kungfu::wingchun::book

#endif // WINGCHUN_BOOK_H
