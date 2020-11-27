#ifndef KF_SERVICE_BAR_GENERATOR
#define KF_SERVICE_BAR_GENERATOR

#include <unordered_set>

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/marketdata.h>

namespace kungfu::wingchun::service {
class BarGenerator : public broker::MarketData {
public:
  BarGenerator(const yijinjing::data::locator_ptr &locator, longfist::enums::mode m, bool low_latency,
               const std::string &json_config);

  void on_start() override;

  bool subscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) override;

  bool unsubscribe(const std::vector<longfist::types::InstrumentKey> &instrument_keys) override;

  bool subscribe_all() override;

private:
  int64_t time_interval_;
  yijinjing::data::location_ptr source_location_;
  std::unordered_map<uint32_t, longfist::types::Bar> bars_;
};
} // namespace kungfu::wingchun::service

#endif // KF_SERVICE_BAR_GENERATOR
