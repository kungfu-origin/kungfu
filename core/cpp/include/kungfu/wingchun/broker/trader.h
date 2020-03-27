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
class Trader : public Broker {
public:
  explicit Trader(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source,
                  const std::string &account_id);

  ~Trader() override = default;

  void on_start() override;

  const std::string &get_account_id() const { return account_id_; }

  virtual longfist::enums::AccountType get_account_type() const = 0;

  virtual bool insert_order(const event_ptr &event) = 0;

  virtual bool cancel_order(const event_ptr &event) = 0;

  virtual bool req_position() = 0;

  virtual bool req_account() = 0;

protected:
  std::unordered_map<uint64_t, state<longfist::types::Order>> orders_ = {};
  std::unordered_map<uint64_t, state<longfist::types::OrderAction>> actions_ = {};
  std::unordered_map<uint64_t, state<longfist::types::Trade>> trades_ = {};

private:
  std::string source_;
  std::string account_id_;
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_TRADER_H
