//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>

#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/strategy/strategy.h>

namespace kungfu::wingchun::strategy {
class Context : public std::enable_shared_from_this<Context> {
public:
  explicit Context(yijinjing::practice::apprentice &app, const rx::connectable_observable<event_ptr> &events);

  virtual ~Context() = default;

  /**
   * Get current time in nano seconds.
   * @return current time in nano seconds
   */
  virtual int64_t now() const;

  /**
   * Add one shot timer callback.
   * @param nanotime when to call in nano seconds
   * @param callback callback function
   */
  virtual void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback);

  /**
   * Add periodically callback.
   * @param duration duration in nano seconds
   * @param callback callback function
   */
  virtual void add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback);

  /**
   * Add account for strategy.
   * @param source TD group
   * @param account TD account ID
   * @param cash_limit cash limit
   */
  virtual void add_account(const std::string &source, const std::string &account, double cash_limit);

  /**
   * Subscribe market data.
   * @param source MD group
   * @param instrument_ids instrument IDs
   * @param exchange_ids exchange IDs
   */
  virtual void subscribe(const std::string &source, const std::vector<std::string> &instrument_ids,
                         const std::string &exchange_ids);

  /**
   * Subscribe all from given MD
   * @param source MD group
   */
  virtual void subscribe_all(const std::string &source);

  /**
   * Insert order.
   * @param instrument_id instrument ID
   * @param exchange_id exchange ID
   * @param account account ID
   * @param volume trade volume
   * @param type price type
   * @param side side
   * @param offset defaults to longfist::enums::Offset::Open
   * @param hedge_flag defaults to longfist::enums::HedgeFlag::Speculation
   * @return inserted order ID
   */
  virtual uint64_t insert_order(const std::string &instrument_id, const std::string &exchange_id,
                                const std::string &account, double limit_price, int64_t volume,
                                longfist::enums::PriceType type, longfist::enums::Side side,
                                longfist::enums::Offset offset, longfist::enums::HedgeFlag hedge_flag);

  /**
   * Cancel order.
   * @param order_id order ID
   * @return order action ID
   */
  virtual uint64_t cancel_order(uint64_t order_id);

  /**
   * Tells whether the book is held.
   * If book is held, all traded positions will be recovered from ledger.
   * If book is not held, ledger will use the information collected in pre_start to build a fresh book.
   * @return true if book is held, false otherwise. Defaults to false.
   */
  [[nodiscard]] bool is_book_held() const;

  /**
   * Tells whether to mirror positions from enrolled accounts.
   * If positions are mirrored, will always have positions equal to the corresponding positions under accounts.
   * If positions are not mirrored, will set positions to 0 for newly added strategies, or the history values recorded
   * by kungfu.
   * @return true if positions are mirrored, false otherwise. Defaults to true.
   */
  [[nodiscard]] bool is_positions_mirrored() const;

  /**
   * Call to hold book.
   */
  void hold_book();

  /**
   * Call to hold positions, i.e. do not mirror positions.
   */
  void hold_positions();

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
   * Get cash limit for given account
   * @param account account ID
   * @return cash limit
   */
  double get_account_cash_limit(const std::string &account) const;

  /**
   * Get current trading day.
   * @return current trading day
   */
  int64_t get_trading_day() const;

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

protected:
  yijinjing::practice::apprentice &app_;
  const rx::connectable_observable<event_ptr> &events_;

  virtual void on_start();

  uint32_t lookup_account_location_id(const std::string &account) const;

  const yijinjing::data::location_ptr &find_md_location(const std::string &source);

private:
  bool book_held_ = false;
  bool positions_mirrored_ = true;
  broker::ManualClient broker_client_;
  book::Bookkeeper bookkeeper_;
  yijinjing::data::location_map md_locations_ = {};
  yijinjing::data::location_map td_locations_ = {};
  std::unordered_map<uint32_t, uint32_t> account_location_ids_ = {};
  std::unordered_map<uint32_t, double> account_cash_limits_ = {};
  std::unordered_map<std::string, yijinjing::data::location_ptr> market_data_ = {};

  friend void enable(Context &context) { context.on_start(); }
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_CONTEXT_H
