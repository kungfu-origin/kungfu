//
// Created by Keren Dong on 2020/4/6.
// Updated for Margin Account on 2022/6/6
//

#ifndef WINGCHUN_ACCOUNTING_STOCK_H
#define WINGCHUN_ACCOUNTING_STOCK_H

#include <exception>
#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/book/bookkeeper.h>
#include <math.h>
#include <mutex>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {

struct contract_discount_and_margin_ratio {
  int32_t contract_multiplier;
  double long_margin_ratio;
  double short_margin_ratio;
  double margin_ratio;
  double conversion_rate; // For collateral/avail_margin calculation
};
class StockAccountingMethod : public AccountingMethod {
public:
  static constexpr double min_comission = 5;
  static constexpr double default_commission_ratio = 0.00032;

  static constexpr int DEFAULT_STOCK_CONTRACT_MULTIPLIER = 1;
  static constexpr float DEFAULT_STOCK_LONG_MARGIN_RATIO = 1.0;
  static constexpr float DEFAULT_STOCK_SHORT_MARGIN_RATIO = 0.6;
  static constexpr float DEFAULT_STOCK_CONVERSION_RATE = 0.7;
  static constexpr int AMOUT_PRECISION = 3;
  static constexpr double MAX_COLLATERAL_RATIO = 1000.0;

  StockAccountingMethod() = default;

  virtual void apply_trading_day(Book_ptr &book, int64_t trading_day) override {

    auto apply = [&](PositionMap &positions) {
      for (auto &pair : positions) {
        auto &position = pair.second;
        auto margin_pre = position.margin;
        if (is_valid_price(position.close_price)) {
          position.pre_close_price = position.close_price;
        } else if (is_valid_price(position.last_price)) {
          position.pre_close_price = position.last_price;
        }
        // collateral; security
        auto cd_mr = get_instr_conversion_margin_rate(book, position);
        auto margin_ratio =
            (position.direction == Direction::Long ? cd_mr.long_margin_ratio : cd_mr.short_margin_ratio);

        if (position.direction == Direction::Short) {
          position.margin = position.pre_close_price * position.volume * margin_ratio;
        }

        position.yesterday_volume = position.volume;
        position.close_price = 0;
        position.update_time = trading_day;
        position.frozen_total = 0;
        position.frozen_yesterday = 0;
        position.trading_day = time::strftime(trading_day, KUNGFU_TRADING_DAY_FORMAT).c_str();

        update_position(book, position);
      }
    };

    apply(book->long_positions);
    apply(book->short_positions);
  }

  virtual void apply_quote(Book_ptr &book, const Quote &quote) override {
    static int counter = 0;
    auto apply = [&](Position &position) {
      if (is_valid_price(quote.last_price) and position.volume) {
        if (not position.last_price) {
          // SPDLOG_INFO("position.last_price set to {} for {} ", quote.last_price, position.instrument_id);
          position.last_price = quote.last_price;
        }
        // position.last_price = position.last_price > 0 ? position.last_price : quote.last_price;
        double price_change = quote.last_price - position.last_price;

        if (price_change) {
          auto cd_mr = get_instr_conversion_margin_rate(book, position);
          double market_value_change = price_change * position.volume;
          double avail_margin_change;

          auto &asset = book->asset;
          auto &asset_margin = book->asset_margin;

          if (position.direction == Direction::Long) {
            avail_margin_change = cd_mr.conversion_rate * market_value_change;
            // position.margin would not be changed for Long direction, the margin depends on debt.
            // TODO: As non-margin position and margin position are combined together, need distinguish each volume.
            // asset_margin.margin_market_value += price_change * position.margin_volume;

            asset.market_value += market_value_change; // Asset.market_value means Long positions only.
            asset.unrealized_pnl += market_value_change;
            asset_margin.total_asset += market_value_change;
          } else {
            double short_margin_change = 0;
            if (quote.last_price < position.avg_open_price) {
              short_margin_change = cd_mr.short_margin_ratio * market_value_change;
            } else {
              short_margin_change = market_value_change; // short_margin_ratio as 100% when last_price > avg_open_price;
            }
            avail_margin_change = -cd_mr.conversion_rate * market_value_change - short_margin_change;
            position.margin += short_margin_change;
            asset_margin.short_margin += short_margin_change;
            asset_margin.short_market_value += market_value_change;
            // Asset_margin.margin is combined with long_margin and short_margin.
            asset_margin.margin += short_margin_change;
            asset.unrealized_pnl -= market_value_change;
          }
          asset_margin.avail_margin += avail_margin_change;

          position.last_price = quote.last_price;
          // update position.unrealized_pnl
          update_position(book, position);
          if (counter > 20) {
            counter = 0;
            calculate_marketvalue(book);
          }
        }
      }
    };
    apply(book->get_position_for(Direction::Long, quote));
    apply(book->get_position_for(Direction::Short, quote));
    ++counter;
  }

