//
// Created by Keren Dong on 2020/4/6.
//

#ifndef WINGCHUN_ACCOUNTING_STOCK_H
#define WINGCHUN_ACCOUNTING_STOCK_H

#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/book/bookkeeper.h>

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

  virtual void apply_trading_day(Book_ptr &book, int64_t trading_day) override {
    for (auto &pair : book->long_positions) {
      auto &position = pair.second;
      if (is_valid_price(position.close_price)) {
        position.pre_close_price = position.close_price;
      } else if (is_valid_price(position.last_price)) {
        position.pre_close_price = position.last_price;
      }
      position.yesterday_volume = position.volume;
      position.close_price = 0;
      position.update_time = trading_day;
      position.trading_day = time::strftime(trading_day, KUNGFU_TRADING_DAY_FORMAT).c_str();
    }
  }

  virtual void apply_quote(Book_ptr &book, const Quote &quote) override {
    auto &position = book->get_position_for(Direction::Long, quote);
    if (is_valid_price(quote.close_price)) {
      position.close_price = quote.close_price;
    }
    if (is_valid_price(quote.last_price)) {
      position.last_price = quote.last_price;
    }
    if (is_valid_price(quote.pre_close_price)) {
      position.pre_close_price = quote.pre_close_price;
    }
    update_position(book, position);
  }

  virtual void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    auto &position = book->get_position_for(input);
    if (input.side == Side::Sell and position.yesterday_volume - position.frozen_yesterday >= input.volume) {
      position.frozen_total += input.volume;
      position.frozen_yesterday += input.volume;
    }
    if (input.side == Side::Buy) {
      book->asset.frozen_cash += input.volume * input.frozen_price;
      book->asset.avail -= input.volume * input.frozen_price;
    }
    update_position(book, position);
  }

  virtual void apply_order(Book_ptr &book, const Order &order) override {
    if (book->orders.find(order.order_id) == book->orders.end()) {
      book->orders.emplace(order.order_id, order);
    }

    if (is_final_status(order.status)) {
      auto &position = book->get_position_for(order);
      if (order.side == Side::Buy) {
        auto frozen = book->get_frozen_price(order.order_id) * order.volume_left;
        book->asset.frozen_cash -= frozen;
        book->asset.avail += frozen;
      } else if (order.side == Side::Sell) {
        position.frozen_total = std::max(position.frozen_total - order.volume_left, VOLUME_ZERO);
        position.frozen_yesterday = std::max(position.frozen_yesterday - order.volume_left, VOLUME_ZERO);
      }
      update_position(book, position);
    }
  }

  virtual void apply_trade(Book_ptr &book, const Trade &trade) override {
    if (trade.side == Side::Sell) {
      apply_sell(book, trade);
    }
    if (trade.side == Side::Buy) {
      apply_buy(book, trade);
    }
    update_position(book, book->get_position_for(trade));
  }

  virtual void update_position(Book_ptr &book, Position &position) override {
    if (position.last_price > 0) {
      position.unrealized_pnl = (position.last_price - position.avg_open_price) * position.volume;
    }
  }

protected:
  std::unordered_map<uint64_t, double> commission_map_ = {};

  virtual void apply_buy(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    if (position.volume + trade.volume > 0 && trade.price > 0) {
      position.avg_open_price = (position.avg_open_price * position.volume + trade.price * trade.volume) /
                                (double)(position.volume + trade.volume);
    }
    auto commission = calculate_commission(trade);
    auto tax = calculate_tax(trade);
    position.volume += trade.volume;

    update_position(book, position);

    auto frozen = book->get_frozen_price(trade.order_id) * trade.volume;
    book->asset.frozen_cash -= frozen;
    book->asset.avail -= commission;
    book->asset.avail -= tax;
    book->asset.avail += frozen;
    book->asset.avail -= trade.price * trade.volume;
    book->asset.intraday_fee += commission + tax;
    book->asset.accumulated_fee += commission + tax;
  }

  virtual void apply_sell(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    auto realized_pnl = (trade.price - position.avg_open_price) * trade.volume;
    auto commission = calculate_commission(trade);
    auto tax = calculate_tax(trade);
    position.frozen_total = std::max(position.frozen_total - trade.volume, VOLUME_ZERO);
    position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
    position.volume = std::max(position.volume - trade.volume, VOLUME_ZERO);
    position.realized_pnl += realized_pnl;

    update_position(book, position);

    book->asset.realized_pnl += realized_pnl;
    book->asset.avail += trade.price * trade.volume;
    book->asset.avail -= commission;
    book->asset.avail -= tax;
    book->asset.intraday_fee += commission + tax;
    book->asset.accumulated_fee += commission + tax;
  }

  virtual double calculate_commission(const Trade &trade) {
    if (commission_map_.find(trade.order_id) == commission_map_.end()) {
      commission_map_.emplace(trade.order_id, min_comission);
    }
    auto commission = commission_map_[trade.order_id];
    auto amount = trade.price * trade.volume * commission_ratio;
    if (commission == min_comission) {
      if (amount > commission) {
        commission_map_.emplace(trade.order_id, 0);
        return amount;
      } else {
        commission_map_[trade.order_id] = commission_map_[trade.order_id] - amount;
        return min_comission;
      }
    } else {
      if (amount > commission) {
        commission_map_.emplace(trade.order_id, 0);
        return amount - commission;
      } else {
        commission_map_[trade.order_id] = commission_map_[trade.order_id] - amount;
        return 0;
      }
    }
  }

  virtual double calculate_tax(const Trade &trade) {
    return trade.side == Side::Sell ? trade.price * trade.volume * 0.001 : 0;
  }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_STOCK_H
