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

  virtual ~Runner() = default;

  Context_ptr get_context() const;

  void add_strategy(const Strategy_ptr &strategy);

  void on_trading_day(const event_ptr &event, int64_t daytime) override;

protected:
  std::vector<Strategy_ptr> strategies_;

  void on_react() override;

  void on_start() override;

  void on_exit() override;

  virtual Context_ptr make_context();

  virtual void pre_start();

  virtual void post_start();

private:
  bool started_ = false;
  bool position_set_;
  Context_ptr context_;

  template <typename DataType>
  static constexpr auto is_own = [](const Context_ptr &context) {
    return rx::filter([&](const event_ptr &event) {
      return event->msg_type() == DataType::tag and context->is_subscribed<DataType>(event);
    });
  };
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_RUNNER_H