  virtual void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    auto &position = book->get_position_for(input);

    double frozen_cash = 0;
    double frozen_margin = 0;
    double frozen_fee = 0;
    auto &asset = book->asset;
    auto &asset_margin = book->asset_margin;
    // Offset: Close
    if ((input.side == Side::Sell || input.side == Side::RepayMargin) and
        position.yesterday_volume - position.frozen_yesterday >= input.volume) {
      position.frozen_total += input.volume;
      position.frozen_yesterday += input.volume;
    } else if (input.side == Side::Buy) { // Offset: Open
      // TODO: book->asset.frozen_fee += frozen_cash * fee_ratio;

      frozen_cash = input.volume * input.frozen_price + frozen_fee;
      book->asset.frozen_cash += frozen_cash;
      book->asset.avail -= frozen_cash;
    } else if (input.side == Side::RepayStock and // Offset: Close
               position.yesterday_volume - position.frozen_yesterday >= input.volume) {
      // TODO: book->asset.frozen_fee += frozen_cash * fee_ratio;
      frozen_cash = input.volume * input.frozen_price + frozen_fee;
      book->asset.frozen_cash += frozen_cash;
      book->asset.avail -= frozen_cash;
      // Short position need frozen
      position.frozen_total += input.volume;
      position.frozen_yesterday += input.volume;
    } else if (input.side == Side::MarginTrade || input.side == Side::ShortSell) {
      auto cd_mr = get_instr_conversion_margin_rate(book, position);
      // TODO: book->asset.frozen_fee += frozen_cash * fee_ratio;
      frozen_cash = input.volume * input.frozen_price + frozen_fee;

      frozen_margin =
          frozen_cash * (input.side == Side::MarginTrade ? cd_mr.long_margin_ratio : cd_mr.short_margin_ratio);
      book->asset.frozen_margin += frozen_margin;
      book->asset_margin.avail_margin -= frozen_margin;
      if (input.side == Side::MarginTrade) {
        book->asset_margin.cash_margin += frozen_margin;
      } else {
        book->asset_margin.short_margin += frozen_margin;
      }
      book->asset_margin.margin += frozen_margin;
    }
    // SPDLOG_TRACE("instrument_id {} input.side {} yesterday_volume {} frozen_total_vol {} position.unrealized_pnl {} "
    //              "last_price{} position.margin {} "
    //              "frozen_cash {} frozen_margin {} frozen_price {} "
    //              "asset.frozen_cash {} asset.avail {} market_value {} unrealized_pnl {} "
    //              "short_margin {} cash_margin {} avail_margin {} margin {} total_asset {} ",
    //              position.instrument_id, (int)input.side, (int)position.yesterday_volume, (int)position.frozen_total,
    //              (double)position.unrealized_pnl, (double)position.last_price, (double)position.margin,
    //              (double)frozen_cash, (double)frozen_margin, (double)input.frozen_price, (double)asset.frozen_cash,
    //              (double)asset.avail, (double)asset.market_value, (double)asset.unrealized_pnl,
    //              (double)asset_margin.short_margin, (double)asset_margin.cash_margin,
    //              (double)asset_margin.avail_margin, (double)asset_margin.margin, (double)asset_margin.total_asset);
  }

  virtual void apply_order(Book_ptr &book, const Order &order) override {
    if (book->orders.find(order.order_id) == book->orders.end()) {
      book->orders.emplace(order.order_id, order);
    }

    if (is_final_status(order.status)) {
      auto &position = book->get_position_for(order);
      auto &asset = book->asset;
      auto &asset_margin = book->asset_margin;
      if (order.side == Side::Buy) {
        auto frozen = book->get_frozen_price(order.order_id) * order.volume_left;
        asset.frozen_cash -= frozen;
        asset.avail += frozen;
      } else if (order.side == Side::RepayStock) {
        double frozen = book->get_frozen_price(order.order_id) * order.volume_left;
        double frozen_fee = 0;
        frozen += frozen_fee;
        asset.frozen_cash -= frozen;
        asset.avail += frozen;
        position.frozen_total = std::max(position.frozen_total - order.volume_left, VOLUME_ZERO);
        position.frozen_yesterday = std::max(position.frozen_yesterday - order.volume_left, VOLUME_ZERO);
      } else if (order.side == Side::Sell || order.side == Side::RepayMargin) {
        // RepayMargin margin_debt frozen: asset_margin.cash_debt  (MarginTrade debt), yet as it can
        // repay debt from other position, so here does not track frozen part.
        // RepayMargin allows order amount more than margin case_debt, when all case_debt repaid, the left
        // cash would be put into asset.cash
        position.frozen_total = std::max(position.frozen_total - order.volume_left, VOLUME_ZERO);
        position.frozen_yesterday = std::max(position.frozen_yesterday - order.volume_left, VOLUME_ZERO);
      } else if (order.side == Side::MarginTrade || order.side == Side::ShortSell) {
        auto cd_mr = get_instr_conversion_margin_rate(book, position);
        double release_frozen_margin =
            book->get_frozen_price(order.order_id) * order.volume_left *
            (order.side == Side::MarginTrade ? cd_mr.long_margin_ratio : cd_mr.short_margin_ratio);
        double frozen_fee = 0;
        release_frozen_margin += frozen_fee;
        asset.frozen_margin -= release_frozen_margin;

        asset_margin.avail_margin += release_frozen_margin;
        if (order.side == Side::MarginTrade) {
          asset_margin.cash_margin -= release_frozen_margin;
        } else {
          asset_margin.short_margin -= release_frozen_margin;
        }
        asset_margin.margin -= release_frozen_margin;
        asset_margin.avail_margin += release_frozen_margin;
      }
      // it assumes position.volume is changed already(apply_trade), otherwise position.volume would be inconsistant.
      update_position(book, position);
    }
  }

  virtual void apply_trade(Book_ptr &book, const Trade &trade) override {
    if (trade.side == Side::Sell) {
      apply_sell(book, trade);
    } else if (trade.side == Side::Buy) {
      apply_buy(book, trade);
    } else if (trade.side == Side::MarginTrade) {
      apply_margintrade(book, trade);
    } else if (trade.side == Side::ShortSell) {
      apply_shortsell(book, trade);
    } else if (trade.side == Side::RepayMargin) {
      apply_repaymargin(book, trade);
    } else if (trade.side == Side::RepayStock) {
      apply_repaystock(book, trade);
    };
    // update_position(book, book->get_position_for(trade));
  }

  virtual void update_position(Book_ptr &book, Position &position) override {
    if (position.last_price > 0) {
      double price_change = position.last_price - position.avg_open_price;
      position.unrealized_pnl =
          (position.direction == Direction::Long ? price_change : -price_change) * position.volume;
    }
  }

