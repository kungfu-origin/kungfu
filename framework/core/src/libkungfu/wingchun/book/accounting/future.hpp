// SPDX-License-Identifier: Apache-2.0

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

#define DEFAULT_INSTRUMENT_CONTRACT_MULTIPLIER 10
#define DEFAULT_INSTRUMENT_LONG_MARGIN_RATIO 0.1
#define DEFAULT_INSTRUMENT_SHORT_MARGIN_RATIO 0.1
#define DEFAULT_INSTRUMENT_EXCHANGE_RATE 1.0

struct contract_multiplier_and_margin_ratio {
  int32_t contract_multiplier;
  double margin_ratio;
  double exchange_rate;
};

class FutureAccountingMethod : public AccountingMethod {
public:
  FutureAccountingMethod() = default;

  void apply_trading_day(Book_ptr &book, int64_t trading_day) override {
    auto apply = [&](PositionMap &positions) {
      for (auto &pair : positions) {
        auto &position = pair.second;
        auto margin_pre = position.margin;
        if (not is_valid_price(position.settlement_price)) {
          if (is_valid_price(position.last_price)) {
            position.settlement_price = position.last_price;
          } else {
            position.settlement_price = position.avg_open_price;
          }
        }

        auto cm_mr = get_instrument_contract_multiplier_and_margin_ratio(book, position.exchange_id,
                                                                         position.instrument_id, position);

        position.margin = cm_mr.contract_multiplier * position.settlement_price * position.volume * cm_mr.margin_ratio;

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
      auto cm_mr =
          get_instrument_contract_multiplier_and_margin_ratio(book, quote.exchange_id, quote.instrument_id, position);

      // 此处仅计算结算价，但需要根据实时行情变化
      if (is_valid_price(quote.settlement_price)) {
        auto margin_pre = position.margin;
        position.margin = cm_mr.contract_multiplier * position.settlement_price * cm_mr.exchange_rate *
                          position.volume * cm_mr.margin_ratio;

        position.settlement_price = quote.settlement_price; // 这行代码为啥在上一行代码的下面？
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
    auto offset = get_offset(book, input);
    auto direction = get_direction(input.instrument_type, input.side, offset);
    auto &position = book->get_position(direction, input.exchange_id, input.instrument_id);

    auto cm_mr =
        get_instrument_contract_multiplier_and_margin_ratio(book, input.exchange_id, input.instrument_id, position);

    if (offset == Offset::Open) {
      auto frozen_margin =
          cm_mr.contract_multiplier * input.frozen_price * cm_mr.exchange_rate * input.volume * cm_mr.margin_ratio;

      book->asset.avail -= frozen_margin;
      book->asset.frozen_cash += frozen_margin;
      book->asset.frozen_margin += frozen_margin;
    }

    if (offset == Offset::Close or offset == Offset::CloseYesterday) {
      position.frozen_total += input.volume;
      if (position.yesterday_volume - position.frozen_yesterday >= input.volume) {
        position.frozen_yesterday += input.volume;
      } else {
        position.frozen_yesterday = position.yesterday_volume;
      }
    }

    if (offset == Offset::CloseToday) {
      position.frozen_total += input.volume;
    }

    update_position(book, position);
  }

  void apply_order(Book_ptr &book, const Order &order) override {
    if (book->orders.find(order.order_id) == book->orders.end()) {
      book->orders.insert_or_assign(order.order_id, order);
    }

    if (not is_final_status(order.status))
      return;

    auto offset = get_offset(book, order);
    auto direction = get_direction(order.instrument_type, order.side, offset);
    auto &position = book->get_position(direction, order.exchange_id, order.instrument_id);
    auto cm_mr =
        get_instrument_contract_multiplier_and_margin_ratio(book, order.exchange_id, order.instrument_id, position);

    if (offset == Offset::Open) {
      auto frozen_margin =
          cm_mr.contract_multiplier * order.frozen_price * cm_mr.exchange_rate * order.volume_left * cm_mr.margin_ratio;

      book->asset.avail += frozen_margin;
      book->asset.frozen_cash -= frozen_margin;
      book->asset.frozen_margin -= frozen_margin;
    }

    if (offset == Offset::Close or offset == Offset::CloseYesterday) {
      position.frozen_total = std::max(position.frozen_total - order.volume_left, VOLUME_ZERO);
      position.frozen_yesterday = std::max(position.frozen_yesterday - order.volume_left, VOLUME_ZERO);
    }

    if (offset == Offset::CloseToday and position.frozen_total >= order.volume_left) {
      position.frozen_total -= order.volume_left;
    }

    update_position(book, position);
  }

  void apply_trade(Book_ptr &book, const Trade &trade) override {
    if (book->trades.find(trade.trade_id) == book->trades.end()) {
      book->trades.emplace(trade.trade_id, trade);
    }

    auto offset = get_offset(book, trade);
    auto direction = get_direction(trade.instrument_type, trade.side, offset);
    auto &position = book->get_position(direction, trade.exchange_id, trade.instrument_id);

    if (offset == Offset::Open) {
      apply_open(book, position, trade);
    }
    if (offset == Offset::Close or offset == Offset::CloseToday or offset == Offset::CloseYesterday) {
      // the extra offset is for merge position situation
      apply_close(book, position, trade);
    }
  }

  void update_position(Book_ptr &book, Position &position) override {
    if (position.last_price > 0) {

      auto cm_mr = get_instrument_contract_multiplier_and_margin_ratio(book, position.exchange_id,
                                                                       position.instrument_id, position);

      auto contract_multiplier = cm_mr.contract_multiplier;
      auto product_key = yijinjing::util::hash_str_32(get_instrument_product(position.instrument_id));
      double cost = 0;

      if (book->commissions.find(product_key) != book->commissions.end()) {
        auto &commission = book->commissions.at(product_key);
        auto close_today_volume = double(position.volume - position.yesterday_volume);
        if (commission.mode == CommissionRateMode::ByAmount) {
          cost = (position.last_price /** cm_mr.exchange_rate*/ * position.yesterday_volume * commission.close_ratio) +
                 (position.last_price /** cm_mr.exchange_rate*/ * close_today_volume * commission.close_today_ratio);
        } else {
          cost = (position.yesterday_volume * commission.close_ratio) +
                 (close_today_volume * commission.close_today_ratio);
        }
        cost = cost * contract_multiplier;
      }

      auto multiplier = contract_multiplier * (position.direction == Direction::Long ? 1 : -1);
      auto price_diff = position.last_price - position.avg_open_price;
      // 浮动盈亏
      position.unrealized_pnl = (price_diff * position.volume) /** cm_mr.exchange_rate*/ * multiplier - cost;
    }
  }

private:
  void apply_open(Book_ptr &book, Position &position, const Trade &trade) {
    auto cm_mr =
        get_instrument_contract_multiplier_and_margin_ratio(book, trade.exchange_id, trade.instrument_id, position);

    auto contract_multiplier = cm_mr.contract_multiplier;
    auto margin_ratio_by_pos = cm_mr.margin_ratio;
    auto margin = contract_multiplier * trade.price * cm_mr.exchange_rate * trade.volume * margin_ratio_by_pos;
    auto commission = calculate_commission(book, trade, position, 0);
    auto frozen_margin = contract_multiplier * book->get_frozen_price(trade.order_id) * cm_mr.exchange_rate *
                         trade.volume * margin_ratio_by_pos;
    position.margin += margin;
    position.avg_open_price = (position.avg_open_price * position.volume + trade.price * trade.volume) /
                              double(position.volume + trade.volume);
    position.volume += trade.volume;
    update_position(book, position);

    book->asset.avail += frozen_margin;
    book->asset.frozen_cash -= frozen_margin;
    book->asset.frozen_margin -= frozen_margin;

    book->asset.avail -= commission;
    book->asset.avail -= margin;
    book->asset.accumulated_fee += commission;
    book->asset.intraday_fee += commission;
    book->asset.margin += margin;
  }

  void apply_close(Book_ptr &book, Position &position, const Trade &trade) {
    auto cm_mr =
        get_instrument_contract_multiplier_and_margin_ratio(book, trade.exchange_id, trade.instrument_id, position);
    auto contract_multiplier = cm_mr.contract_multiplier;
    auto margin = contract_multiplier * trade.price * cm_mr.exchange_rate * trade.volume * cm_mr.margin_ratio;
    auto delta_margin = std::min(position.margin, margin);
    position.margin -= delta_margin;
    position.volume -= trade.volume;
    position.frozen_total -= trade.volume;
    auto close_today_volume = 0.0;
    if (trade.offset != Offset::CloseToday) {
      close_today_volume = std::max(trade.volume - position.yesterday_volume, VOLUME_ZERO);
      position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
      position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    } else {
      close_today_volume = trade.volume;
    }

    auto commission = calculate_commission(book, trade, position, close_today_volume);
    auto realized_pnl =
        (trade.price - position.avg_open_price) /** cm_mr.exchange_rate*/ * trade.volume * contract_multiplier;
    if (position.direction == Direction::Short) {
      realized_pnl = -realized_pnl;
    }
    position.realized_pnl += realized_pnl;
    update_position(book, position);
    book->asset.realized_pnl += realized_pnl * cm_mr.exchange_rate;
    book->asset.avail += delta_margin;
    book->asset.avail -= commission;
    book->asset.accumulated_fee += commission;
    book->asset.intraday_fee += commission;
  }

  template <typename TradingData>
  [[nodiscard]] bool need_to_merge_long_short_positions(Book_ptr &book, const TradingData &trading_data) const {
    if (not able_long_short_position_merge(trading_data.exchange_id))
      return false;

    auto &oppsite_position = book->get_oppsite_position_for(trading_data);
    if (oppsite_position.volume > 0)
      return true;
    return false;
  }

  template <typename TradingData>
  [[nodiscard]] bool need_to_open_oppsite(Book_ptr &book, const TradingData &trading_data) const {
    if (not able_long_short_position_merge(trading_data.exchange_id))
      return false;

    auto &position = book->get_position_for(trading_data);
    if (position.volume <= 0 && trading_data.offset != Offset::Open)
      return true;
    return false;
  }

  template <typename TradingData>
  [[nodiscard]] longfist::enums::Offset get_offset(Book_ptr &book, const TradingData &trading_data) const {
    auto offset = trading_data.offset;
    if (need_to_merge_long_short_positions(book, trading_data) && offset == Offset::Open) {
      return Offset::Close;
    }

    if (need_to_open_oppsite(book, trading_data) && offset != Offset::Open) {
      return Offset::Open;
    }

    return offset;
  }

  static double calculate_commission(Book_ptr &book, const Trade &trade, const Position &position,
                                     double close_today_volume) {
    auto cm_mr =
        get_instrument_contract_multiplier_and_margin_ratio(book, trade.exchange_id, trade.instrument_id, position);

    auto contract_multiplier = cm_mr.contract_multiplier;
    auto product_key = yijinjing::util::hash_str_32(get_instrument_product(trade.instrument_id));
    if (book->commissions.find(product_key) == book->commissions.end()) {
      SPDLOG_WARN("commission information missing for {}@{}", trade.instrument_id, trade.exchange_id);
      return 0;
    }
    auto &commission = book->commissions.at(product_key);
    if (commission.mode == CommissionRateMode::ByAmount) {
      if (trade.offset == Offset::Open) {
        return trade.price * cm_mr.exchange_rate * trade.volume * contract_multiplier * commission.open_ratio;
      } else {
        auto volume_left = double(trade.volume) - close_today_volume;
        return (trade.price * cm_mr.exchange_rate * volume_left * contract_multiplier * commission.close_ratio) +
               (trade.price * cm_mr.exchange_rate * close_today_volume * contract_multiplier *
                commission.close_today_ratio);
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

  static contract_multiplier_and_margin_ratio
  get_instrument_contract_multiplier_and_margin_ratio(Book_ptr &book, const char *exchange_id,
                                                      const char *instrument_id, const Position &position) {
    auto hashed_instrument_key = hash_instrument(exchange_id, instrument_id);
    contract_multiplier_and_margin_ratio cm_mr = {};
    if (book->instruments.find(hashed_instrument_key) == book->instruments.end()) {
      SPDLOG_WARN("instrument information missing for {}@{}", instrument_id, exchange_id);
      cm_mr.contract_multiplier = DEFAULT_INSTRUMENT_CONTRACT_MULTIPLIER;
      cm_mr.margin_ratio = position.direction == Direction::Long ? DEFAULT_INSTRUMENT_LONG_MARGIN_RATIO
                                                                 : DEFAULT_INSTRUMENT_SHORT_MARGIN_RATIO;
      cm_mr.exchange_rate = DEFAULT_INSTRUMENT_EXCHANGE_RATE;
      return cm_mr;
    }

    auto &instrument = book->instruments.at(hashed_instrument_key);
    cm_mr.contract_multiplier = instrument.contract_multiplier;
    cm_mr.margin_ratio = margin_ratio(instrument, position);
    cm_mr.exchange_rate = is_equal(instrument.exchange_rate, 0.0) ? 1.0 : instrument.exchange_rate;
    return cm_mr;
  }

  static double margin_ratio(const Instrument &instrument, const Position &position) {
    return position.direction == Direction::Long ? instrument.long_margin_ratio : instrument.short_margin_ratio;
  }

  static bool able_long_short_position_merge(const char *exchange_id) {
    if (strcmp(exchange_id, EXCHANGE_US_FUTURE) == 0 || strcmp(exchange_id, EXCHANGE_HK_FUTURE) == 0 ||
        strcmp(exchange_id, EXCHANGE_SGX_FUTURE) == 0 || strcmp(exchange_id, EXCHANGE_LON_FUTURE) ||
        strcmp(exchange_id, EXCHANGE_AEX_FUTURE) || strcmp(exchange_id, EXCHANGE_AUX_FUTURE) ||
        strcmp(exchange_id, EXCHANGE_HEXS_FUTURE) || strcmp(exchange_id, EXCHANGE_IDX_FUTURE) ||
        strcmp(exchange_id, EXCHANGE_KORC) || strcmp(exchange_id, EXCHANGE_LME) ||
        strcmp(exchange_id, EXCHANGE_MYS_FUTURE) || strcmp(exchange_id, EXCHANGE_ABB) ||
        strcmp(exchange_id, EXCHANGE_PRX_FUTURE) || strcmp(exchange_id, EXCHANGE_SIX_FUTURE) ||
        strcmp(exchange_id, EXCHANGE_TAX_FUTURE) || strcmp(exchange_id, EXCHANGE_JP_FUTURE) ||
        strcmp(exchange_id, EXCHANGE_TSE_FUTURE) || strcmp(exchange_id, EXCHANGE_XETRA)) {
      return true;
    }

    return false;
  }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_FUTURE_H
