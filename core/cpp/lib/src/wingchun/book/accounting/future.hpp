//
// Created by Keren Dong on 2020/4/6.
//

#ifndef WINGCHUN_ACCOUNTING_FUTURE_H
#define WINGCHUN_ACCOUNTING_FUTURE_H

#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
class FutureAccountingMethod : public AccountingMethod {
public:
  FutureAccountingMethod() = default;

  void apply_trading_day(Book_ptr &book, int64_t trading_day) override {
    auto apply = [&](PositionMap &positions) {
      for (auto &pair : positions) {
        auto &position = pair.second;
        auto instrument_key = hash_instrument(position.exchange_id, position.instrument_id);
        if (book->instruments.find(instrument_key) == book->instruments.end()) {
          SPDLOG_WARN("instrument information missing for {}@{}", position.instrument_id, position.exchange_id);
          continue;
        }
        auto &instrument = book->instruments.at(instrument_key);
        auto margin_pre = position.margin;
        if (not is_valid_price(position.settlement_price)) {
          if (is_valid_price(position.last_price)) {
            position.settlement_price = position.last_price;
          } else {
            position.settlement_price = position.avg_open_price;
          }
        }
        position.margin = instrument.contract_multiplier * position.settlement_price * position.volume *
                          margin_ratio(instrument, position);
        book->asset.avail -= position.margin - margin_pre;
        position.pre_settlement_price = position.settlement_price;
        position.last_price = position.settlement_price;
        position.settlement_price = 0;
        position.yesterday_volume = position.volume;
        position.trading_day = time::strftime(trading_day, KUNGFU_TRADING_DAY_FORMAT).c_str();
        update_position(book, position);
      }
    };
    apply(book->long_positions);
    apply(book->short_positions);
  }

  void apply_quote(Book_ptr &book, const Quote &quote) override {
    auto apply = [&](Position &position) {
      auto instrument_key = hash_instrument(quote.exchange_id, quote.instrument_id);
      if (book->instruments.find(instrument_key) == book->instruments.end()) {
        SPDLOG_WARN("instrument information missing for {}@{}", quote.instrument_id, quote.exchange_id);
        return;
      }
      auto &instrument = book->instruments.at(instrument_key);
      if (is_valid_price(quote.settlement_price)) {
        auto margin_pre = position.margin;
        position.margin = instrument.contract_multiplier * position.settlement_price * position.volume *
                          margin_ratio(instrument, position);
        position.settlement_price = quote.settlement_price;
        book->asset.avail -= position.margin - margin_pre;
      }
      if (is_valid_price(quote.last_price)) {
        position.last_price = quote.last_price;
      }
      if (is_valid_price(quote.pre_settlement_price)) {
        position.pre_settlement_price = quote.pre_settlement_price;
      }
      update_position(book, position);
    };
    apply(book->get_position_for(Direction::Long, quote));
    apply(book->get_position_for(Direction::Short, quote));
  }

  void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    auto &position = book->get_position_for(input);
    auto instrument_key = hash_instrument(input.exchange_id, input.instrument_id);
    if (book->instruments.find(instrument_key) == book->instruments.end()) {
      SPDLOG_WARN("instrument information missing for {}@{}", input.instrument_id, input.exchange_id);
      return;
    }
    auto &instrument = book->instruments.at(instrument_key);
    if (input.offset == Offset::Open) {
      auto frozen_margin =
          instrument.contract_multiplier * input.frozen_price * input.volume * margin_ratio(instrument, position);
      book->asset.avail -= frozen_margin;
      book->asset.frozen_cash += frozen_margin;
      book->asset.frozen_margin += frozen_margin;
    }
    if (input.offset == Offset::Close or input.offset == Offset::CloseYesterday) {
      position.frozen_total += input.volume;
      if (position.yesterday_volume - position.frozen_yesterday >= input.volume) {
        position.frozen_yesterday += input.volume;
      } else {
        position.frozen_yesterday = position.yesterday_volume;
      }
    }
    if (input.offset == Offset::CloseToday) {
      position.frozen_total += input.volume;
    }
    update_position(book, position);
  }

  void apply_order(Book_ptr &book, const Order &order) override {
    if (book->orders.find(order.order_id) == book->orders.end()) {
      book->orders.emplace(order.order_id, order);
    }
    if (is_final_status(order.status)) {
      auto &position = book->get_position_for(order);
      auto instrument_key = hash_instrument(order.exchange_id, order.instrument_id);
      if (book->instruments.find(instrument_key) == book->instruments.end()) {
        SPDLOG_WARN("instrument information missing for {}@{}", order.instrument_id, order.exchange_id);
        return;
      }
      auto &instrument = book->instruments.at(instrument_key);
      if (order.offset == Offset::Open) {
        auto frozen_margin = instrument.contract_multiplier * order.frozen_price * order.volume_left *
                             margin_ratio(instrument, position);
        book->asset.avail += frozen_margin;
        book->asset.frozen_cash -= frozen_margin;
        book->asset.frozen_margin -= frozen_margin;
      }
      if (order.offset == Offset::Close or order.offset == Offset::CloseYesterday) {
        position.frozen_total = std::max(position.frozen_total - order.volume_left, VOLUME_ZERO);
        position.frozen_yesterday = std::max(position.frozen_yesterday - order.volume_left, VOLUME_ZERO);
      }
      if (order.offset == Offset::CloseToday and position.frozen_total >= order.volume_left) {
        position.frozen_total -= order.volume_left;
      }
      update_position(book, position);
    }
  }

  void apply_trade(Book_ptr &book, const Trade &trade) override {
    if (book->instruments.find(hash_instrument(trade.exchange_id, trade.instrument_id)) == book->instruments.end()) {
      SPDLOG_WARN("instrument information missing for {}@{}", trade.instrument_id, trade.exchange_id);
      return;
    }
    if (trade.offset == Offset::Open) {
      apply_open(book, trade);
    }
    if (trade.offset == Offset::Close or trade.offset == Offset::CloseToday or trade.offset == Offset::CloseYesterday) {
      apply_close(book, trade);
    }
  }

  void update_position(Book_ptr &book, Position &position) override {
    if (position.last_price > 0) {
      auto &instrument = book->instruments.at(hash_instrument(position.exchange_id, position.instrument_id));
      auto product_key = yijinjing::util::hash_str_32(get_instrument_product(instrument.instrument_id));
      double cost = 0;
      if (book->commissions.find(product_key) != book->commissions.end()) {
        auto &commission = book->commissions.at(product_key);
        auto close_today_volume = double(position.volume - position.yesterday_volume);
        if (commission.mode == CommissionRateMode::ByAmount) {
          cost = (position.last_price * position.yesterday_volume * commission.close_ratio) +
                 (position.last_price * close_today_volume * commission.close_today_ratio);
        } else {
          cost = (position.yesterday_volume * commission.close_ratio) +
                 (close_today_volume * commission.close_today_ratio);
        }
        cost = cost * instrument.contract_multiplier;
      }
      auto multiplier = instrument.contract_multiplier * (position.direction == Direction::Long ? 1 : -1);
      auto price_diff = position.last_price - position.avg_open_price;
      position.unrealized_pnl = (price_diff * position.volume) * multiplier - cost;
    }
  }

