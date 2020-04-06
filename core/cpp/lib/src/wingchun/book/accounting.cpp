//
// Created by Keren Dong on 2020/4/4.
//

#include "accounting/future.hpp"
#include "accounting/stock.hpp"

using namespace kungfu::wingchun;

namespace kungfu::wingchun::book {
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