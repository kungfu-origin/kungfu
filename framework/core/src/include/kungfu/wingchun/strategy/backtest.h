//
// Created by Keren Dong on 2020/7/20.
//

#ifndef WINGCHUN_BACKTEST_H
#define WINGCHUN_BACKTEST_H

#include <kungfu/wingchun/strategy/context.h>

namespace kungfu::wingchun::strategy {
class BacktestContext : public Context {
public:
  BacktestContext() = default;
  ~BacktestContext() = default;

  /**
   * Get current time in nano seconds.
   * @return current time in nano seconds
   */
  int64_t now() const override;

  /**
   * Add one shot timer callback.
   * @param nanotime when to call in nano seconds
   * @param callback callback function
   */
  void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback) override;

  /**
   * Add periodically callback.
   * @param duration duration in nano seconds
   * @param callback callback function
   */
  void add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback) override;

  /**
   * Add account for strategy.
   * @param source TD group
   * @param account TD account ID
   * @param cash_limit cash limit
   */
  void add_account(const std::string &source, const std::string &account, double cash_limit) override;

  /**
   * Subscribe market data.
   * @param source MD group
   * @param instrument_ids instrument IDs
   * @param exchange_ids exchange IDs
   */
  void subscribe(const std::string &source, const std::vector<std::string> &instrument_ids,
                 const std::string &exchange_ids) override;

  /**
   * Subscribe all from given MD
   * @param source MD group
   */
  void subscribe_all(const std::string &source, uint8_t exchanges_ids = 0, uint64_t instrument_types = 0,
                     uint64_t callback_types = 0) override;

  /**
   * Insert order.
   * @param instrument_id instrument ID
   * @param exchange_id exchange ID
   * @param account account ID
   * @param volume trade volume
   * @param type price type
   * @param side side
   * @param offset Deprecated, defaults to longfist::enums::Offset::Open
   * @param hedge_flag Deprecated, defaults to longfist::enums::HedgeFlag::Speculation
   * @return inserted order ID
   */
  uint64_t insert_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &account,
                        double limit_price, int64_t volume, longfist::enums::PriceType type, longfist::enums::Side side,
                        longfist::enums::Offset offset, longfist::enums::HedgeFlag hedge_flag) override;

  /**
   * Cancel order.
   * @param order_id order ID
   * @return order action ID
   */
  uint64_t cancel_order(uint64_t order_id) override;

  /**
   * Get cash limit for given account
   * @param account account ID
   * @return cash limit
   */
  double get_account_cash_limit(const std::string &account) const override;

  /**
   * Get current trading day.
   * @return current trading day
   */
  int64_t get_trading_day() const override;
};

DECLARE_PTR(BacktestContext)
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_BACKTEST_H