private:
  void apply_open(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    auto &instrument = book->instruments.at(hash_instrument(trade.exchange_id, trade.instrument_id));
    auto margin = instrument.contract_multiplier * trade.price * trade.volume * margin_ratio(instrument, position);
    auto commission = calculate_commission(book, trade, instrument, position, trade.close_today_volume);
    auto frozen_margin = instrument.contract_multiplier * book->get_frozen_price(trade.order_id) * trade.volume *
                         margin_ratio(instrument, position);
    position.margin += margin;
    position.avg_open_price = (position.avg_open_price * position.volume + trade.price * trade.volume) /
                              double(position.volume + trade.volume);
    position.volume += trade.volume;
    update_position(book, position);
    book->asset.frozen_cash -= frozen_margin;
    book->asset.frozen_margin -= frozen_margin;
    book->asset.avail -= commission;
    // book->asset.avail += frozen_margin;
    book->asset.avail -= margin;
    book->asset.accumulated_fee += commission;
    book->asset.intraday_fee += commission;
    book->asset.margin += margin;
  }

  void apply_close(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    auto &instrument = book->instruments.at(hash_instrument(trade.exchange_id, trade.instrument_id));
    auto today_volume_pre = position.volume - position.yesterday_volume;
    auto margin = instrument.contract_multiplier * trade.price * trade.volume * margin_ratio(instrument, position);
    auto delta_margin = std::min(position.margin, margin);
    position.margin -= delta_margin;
    position.volume -= trade.volume;
    position.frozen_total -= trade.volume;
    if (trade.offset != Offset::CloseToday) {
      position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
      position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    }
    auto close_today_volume = position.volume - position.yesterday_volume - today_volume_pre;
    auto commission = calculate_commission(book, trade, instrument, position, close_today_volume);
    auto realized_pnl = (trade.price - position.avg_open_price) * trade.volume * instrument.contract_multiplier;
    if (position.direction == Direction::Short) {
      realized_pnl = -realized_pnl;
    }
    position.realized_pnl += realized_pnl;
    update_position(book, position);
    book->asset.realized_pnl += realized_pnl;
    book->asset.avail += delta_margin;
    book->asset.avail -= commission;
    book->asset.accumulated_fee += commission;
    book->asset.intraday_fee += commission;
  }

  static double calculate_commission(Book_ptr &book, const Trade &trade, const Instrument &instrument,
                                     const Position &position, double close_today_volume) {
    auto contract_multiplier = instrument.contract_multiplier;
    auto product_key = yijinjing::util::hash_str_32(get_instrument_product(instrument.instrument_id));
    if (book->commissions.find(product_key) == book->commissions.end()) {
      SPDLOG_WARN("commission information missing for {}@{}", trade.instrument_id, trade.exchange_id);
      return 0;
    }
    auto &commission = book->commissions.at(product_key);
    if (commission.mode == CommissionRateMode::ByAmount) {
      if (trade.offset == Offset::Open) {
        return trade.price * trade.volume * contract_multiplier * commission.open_ratio;
      } else {
        auto volume_left = double(trade.volume) - close_today_volume;
        return (trade.price * volume_left * contract_multiplier * commission.close_ratio) +
               (trade.price * close_today_volume * contract_multiplier * commission.close_today_ratio);
      }
    } else {
      if (trade.offset == Offset::Open) {
        return double(trade.volume) * contract_multiplier * commission.open_ratio;
      } else {
        auto volume_left = double(trade.volume - close_today_volume);
        return (volume_left * contract_multiplier * commission.close_ratio) +
               (close_today_volume * contract_multiplier * commission.close_today_ratio);
      }
    }
  }

  static double margin_ratio(const Instrument &instrument, const Position &position) {
    return position.direction == Direction::Long ? instrument.long_margin_ratio : instrument.short_margin_ratio;
  }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_FUTURE_H
