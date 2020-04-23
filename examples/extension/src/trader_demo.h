#ifndef TD_KFEXT_DEMO_H
#define TD_KFEXT_DEMO_H

#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/yijinjing/common.h>

namespace kungfu::wingchun::kfext_demo {
class TraderDemo : public broker::Trader {
public:
  TraderDemo(bool low_latency, yijinjing::data::locator_ptr locator,
             std::map<std::string, std::string> &config_str,
             std::map<std::string, int> &config_int,
             std::map<std::string, double> &config_double);

  longfist::enums::AccountType get_account_type() const override {
    return longfist::enums::AccountType::Stock;
  }

  void on_start() override;

  bool insert_order(const event_ptr &event) override;

  bool cancel_order(const event_ptr &event) override;

  bool req_position() override;

  bool req_account() override;
};
} // namespace kungfu::wingchun::kfext_demo
#endif // TD_KFEXT_DEMO_H
