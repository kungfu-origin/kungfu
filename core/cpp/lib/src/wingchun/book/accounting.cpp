//
// Created by Keren Dong on 2020/4/4.
//

#include <kungfu/wingchun/book/book.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
class StockAccountingMethod : public AccountingMethod {
public:
  static constexpr double min_comission = 5;
  static constexpr double commission_ratio = 0.0008;

  StockAccountingMethod() = default;

  void apply_trading_day(Book_ptr book, int64_t trading_day) override {}

  void apply_quote(Book_ptr book, const longfist::types::Quote &quote) override {
    auto &position = book->get_long_position(quote);
    if (is_valid_price(quote.close_price)) {
      position.close_price = quote.close_price;
    }
    if (is_valid_price(quote.last_price)) {
      position.last_price = quote.last_price;
    }
    if (is_valid_price(quote.pre_close_price)) {
      position.pre_close_price = quote.pre_close_price;
    }
    position.unrealized_pnl =
        position.last_price > 0 ? (position.last_price - position.avg_open_price) * position.volume : 0;
  }

  void apply_order_input(Book_ptr book, const longfist::types::OrderInput &input) override {}

  void apply_order(Book_ptr book, const longfist::types::Order &order) override {}

  void apply_trade(Book_ptr book, const longfist::types::Trade &trade) override {}

private:
  std::unordered_map<uint64_t, double> commission_map_ = {};
};

class FutureAccountingMethod : public AccountingMethod {
public:
  FutureAccountingMethod() = default;

  void apply_trading_day(Book_ptr book, int64_t trading_day) override {}

  void apply_quote(Book_ptr book, const longfist::types::Quote &quote) override {}

  void apply_order_input(Book_ptr book, const longfist::types::OrderInput &input) override {}

  void apply_order(Book_ptr book, const longfist::types::Order &order) override {}

  void apply_trade(Book_ptr book, const longfist::types::Trade &trade) override {}

private:
  std::unordered_map<uint64_t, double> commission_map_ = {};
};

void AccountingMethod::setup_defaults(Bookkeeper &bookkeeper) {
  auto stock_accounting_method = std::make_shared<StockAccountingMethod>();
  auto future_accounting_method = std::make_shared<FutureAccountingMethod>();

  bookkeeper.set_accounting_method(InstrumentType::Stock, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Bond, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Fund, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::StockOption, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::TechStock, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Index, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Repo, stock_accounting_method);

  bookkeeper.set_accounting_method(InstrumentType::Future, future_accounting_method);

  bookkeeper.set_accounting_method(InstrumentType::Unknown, stock_accounting_method);
}
} // namespace kungfu::wingchun::book