protected:
  std::unordered_map<uint64_t, double> commission_map_ = {};
  // Guard for multi-threaded
  std::mutex accounting_mutex_;
  // AccountingMethod is stateless, involve context value?
  double short_market_value_;
  double long_market_value_;

  virtual void calculate_marketvalue(Book_ptr &book) {
    double short_market_value = 0;
    double long_market_value = 0;

    auto apply = [&](PositionMap &positions, double &market_value) {
      for (auto &pair : positions) {
        auto &position = pair.second;
        auto margin_pre = position.margin;
        if (is_valid_price(position.last_price)) {
          market_value += position.volume * position.last_price;
        } else {
          if (is_valid_price(position.pre_close_price)) {
            market_value += position.volume * position.pre_close_price;
          } else if (is_valid_price(position.avg_open_price)) {
            market_value += position.volume * position.avg_open_price;
          }
        }
        position.update_time = yijinjing::time::now_in_nano();
        update_position(book, position);
      }
    };

    apply(book->long_positions, long_market_value);
    long_market_value_ = long_market_value;
    apply(book->short_positions, short_market_value);
    short_market_value_ = short_market_value;
  }

  virtual void apply_buy(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    double trade_amt = trade.price * trade.volume;
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    double tax = calculate_tax(trade);
    position.last_price = position.last_price > 0 ? position.last_price : trade.price;
    if (position.volume + trade.volume > 0 && trade.price > 0) {
      position.avg_open_price =
          (position.avg_open_price * position.volume + trade_amt) / (double)(position.volume + trade.volume);
      position.position_cost_price = (position.position_cost_price * position.volume + trade_amt + commission + tax) /
                                     (double)(position.volume + trade.volume);
    }
    double unrealized_pnl_change = (position.last_price - trade.price) * trade.volume;
    position.volume += trade.volume;
    // update_position(book, position);
    // equals to :
    position.unrealized_pnl += unrealized_pnl_change;

    auto &asset = book->asset;

    double frozen_cash_to_release = book->get_frozen_price(trade.order_id) * trade.volume;
    asset.frozen_cash -= frozen_cash_to_release;
    double avail_cash_change = frozen_cash_to_release - trade_amt - (commission + tax);
    asset.avail += avail_cash_change;
    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;
    asset.unrealized_pnl += unrealized_pnl_change;
    // position.last_price is used for market_value calc instead of trade.price.
    double position_market_value_change = trade.volume * position.last_price;
    asset.market_value += position_market_value_change;
    // asset.unrealized_pnl need to be revised if position.last_price changes.
    // auto &asset_margin = book->asset_margin;
    if (asset_margin.total_asset) { // Margin account case
      asset_margin.total_asset += position_market_value_change - trade_amt - (commission + tax);
      auto cd_mr = get_instr_conversion_margin_rate(book, position);
      double avail_margin_changes =
          position_market_value_change * cd_mr.conversion_rate - trade_amt - (commission + tax);
      book->asset_margin.avail_margin += avail_margin_changes;
      calculate_collateral_ratio(asset_margin);
    }
  }

  virtual void apply_shortsell(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    double trade_amt = trade.price * trade.volume;
    // TODO: margin_commission requires a dedicate calculate_margin_commission(Trade&);
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    auto tax = calculate_tax(trade);
    if (position.volume + trade.volume > 0 && trade.price > 0) {
      position.avg_open_price =
          (position.avg_open_price * position.volume + trade_amt) / (double)(position.volume + trade.volume);
      position.position_cost_price = (position.position_cost_price * position.volume + trade_amt - commission - tax) /
                                     (double)(position.volume + trade.volume);
    }
    double original_volume = position.volume;
    position.volume += trade.volume;
    // The market value is calculated in Book::update()
    if (not position.last_price) {
      // SPDLOG_INFO("position.last_price set to {} for {} ", trade.price, position.instrument_id);
      position.last_price = trade.price;
    }
    position.last_price = position.last_price > 0 ? position.last_price : position.avg_open_price;
    double original_last_price = position.last_price;

    auto &asset = book->asset;
    // auto &asset_margin = book->asset_margin;

    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    double frozen_margin_to_release = book->get_frozen_price(trade.order_id) * trade.volume * cd_mr.short_margin_ratio;
    double original_position_margin_change =
        (trade.price - original_last_price) * original_volume * cd_mr.short_margin_ratio;
    double margin_by_trade = trade_amt * cd_mr.short_margin_ratio;

    // short_market_value:
    double original_position_market_value = original_last_price * original_volume;
    double market_value_after_trade = trade.price * position.volume;
    double position_market_value_change_after_trade = market_value_after_trade - original_position_market_value;

    double original_position_margin = original_position_market_value * cd_mr.short_margin_ratio;
    double position_margin_after_trade = market_value_after_trade * cd_mr.short_margin_ratio;
    double position_margin_change_after_trade = position_margin_after_trade - original_position_margin;

    // SPDLOG_TRACE("original_position_market_value {} market_value_after_trade {} original_last_price {}",
    //              original_position_market_value, market_value_after_trade, original_last_price);

    // Update position last_price with trade.price --- mark the price at the trading time for delta change
    // calculation as don't know the exact last_price at present
    position.last_price = trade.price;
    position.margin = position_margin_after_trade;

    asset.frozen_margin -= frozen_margin_to_release;
    // asset.unrealized_pnl += unrealized_pnl_change;

    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;

    // Margin change casued by "Trade"
    double delta_margin = position_margin_change_after_trade - frozen_margin_to_release;
    asset_margin.margin += delta_margin;
    double short_cash = trade_amt - commission - tax;
    asset_margin.total_asset += short_cash;
    asset_margin.short_cash += short_cash;
    asset_margin.short_margin += delta_margin;
    // The total margin of account would not be changed with this ShortSell trade, so the avail_margin
    // only consider the total margin changes.
    asset_margin.avail_margin -= delta_margin + commission + tax;
    // SPDLOG_TRACE("asset_margin.margin {} short_margin {} avail_margin {}  frozen_margin_to_release {} "
    //              "position_margin_change_after_trade {} delta_margin {} ",
    //              asset_margin.margin, asset_margin.short_margin, asset_margin.avail_margin, frozen_margin_to_release,
    //              position_margin_change_after_trade, delta_margin);

    asset_margin.short_market_value += position_market_value_change_after_trade; // trade_amt;
    calculate_collateral_ratio(asset_margin);
  }

  virtual void apply_margintrade(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);

    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    double trade_amt = trade.price * trade.volume;
    // TODO: margin_commission requires a dedicate calculate_margin_commission(Trade&);
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    auto tax = calculate_tax(trade);
    // position.last_price = position.last_price > 0 ? position.last_price : position.avg_open_price;
    //  if position.last_price == 0, there is no position, position.volume expected : 0
    if (not position.last_price) {
      position.last_price = trade.price;
    }

    // The market_value calculation would be reset in Book::update(), so it does not matter if has
    // deviation of the actual status.
    // TODO: the commission & tax should be included ?
    double cash_debt_change = trade_amt;
    double market_value_change = trade.volume * position.last_price;
    // TODO: the commission & tax should be included ?
    double cash_margin_change = trade_amt * cd_mr.long_margin_ratio;

    double unrealized_pnl_change = (position.last_price - trade.price) * trade.volume;
    // SPDLOG_TRACE("position.last_price {} trade.price {} (commission + tax) {} unrealized_pnl_change {}",
    //              position.last_price, trade.price, (commission + tax), unrealized_pnl_change);

    double original_volume = position.volume;
    if (position.volume + trade.volume > 0 && trade.price > 0) {
      position.avg_open_price =
          (position.avg_open_price * original_volume + trade_amt) / (double)(original_volume + trade.volume);
      position.position_cost_price = (position.position_cost_price * position.volume + trade_amt + commission + tax) /
                                     (double)(position.volume + trade.volume);
    }
    position.unrealized_pnl += unrealized_pnl_change;
    // Track the MarginTrade part cash_debt
    position.margin += cash_debt_change; // The margin is actually the cash debt of Position instead of margin
    position.volume += trade.volume;

    auto &asset = book->asset;
    // auto &asset_margin = book->asset_margin;

    double frozen_margin_to_release = book->get_frozen_price(trade.order_id) * trade.volume * cd_mr.long_margin_ratio;
    double delta_margin =
        cash_margin_change - frozen_margin_to_release; // + unrealized_pnl_change * cd_mr.conversion_rate

    // SPDLOG_TRACE("cash_margin_change {} frozen_margin_to_release {} trade_amt {}  delta_margin {}",
    // cash_margin_change,
    //              frozen_margin_to_release, trade_amt, delta_margin);

    asset.market_value += market_value_change;
    asset.frozen_margin -= frozen_margin_to_release;
    asset.unrealized_pnl += unrealized_pnl_change;
    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;

    //(commission + tax) would be settled as margin debt?
    asset_margin.total_asset += market_value_change;
    // TODO: If the commission&tax is taken as debt:
    asset_margin.cash_debt += trade_amt + commission + tax;
    asset_margin.margin += delta_margin;
    asset_margin.cash_margin += delta_margin;
    asset_margin.avail_margin -= delta_margin + commission + tax;

    asset_margin.margin_market_value += market_value_change;
    // Position should use margin_volume to identify the long margin part.
    calculate_collateral_ratio(asset_margin);
  }

  virtual void apply_repaymargin(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    if (not position.last_price) {
      // SPDLOG_INFO("position.last_price: 0, set to {} ", trade.price);
      position.last_price = trade.price;
    }
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    auto tax = calculate_tax(trade);
    position.frozen_total = std::max(position.frozen_total - trade.volume, VOLUME_ZERO);
    position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
    position.volume = std::max(position.volume - trade.volume, VOLUME_ZERO);
    // Use position_cost_price would be better than avg_open_price for realized_pnl
    auto realized_pnl = (trade.price - position.avg_open_price) * trade.volume;
    auto unrealized_pnl_change = (position.last_price - position.avg_open_price) * trade.volume;
    position.realized_pnl += realized_pnl;
    // Need revise the unrealized_pnl since the price may change.
    double prev_unrealized_pnl = position.unrealized_pnl;
    // position.unrealized_pnl *= trade.price / position.last_price;
    position.unrealized_pnl -= unrealized_pnl_change;

    double prev_position_market_value = (position.volume + trade.volume) * position.last_price;
    // double prev_actual_position_margin = prev_position_market_value * cd_mr.long_margin_ratio;
    // Update position last_price with trade.price --- mark the price at the trading time for delta change calculation
    // as don't know the exact last_price at present
    // position.last_price = trade.price;
    // double curr_position_market_value = position.volume * position.last_price;
    // double position_market_value_change = curr_position_market_value - prev_position_market_value;
    double position_market_value_change = -position.last_price * trade.volume;

    auto &asset = book->asset;
    asset.realized_pnl += realized_pnl;
    asset.unrealized_pnl -= unrealized_pnl_change;

    asset.market_value += position_market_value_change;
    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;

    double trade_amt = trade.price * trade.volume;
    double income = trade_amt - (commission + tax);

    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    // auto &asset_margin = book->asset_margin;
    // TODO: need check repaymargin for a specified (same instrument contract) MarginDebt or not
    if (income > asset_margin.cash_debt) {
      book->asset.avail += income - asset_margin.cash_debt;
      double stock_to_cash_increased_margin = (income - asset_margin.cash_debt) * (1 - cd_mr.conversion_rate);
      asset_margin.avail_margin += asset_margin.cash_margin + stock_to_cash_increased_margin;
            // SPDLOG_TRACE("stock_to_cash_increased_margin {} asset_margin.cash_margin {}  asset_margin.avail_margin {}",
            //        stock_to_cash_increased_margin, asset_margin.cash_margin, asset_margin.avail_margin);

      // if total_asset contains the position market value, then repaymargin reduces the market value.
      asset_margin.total_asset -= asset_margin.cash_debt;
      // TODO: need check whether other cash_debt can be closed via this RepayMargin trade.
      asset_margin.cash_margin = 0;
      asset_margin.cash_debt = 0;
      asset_margin.margin = asset_margin.short_margin;
      // asset_margin.margin_market_value -= trade_amt; --> replaced by:
      asset_margin.margin_market_value += position_market_value_change;
      position.margin = 0;
    } else {

      double released_margin = income * cd_mr.long_margin_ratio;
      // This is not true when the repaid margin debt instrument is not the one of this Position.
      position.margin = std::max(position.margin - released_margin, (double)VOLUME_ZERO);
      asset_margin.cash_debt -= income;
      asset_margin.cash_margin -= released_margin;
      asset_margin.margin -= released_margin;
      asset_margin.avail_margin += released_margin - (commission + tax);
      asset_margin.total_asset += position_market_value_change - (commission + tax); // trade_amt
      // Below logic is not true:
      asset_margin.margin_market_value += position_market_value_change;
    }
    calculate_collateral_ratio(asset_margin);
  }

  virtual void apply_repaystock(Book_ptr &book, const Trade &trade) {

    auto &position = book->get_position_for(trade);
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    auto tax = calculate_tax(trade);
    // Position Direction: Short
    position.frozen_total = std::max(position.frozen_total - trade.volume, VOLUME_ZERO);
    position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
    position.volume = std::max(position.volume - trade.volume, VOLUME_ZERO);

    position.last_price = position.last_price > 0 ? position.last_price : trade.price;

    auto realized_pnl = (position.avg_open_price - trade.price) * trade.volume;
    double trade_amt = trade.price * trade.volume;
    double repay_debt_mrkt_value = position.last_price * trade.volume;
    double frozen_cash_to_release = book->get_frozen_price(trade.order_id) * trade.volume;

    auto cd_mr = get_instr_conversion_margin_rate(book, position);
    double released_margin = repay_debt_mrkt_value * cd_mr.short_margin_ratio;

    position.realized_pnl += realized_pnl;
    position.unrealized_pnl -= realized_pnl;
    position.margin -= released_margin;

    auto &asset = book->asset;
    // auto &asset_margin = book->asset_margin;

    double delta_cash = frozen_cash_to_release - trade_amt - (commission + tax);
    // realized_pnl of short position would not be involved in Asset.realized_pnl (HTS)

    // asset.frozen_cash = std::max(asset.frozen_cash - frozen_cash, 0.0);
    asset.frozen_cash -= frozen_cash_to_release;

    // TODO: The trade fee of RepayStock (commission + tax) should be paid via cash (Asset.avail)
    // instead of creating/increasing the short/cash debts ...
    asset.avail += delta_cash;
    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;

    asset_margin.margin -= released_margin;
    asset_margin.avail_margin += released_margin - (commission + tax);
    asset_margin.short_margin -= released_margin;
    asset_margin.short_market_value -= repay_debt_mrkt_value;
    asset_margin.short_cash -= trade_amt;
    asset_margin.total_asset -= trade_amt + (commission + tax);
    calculate_collateral_ratio(asset_margin);
  }

  virtual void apply_sell(Book_ptr &book, const Trade &trade) {
    auto &position = book->get_position_for(trade);
    auto &asset_margin = book->asset_margin;
    double commission = calculate_commission(trade, asset_margin.commission_ratio);
    double tax = calculate_tax(trade);
    position.frozen_total = std::max(position.frozen_total - trade.volume, VOLUME_ZERO);
    position.frozen_yesterday = std::max(position.frozen_yesterday - trade.volume, VOLUME_ZERO);
    position.yesterday_volume = std::max(position.yesterday_volume - trade.volume, VOLUME_ZERO);
    position.volume = std::max(position.volume - trade.volume, VOLUME_ZERO);
    double realized_pnl = (trade.price - position.avg_open_price) * trade.volume;
    position.realized_pnl += realized_pnl;

    double unrealized_pnl_change = (position.last_price - position.avg_open_price) * trade.volume;
    position.unrealized_pnl -= unrealized_pnl_change;
    // position.unrealized_pnl -= realized_pnl;

    update_position(book, position);
    auto &asset = book->asset;
    double trade_amt = trade.price * trade.volume;
    // repay_margin(repay_cash_debt) is actually repay cash_debt !!! not the margin concept
    double repay_cash_debt = std::min(position.margin, (trade_amt - (commission + tax)));
    // position.margin -= repay_cash_debt * cd_mr.long_margin_ratio;
    double cash_delivery = trade_amt - repay_cash_debt - (commission + tax);
    asset.realized_pnl += realized_pnl;
    asset.unrealized_pnl -= realized_pnl; // unrealized_pnl_change
    asset.avail += cash_delivery;
    asset.market_value -= trade_amt;
    asset.intraday_fee += commission + tax;
    asset.accumulated_fee += commission + tax;
    // auto &asset_margin = book->asset_margin;
    if (asset_margin.total_asset) { // asset_margin.avail_margin || asset_margin.margin
      auto cd_mr = get_instr_conversion_margin_rate(book, position);
      double avail_margin_changes = (cash_delivery - (trade_amt - repay_cash_debt) * cd_mr.conversion_rate) +
                                    repay_cash_debt * cd_mr.long_margin_ratio;
      position.margin -= repay_cash_debt * cd_mr.long_margin_ratio;
      asset_margin.avail_margin += avail_margin_changes;
      asset_margin.margin -= repay_cash_debt * cd_mr.long_margin_ratio;
      asset_margin.cash_margin -= repay_cash_debt * cd_mr.long_margin_ratio;
      asset_margin.cash_debt -= repay_cash_debt;

      asset_margin.total_asset += cash_delivery - trade_amt;

      // Not correct but better than no action. it depends on the RepayMargin targets (repay debts of instruments)
      // price.
      // sigma instrument_x.margin_trade_price * repaid_volume  = repay_cash_debt;
      // yet the actual margin_market_value change is: < Conclusion from HTS ITPMargin logic>
      // sigma instrument_x.last_price/instrument_x.margin_trade_price * repaid_volume;

      asset_margin.margin_market_value -= repay_cash_debt;
      calculate_collateral_ratio(asset_margin);
    }
  }

  virtual double calculate_commission(const Trade &trade) {
    return calculate_commission(trade, default_commission_ratio);
  }

  virtual double calculate_commission(const Trade &trade, double acct_commission_ratio) {
    if (commission_map_.find(trade.order_id) == commission_map_.end()) {
      commission_map_.emplace(trade.order_id, min_comission);
    }
    auto commission = commission_map_[trade.order_id]; // commission of history trades of the order
    double commission_ratio = acct_commission_ratio > 0 ? acct_commission_ratio : default_commission_ratio;
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
    bool is_short = (trade.side == Side::Sell) || (trade.side == Side::ShortSell) || (trade.side == Side::RepayMargin);
    return is_short ? trade.price * trade.volume * 0.001 : 0;
  }

  static contract_discount_and_margin_ratio get_instr_conversion_margin_rate(Book_ptr &book, const Position &position) {
    const char *exchange_id = position.exchange_id;
    const char *instrument_id = position.instrument_id;
    uint32_t hashed_instrument_key = hash_instrument(exchange_id, instrument_id);
    contract_discount_and_margin_ratio cd_mr = {};

    // typedef std::unordered_map<uint32_t, longfist::types::Instrument> InstrumentMap;
    if (book->instruments.find(hashed_instrument_key) == book->instruments.end()) {
      cd_mr.contract_multiplier = DEFAULT_STOCK_CONTRACT_MULTIPLIER;
      cd_mr.margin_ratio =
          position.direction == Direction::Long ? DEFAULT_STOCK_LONG_MARGIN_RATIO : DEFAULT_STOCK_SHORT_MARGIN_RATIO;
      cd_mr.long_margin_ratio = DEFAULT_STOCK_LONG_MARGIN_RATIO;
      cd_mr.short_margin_ratio = DEFAULT_STOCK_SHORT_MARGIN_RATIO;
      cd_mr.conversion_rate = DEFAULT_STOCK_CONVERSION_RATE;
      longfist::types::Instrument instrument = {};
      strcpy(instrument.instrument_id, position.instrument_id);
      strcpy(instrument.exchange_id, position.exchange_id);
      // InstrumentType
      instrument.instrument_type = get_instrument_type(instrument.exchange_id, instrument.instrument_id);
      instrument.contract_multiplier = 1;
      instrument.long_margin_ratio = cd_mr.long_margin_ratio;
      instrument.short_margin_ratio = cd_mr.short_margin_ratio;
      instrument.conversion_rate = cd_mr.conversion_rate;
      // instruments is defined with "const InstrumentMap &instruments;"
      // book->instruments.emplace(hash_instrument(exchange_id, instrument_id), instrument);
      return cd_mr;
    }
    try {
      auto &instrument = book->instruments.at(hashed_instrument_key);
      cd_mr.contract_multiplier = instrument.contract_multiplier;
      cd_mr.margin_ratio = margin_ratio(instrument, position);
      cd_mr.long_margin_ratio = instrument.long_margin_ratio;
      cd_mr.short_margin_ratio = instrument.short_margin_ratio;
      cd_mr.conversion_rate = instrument.conversion_rate;
    } catch (std::exception ex) {
      SPDLOG_ERROR("Exception for instrument_id {}: {}", instrument_id, ex.what());
      cd_mr.margin_ratio =
          position.direction == Direction::Long ? DEFAULT_STOCK_LONG_MARGIN_RATIO : DEFAULT_STOCK_SHORT_MARGIN_RATIO;
      cd_mr.long_margin_ratio = DEFAULT_STOCK_LONG_MARGIN_RATIO;
      cd_mr.short_margin_ratio = DEFAULT_STOCK_SHORT_MARGIN_RATIO;
      cd_mr.conversion_rate = DEFAULT_STOCK_CONVERSION_RATE;
    }
    return cd_mr;
  }

  void calculate_collateral_ratio(AssetMargin &asset_margin) {
    double total_debt = asset_margin.cash_debt + asset_margin.short_market_value + asset_margin.margin_interest;
    asset_margin.collateral_ratio =
        total_debt > 0 ? (asset_margin.total_asset + asset_margin.settlement) / total_debt : MAX_COLLATERAL_RATIO;
    // SPDLOG_TRACE("collateral_ratio {} total_asset {} cash_debt {} short_market_value {}  total_debt {}",
    //              asset_margin.collateral_ratio, asset_margin.total_asset, asset_margin.cash_debt,
    //              asset_margin.short_market_value, total_debt);
    asset_margin.collateral_ratio = (std::min)(asset_margin.collateral_ratio, MAX_COLLATERAL_RATIO);
  }

  static double margin_ratio(const Instrument &instrument, const Position &position) {
    return position.direction == Direction::Long ? instrument.long_margin_ratio : instrument.short_margin_ratio;
  }
  static double roundn(double value, int n = AMOUT_PRECISION) {
    double x = pow(10.0, (double)n);
    double round_val = round(value * x) / x;
    return round_val;
  }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_STOCK_H
