//
// Created by qianyong liu on 2021/7/27.
//

#ifndef WINGCHUN_ACCOUNTING_CRYPTO_H
#define WINGCHUN_ACCOUNTING_CRYPTO_H

#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book {
class CryptoAccountingMethod : public AccountingMethod {
public:
  static constexpr double min_comission = 5;
  static constexpr double commission_ratio = 0.0008;

  void get_instrument(const Book_ptr &book, const Trade &trade, char *instrument_a, char *instrument_b,
                      char *instrument_commission, int64_t &volume_a, int64_t &volume_b, int64_t &volume_commission) {
    auto instrument_hash = hash_instrument(trade.exchange_id, trade.instrument_id);
    auto &instrument = book->instruments.at(instrument_hash);
    strncpy(instrument_a, instrument.instrument_id, strlen(instrument.instrument_id) - instrument.delivery_year);
    strcpy(instrument_b, &(instrument.instrument_id[strlen(instrument.instrument_id) - instrument.delivery_year]));
    if (trade.hedge_flag == HedgeFlag::Speculation)
      strcpy(instrument_commission, "BNB");
    else if (trade.hedge_flag == HedgeFlag::Arbitrage)
      strcpy(instrument_commission, instrument_b);
    else
      strcpy(instrument_commission, instrument_a);
    volume_a = trade.volume;
    volume_b = trade.volume * trade.price;
    volume_commission = trade.commission;
  }

  CryptoAccountingMethod() = default;

  virtual void apply_trading_day(Book_ptr &book, int64_t trading_day) override {}

  virtual void apply_quote(Book_ptr &book, const Quote &quote) override {}

  virtual void apply_order_input(Book_ptr &book, const OrderInput &input) override {}

  virtual void apply_order(Book_ptr &book, const Order &order) override {}

  virtual void apply_trade(Book_ptr &book, const Trade &trade) override {
    kungfu::array<char, INSTRUMENT_ID_LEN> instrument_a;
    kungfu::array<char, INSTRUMENT_ID_LEN> instrument_b;
    kungfu::array<char, INSTRUMENT_ID_LEN> instrument_commission;
    int64_t volume_a = 0;
    int64_t volume_b = 0;
    int64_t volume_commission = 0;
    get_instrument(book, trade, instrument_a, instrument_b, instrument_commission, volume_a, volume_b,
                   volume_commission);
    auto &position_a = book->get_long_position(trade.exchange_id, instrument_a);
    auto &position_b = book->get_long_position(trade.exchange_id, instrument_b);
    auto &position_commission = book->get_long_position(trade.exchange_id, instrument_commission);
    if (trade.side == Side::Buy) {
      position_a.volume += volume_a;
      position_b.volume -= volume_b;
    }
    if (trade.side == Side::Sell) {
      position_a.volume -= volume_a;
      position_b.volume += volume_b;
    }
    position_commission.volume -= volume_commission;
  }

  virtual void update_position(Book_ptr &book, Position &position) override {
    auto position_in = book->get_long_position(position.instrument_id, position.exchange_id);
    position_in.volume = position.volume;
    position_in.frozen_total = position.frozen_total;
  }

protected:
  std::unordered_map<uint64_t, double> commission_map_ = {};

  virtual void apply_buy(Book_ptr &book, const Trade &trade) {}

  virtual void apply_sell(Book_ptr &book, const Trade &trade) {}

  virtual double calculate_commission(const Trade &trade) { return 0; }

  virtual double calculate_tax(const Trade &trade) { return 0; }
};
} // namespace kungfu::wingchun::book
#endif // WINGCHUN_ACCOUNTING_CRYPTO_H