//
// Created by Keren Dong on 2020/4/6.
//

#ifndef WINGCHUN_BOOKKEEPER_H
#define WINGCHUN_BOOKKEEPER_H

#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::book {
// key = location_uid
typedef std::unordered_map<uint32_t, Book_ptr> BookMap;

typedef std::unordered_map<longfist::enums::InstrumentType, AccountingMethod_ptr> AccountingMethodMap;

class Bookkeeper {
public:
  explicit Bookkeeper(yijinjing::practice::apprentice &app, broker::Client &broker_client, bool bypass_quotes = false);

  virtual ~Bookkeeper() = default;

  bool has_book(uint32_t location_uid);

  Book_ptr get_book(uint32_t uid);

  void drop_book(uint32_t uid);

  [[nodiscard]] const BookMap &get_books() const;

  void set_accounting_method(longfist::enums::InstrumentType instrument_type,
                             const AccountingMethod_ptr &accounting_method);

  void on_trading_day(int64_t daytime);

  void on_start(const rx::connectable_observable<event_ptr> &events);

  void on_order_input(int64_t update_time, uint32_t source, uint32_t dest, const longfist::types::OrderInput &input);

  void restore(const yijinjing::cache::bank &state_bank);

  void guard_positions();

private:
  yijinjing::practice::apprentice &app_;
  broker::Client &broker_client_;
  const bool bypass_quotes_;

  bool positions_guarded_ = false;
  CommissionMap commissions_ = {};
  InstrumentMap instruments_ = {};
  BookMap books_ = {};
  AccountingMethodMap accounting_methods_ = {};

  Book_ptr make_book(uint32_t location_uid);

  void update_instrument(const longfist::types::Instrument &instrument);

  void update_book(const event_ptr &event, const longfist::types::InstrumentKey &instrument_key);

  void update_book(const event_ptr &event, const longfist::types::Quote &quote);

  void try_update_asset(const longfist::types::Asset &asset);

  void try_update_position(const longfist::types::Position &position);

  template <typename TradingData, typename ApplyMethod = void (AccountingMethod::*)(Book_ptr, const TradingData &)>
  void update_book(const event_ptr &event, ApplyMethod method) {
    update_book(event->gen_time(), event->source(), event->dest(), event->data<TradingData>(), method);
  }

  template <typename TradingData, typename ApplyMethod = void (AccountingMethod::*)(Book_ptr, const TradingData &)>
  void update_book(int64_t update_time, uint32_t source, uint32_t dest, const TradingData &data, ApplyMethod method) {
    if (accounting_methods_.find(data.instrument_type) == accounting_methods_.end()) {
      SPDLOG_WARN("accounting method not found for {}: {}", data.type_name.c_str(), data.to_string());
      return;
    }
    AccountingMethod &accounting_method = *accounting_methods_.at(data.instrument_type);
    auto apply_and_update = [&](uint32_t book_uid) {
      auto book = get_book(book_uid);
      auto &position = book->get_position_for(data);
      (accounting_method.*method)(book, data);
      position.update_time = update_time;
      book->update(update_time);
    };
    apply_and_update(source);
    apply_and_update(dest);
  }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_BOOKKEEPER_H
