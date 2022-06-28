//
// Created by Keren Dong on 2020/4/4.
//

#include "accounting/bond.hpp"
#include "accounting/crypto.hpp"
#include "accounting/future.hpp"
#include "accounting/repo.hpp"
#include "accounting/stock.hpp"

using namespace kungfu::wingchun;

namespace kungfu::wingchun::book {
void AccountingMethod::setup_defaults(Bookkeeper &bookkeeper) {
  auto stock_accounting_method = std::make_shared<StockAccountingMethod>();
  auto bond_accounting_method = std::make_shared<BondAccountingMethod>();
  auto future_accounting_method = std::make_shared<FutureAccountingMethod>();
  auto repo_accounting_method = std::make_shared<RepoAccountingMethod>();
  auto crypto_accounting_method = std::make_shared<CryptoAccountingMethod>();

  bookkeeper.set_accounting_method(InstrumentType::Stock, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Bond, bond_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Fund, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::StockOption, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::TechStock, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Index, stock_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Repo, repo_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Future, future_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Crypto, crypto_accounting_method);
  bookkeeper.set_accounting_method(InstrumentType::Unknown, stock_accounting_method);
}
} // namespace kungfu::wingchun::book