//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_RUNNER_H
#define WINGCHUN_RUNNER_H

#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::strategy {
class Runner : public yijinjing::practice::apprentice {
public:
  Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name,
         longfist::enums::mode m, bool low_latency);

  ~Runner() override = default;

  Context_ptr get_context() const;

  void add_strategy(const Strategy_ptr &strategy);

  void on_trading_day(const event_ptr &event, int64_t daytime) override;

protected:
  void on_react() override;

  void on_start() override;

  void on_active() override;

  void on_exit() override;

  virtual Context_ptr make_context();

  virtual void pre_start();

  virtual void post_start();

  virtual void pre_stop();

  virtual void post_stop();

private:
  yijinjing::data::location ledger_location_;
  bool positions_requested_ = false;
  bool positions_set_ = false;
  bool started_ = false;
  std::vector<Strategy_ptr> strategies_ = {};
  Context_ptr context_;

  void prepare(const event_ptr &event);

  template <typename OnMethod = void (Strategy::*)(Context_ptr &)>
  void invoke(OnMethod method) {
    for (const auto &strategy : strategies_) {
      (*strategy.*method)(context_);
    }
  }

  template <typename TradingData, typename OnMethod = void (Strategy::*)(Context_ptr &, const TradingData &)>
  void invoke(OnMethod method, const TradingData &data) {
    for (const auto &strategy : strategies_) {
      (*strategy.*method)(context_, data);
    }
  }

  template <typename DataType>
  static constexpr auto is_own = [](const Context_ptr &context) {
    return rx::filter([&](const event_ptr &event) {
      const DataType &data = event->data<DataType>();
      return event->msg_type() == DataType::tag and
             context->get_broker_client().is_subscribed(event->source(), data.exchange_id, data.instrument_id);
    });
  };
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_RUNNER_H
