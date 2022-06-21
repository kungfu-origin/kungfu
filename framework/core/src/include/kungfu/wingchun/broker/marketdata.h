//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_MARKETDATA_H
#define WINGCHUN_MARKETDATA_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {

FORWARD_DECLARE_CLASS_PTR(MarketData)

class MarketDataVendor : public BrokerVendor {
public:
  MarketDataVendor(locator_ptr locator, const std::string &group, const std::string &name, bool low_latency);

  void set_service(MarketData_ptr service);

protected:
  void on_react() override;

  void on_start() override;

  BrokerService_ptr get_service() override;

private:
  MarketData_ptr service_ = {};
};

class MarketData : public BrokerService {
  friend class MarketDataVendor;

public:
  explicit MarketData(BrokerVendor &vendor) : BrokerService(vendor){};

  virtual bool subscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) = 0;

  virtual bool subscribe_all() = 0;

  virtual bool subscribe_custom(const longfist::types::SubscribeAll &subscribeall) { return subscribe_all(); };

  virtual bool unsubscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) = 0;

protected:
  [[nodiscard]] bool has_instrument(const std::string &instrument_id) const;

  [[nodiscard]] const longfist::types::Instrument &get_instrument(const std::string &instrument_id) const;

  void update_instrument(longfist::types::Instrument instrument);

  std::unordered_map<std::string, longfist::types::Instrument> instruments_ = {};
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_MARKETDATA_H
