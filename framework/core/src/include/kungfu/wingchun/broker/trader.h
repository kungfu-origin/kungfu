//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_TRADER_H
#define WINGCHUN_TRADER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {

FORWARD_DECLARE_PTR(Trader)

class TraderVendor : public BrokerVendor {
public:
  TraderVendor(locator_ptr locator, const std::string &group, const std::string &name, bool low_latency);

  void setup(Trader_ptr service);

protected:
  void on_start() override;

  BrokerService_ptr get_service() override;

private:
  Trader_ptr service_ = {};

  void clean_orders();
};

class Trader : public BrokerService {
  friend class TraderVendor;

public:
  typedef std::unordered_map<uint64_t, state<longfist::types::Order>> OrderMap;
  typedef std::unordered_map<uint64_t, state<longfist::types::OrderAction>> OrderActionMap;
  typedef std::unordered_map<uint64_t, state<longfist::types::Trade>> TradeMap;

  explicit Trader(BrokerVendor &vendor) : BrokerService(vendor){};

  [[nodiscard]] virtual longfist::enums::AccountType get_account_type() const = 0;

  virtual bool insert_order(const event_ptr &event) = 0;

  virtual bool cancel_order(const event_ptr &event) = 0;

  virtual bool req_position() = 0;

  virtual bool req_account() = 0;

  [[nodiscard]] const std::string &get_account_id() const;

protected:
  OrderMap orders_ = {};
  OrderActionMap actions_ = {};
  TradeMap trades_ = {};

  bool update_asset_ = false;
  bool update_position_ = false;

  [[nodiscard]] yijinjing::journal::writer_ptr get_asset_writer() const;

  [[nodiscard]] yijinjing::journal::writer_ptr get_position_writer() const;

  void enable_asset_sync();

  void enable_positions_sync();
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_TRADER_H
