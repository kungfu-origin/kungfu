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
  CommissionMap commissions_ = {};
  InstrumentMap instruments_ = {};
  AccountingMethodMap accounting_methods_ = {};

  Book_ptr make_book(uint32_t location_uid);

  void try_update_asset(uint32_t location_uid, const longfist::types::Asset &asset);

  void try_update_position(uint32_t location_uid, const longfist::types::Position &position);

  void try_subscribe_position(const longfist::types::Position &position);

  template <typename DataType, typename ApplyMethod = void (AccountingMethod::*)(Book_ptr, const DataType &)>
  void update_book(const event_ptr &event, ApplyMethod apply) {
    const DataType &data = event->data<DataType>();
    if (accounting_methods_.find(data.instrument_type) == accounting_methods_.end()) {
      SPDLOG_WARN("accounting method not found for {}: {}", data.type_name.c_str(), data.to_string());
      return;
    }
    AccountingMethod &accounting_method = *accounting_methods_.at(data.instrument_type);
    auto apply_and_update = [&](uint32_t book_uid) {
      auto book = get_book(book_uid);
      (accounting_method.*apply)(book, data);
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
