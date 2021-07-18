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
class MarketData : public Broker {
public:
  explicit MarketData(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source);

  ~MarketData() override = default;

  void on_start() override;

  virtual bool subscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) = 0;

  virtual bool subscribe_all() = 0;

  virtual bool unsubscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) = 0;

protected:
  void on_react() override;

  bool has_instrument(const std::string &instrument_id) const;

  const longfist::types::Instrument &get_instrument(const std::string &instrument_id) const;

  std::unordered_map<std::string, longfist::types::Instrument> instruments_ = {};

private:
  void update_instrument(longfist::types::Instrument instrument);
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_MARKETDATA_H
