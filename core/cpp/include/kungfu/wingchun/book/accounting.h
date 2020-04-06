//
// Created by Keren Dong on 2020/4/6.
//

#ifndef WINGCHUN_ACCOUNTING_H
#define WINGCHUN_ACCOUNTING_H

#include <kungfu/wingchun/book/book.h>

namespace kungfu::wingchun::book {
class AccountingMethod {
public:
  AccountingMethod() = default;

  virtual ~AccountingMethod() = default;

  virtual void apply_trading_day(Book_ptr book, int64_t trading_day) = 0;

  virtual void apply_quote(Book_ptr book, const longfist::types::Quote &quote) = 0;

  virtual void apply_order_input(Book_ptr book, const longfist::types::OrderInput &input) = 0;

  virtual void apply_order(Book_ptr book, const longfist::types::Order &order) = 0;

  virtual void apply_trade(Book_ptr book, const longfist::types::Trade &trade) = 0;

  static void setup_defaults(Bookkeeper &bookkeeper);
};
DECLARE_PTR(AccountingMethod)
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_H
