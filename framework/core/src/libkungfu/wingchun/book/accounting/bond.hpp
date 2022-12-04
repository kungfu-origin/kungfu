#ifndef WINGCHUN_ACCOUNTING_BOND_H
#define WINGCHUN_ACCOUNTING_BOND_H

#include "stock.hpp"

namespace kungfu::wingchun::book {

class BondAccountingMethod : public StockAccountingMethod {
public:
  BondAccountingMethod() = default;

  void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    auto &position = book->get_position_for(input);
    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    if (!is_convertible_bond(input.instrument_id, input.exchange_id)) {
      StockAccountingMethod::apply_order_input(book, input);
      return;
    }

    if (input.side == Side::Sell && position.volume - position.frozen_total >= input.volume) {
      position.frozen_total += input.volume;
      position.frozen_yesterday = std::max(position.frozen_yesterday + input.volume, position.yesterday_volume);
    }
    if (input.side == Side::Buy) {
      book->asset.frozen_cash += input.volume * input.frozen_price * cd_mr.exchange_rate;
      book->asset.avail -= input.volume * input.frozen_price * cd_mr.exchange_rate;
    }
  }
};
} // namespace kungfu::wingchun::book

#endif