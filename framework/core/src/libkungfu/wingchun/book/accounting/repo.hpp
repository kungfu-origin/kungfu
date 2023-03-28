// SPDX-License-Identifier: Apache-2.0

#ifndef WINGCHUN_ACCOUNTING_REPO_H
#define WINGCHUN_ACCOUNTING_REPO_H

#include "stock.hpp"

namespace kungfu::wingchun::book {
class RepoAccountingMethod : public StockAccountingMethod {
public:
  RepoAccountingMethod() = default;

  void apply_quote(Book_ptr &book, const Quote &quote) override {}

  void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    const auto &position = book->get_position_for(input);
    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    if (input.side == Side::Sell) {
      book->asset.frozen_cash += input.volume * cd_mr.exchange_rate;
      book->asset.avail -= input.volume * cd_mr.exchange_rate;
      update_position(book, book->get_position_for(input));
    }
  }

  void apply_order(Book_ptr &book, const Order &order) override {
    if (book->orders.find(order.order_id) == book->orders.end()) {
      book->orders.emplace(order.order_id, order);
    }

    if (is_final_status(order.status)) {
      const auto &position = book->get_position_for(order);
      auto cd_mr = get_instr_conversion_margin_rate(book, position);
      if (order.side == Side::Sell) {
        book->asset.frozen_cash -= order.volume_left * cd_mr.exchange_rate;
        book->asset.avail += order.volume_left * cd_mr.exchange_rate;
      }
    }
    update_position(book, book->get_position_for(order));
  }

  void update_position(Book_ptr &book, Position &position) override {
    // 无需计算逆回购的收益，逆回购收益在买入时就固定了
  }

  void apply_sell(Book_ptr &book, const Trade &trade) override {
    auto &position = book->get_position_for(trade);
    if (position.volume + trade.volume > 0 && trade.price > 0) {
      position.avg_open_price = (position.avg_open_price * position.volume + trade.price * trade.volume) /
                                (double)(position.volume + trade.volume);
    }
    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    position.avg_open_price = 1;
    auto commission = calculate_commission(book, trade);
    auto tax = calculate_tax(trade);
    //    auto days = get_repo_expire_days(trade.instrument_id);
    //    auto profit = trade.volume / 100 / 360 * days;

    position.volume += trade.volume;
    //    position.realized_pnl += profit;
    update_position(book, position);

    //    book->asset.realized_pnl += profit;
    book->asset.frozen_cash -= trade.volume * cd_mr.exchange_rate;
    book->asset.avail -= commission + tax;
    book->asset.intraday_fee += commission + tax;
    book->asset.accumulated_fee += commission + tax;
  }

  double calculate_commission(const Book_ptr &book, const Trade &trade) {
    const auto &position = book->get_position_for(trade);
    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    auto rate = get_repo_commission_rate(trade.instrument_id);
    return trade.volume * rate * cd_mr.exchange_rate;
  }

  double calculate_tax(const Trade &trade) override { return 0.0; }
};
} // namespace kungfu::wingchun::book

#endif