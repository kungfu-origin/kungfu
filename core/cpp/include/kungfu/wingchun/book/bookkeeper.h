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
  explicit Bookkeeper(yijinjing::practice::apprentice &app, broker::Client &broker_client);

  virtual ~Bookkeeper() = default;

  bool has_book(uint32_t location_uid);

  Book_ptr get_book(uint32_t uid);

  [[nodiscard]] const BookMap &get_books() const;

  void set_accounting_method(longfist::enums::InstrumentType instrument_type,
                             const AccountingMethod_ptr &accounting_method);

  void on_trading_day(int64_t daytime);

  void on_start(const rx::connectable_observable<event_ptr> &events);

  void restore(const yijinjing::cache::bank &state_bank);

private:
  yijinjing::practice::apprentice &app_;
  broker::Client &broker_client_;

  CommissionMap commissions_ = {};
  InstrumentMap instruments_ = {};
  BookMap books_ = {};
  AccountingMethodMap accounting_methods_ = {};

  Book_ptr make_book(uint32_t location_uid);

  void try_update_asset(const longfist::types::Asset &asset);

  void try_update_position(const longfist::types::Position &position);

  void update_book(const event_ptr &event, const longfist::types::InstrumentKey &instrument_key);

  void update_book(const event_ptr &event, const longfist::types::Quote &quote);

  template <typename TradingData, typename ApplyMethod = void (AccountingMethod::*)(Book_ptr, const TradingData &)>
  void update_book(const event_ptr &event, ApplyMethod method) {
    const TradingData &data = event->data<TradingData>();
    if (accounting_methods_.find(data.instrument_type) == accounting_methods_.end()) {
      SPDLOG_WARN("accounting method not found for {}: {}", data.type_name.c_str(), data.to_string());
      return;
    }
    AccountingMethod &accounting_method = *accounting_methods_.at(data.instrument_type);
    auto apply_and_update = [&](uint32_t book_uid) {
      auto book = get_book(book_uid);
      auto &position = book->get_position(data);
      (accounting_method.*method)(book, data);
      position.update_time = event->gen_time();
      book->update(event->gen_time());
    };
    apply_and_update(event->source());
    apply_and_update(event->dest());
  }

  template <typename DataType>
  static constexpr auto is_own = [](const broker::Client &broker_client) {
    return rx::filter([&](const event_ptr &event) {
      const DataType &data = event->data<DataType>();
      return event->msg_type() == DataType::tag and
             broker_client.is_subscribed(event->source(), data.exchange_id, data.instrument_id);
    });
  };
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_BOOKKEEPER_H
