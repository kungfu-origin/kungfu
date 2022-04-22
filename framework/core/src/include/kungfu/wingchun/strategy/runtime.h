//
// Created by Keren Dong on 2020/7/20.
//

#ifndef WINGCHUN_RUNTIME_H
#define WINGCHUN_RUNTIME_H

#include <kungfu/wingchun/strategy/context.h>

namespace kungfu::wingchun::strategy {
class RuntimeContext : public Context {
public:
  explicit RuntimeContext(yijinjing::practice::apprentice &app, const rx::connectable_observable<event_ptr> &events);

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
  void subscribe_all(const std::string &source, uint8_t exchanges_ids = 0,
                             uint64_t instrument_types = 0,
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
   *
   * @param instrument_id instrument ID
   * @param exchange_id exchange ID
   * @param source source ID
   * @param account account ID
   * @param limit_price limit price
   * @param volume trade volume
   * @param type price type
   * @param side side
   * @param offset offset, defaults to longfist::enums::Offset::Open
   * @param hedge_flag hedge_flag, defaults to longfist::enums::HedgeFlag::Speculation
   * @return
   */
  uint64_t make_order(const std::string &instrument_id, const std::string &exchange_id, const std::string &source,
                      const std::string &account, double limit_price, int64_t volume, longfist::enums::PriceType type,
                      longfist::enums::Side side, longfist::enums::Offset offset,
                      longfist::enums::HedgeFlag hedge_flag) override;

  /**
   *
   * @param account_location_uid location uid of source ID and account ID
   * @param instrument_id instrument
   * @param exchange_id exchange ID
   * @param account account ID
   * @param limit_price limit price
   * @param volume trade volume
   * @param type price type
   * @param side side
   * @param offset offset, defaults to longfist::enums::Offset::Open
   * @param hedge_flag hedge_flag, defaults to longfist::enums::HedgeFlag::Speculation
   * @return
   */
  uint64_t insert_order(uint32_t account_location_uid, const std::string &instrument_id, const std::string &exchange_id,
                        const std::string &account, double limit_price, int64_t volume, longfist::enums::PriceType type,
                        longfist::enums::Side side, longfist::enums::Offset offset,
                        longfist::enums::HedgeFlag hedge_flag);

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

  /**
   * Get subscribed MD locations.
   * @return subscribed MD locations
   */
  const yijinjing::data::location_map &list_md() const;

  /**
   * Get enrolled TD locations.
   * @return enrolled TD locations
   */
  const yijinjing::data::location_map &list_accounts() const;

  /**
   * Get broker client.
   * @return broker client reference
   */
  broker::Client &get_broker_client();

  /**
   * Get bookkeeper.
   * @return bookkeeper reference
   */
  book::Bookkeeper &get_bookkeeper();

  /**
   * query history order
   */
  void req_history_order(const std::string &source, const std::string &account) override;

  /**
   * query history trade
   */
  void req_history_trade(const std::string &source, const std::string &account) override;

  /**
   * request deregister.
   * @return void
   */
  void req_deregister() override;

  /**
   * Update Strategy State
   * @param state StrategyState
   * @param infos vector<string>, info_a, info_b, info_c.
   */
  void update_strategy_state(longfist::types::StrategyStateUpdate &state_update) override;

protected:
  yijinjing::practice::apprentice &app_;
  const rx::connectable_observable<event_ptr> &events_;

  virtual void on_start();

  uint32_t lookup_account_location_id(const std::string &account) const;

  uint32_t lookup_source_account_location_id(const std::string &source, const std::string &account) const;

  const yijinjing::data::location_ptr &find_md_location(const std::string &source);

private:
  broker::PassiveClient broker_client_;
  book::Bookkeeper bookkeeper_;
  yijinjing::data::location_map md_locations_ = {};
  yijinjing::data::location_map td_locations_ = {};
  std::unordered_map<uint32_t, uint32_t> account_location_ids_ = {};
  std::unordered_map<uint32_t, uint32_t> source_account_location_ids_ = {};
  std::unordered_map<uint32_t, double> account_cash_limits_ = {};
  std::unordered_map<std::string, yijinjing::data::location_ptr> market_data_ = {};

  friend void enable(RuntimeContext &context) { context.on_start(); }
};

DECLARE_PTR(RuntimeContext)
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_RUNTIME_H
