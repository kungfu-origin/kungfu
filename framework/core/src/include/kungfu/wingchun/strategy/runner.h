//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_RUNNER_H
#define WINGCHUN_RUNNER_H

#include <kungfu/wingchun/strategy/runtime.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::strategy {
class Runner : public yijinjing::practice::apprentice, public kungfu::wingchun::book::BookListener {
public:
  Runner(yijinjing::data::locator_ptr locator, const std::string &group, const std::string &name,
         longfist::enums::mode m, bool low_latency);

  ~Runner() override = default;

  [[nodiscard]] RuntimeContext_ptr get_context() const;

  void add_strategy(const Strategy_ptr &strategy);

  void on_exit() override;

  void on_trading_day(const event_ptr &event, int64_t daytime) override;

  void on_book_sync_reset(const book::Book &old_book, const book::Book &new_book) override;

  void on_asset_sync_reset(const longfist::types::Asset &old_asset, const longfist::types::Asset &new_asset) override;

protected:
  void on_react() override;

  void on_start() override;

  void on_active() override;

  virtual RuntimeContext_ptr make_context();

  virtual void pre_start();

  virtual void post_start();

  virtual void pre_stop();

  virtual void post_stop();

private:
  yijinjing::data::location ledger_location_;
  bool positions_requested_ = false;
  bool positions_set_;
  bool started_;
  std::vector<Strategy_ptr> strategies_ = {};
  RuntimeContext_ptr context_;

  void prepare(const event_ptr &event);
  void inspect_channel(const event_ptr &event);

  template <typename OnMethod = void (Strategy::*)(Context_ptr &)> void invoke(OnMethod method) {
    auto context = std::dynamic_pointer_cast<Context>(context_);
    for (const auto &strategy : strategies_) {
      (*strategy.*method)(context);
    }
  }

  template <typename TradingData, typename OnMethod = void (Strategy::*)(Context_ptr &, const TradingData &)>
  void invoke(OnMethod method, const TradingData &data) {
    auto context = std::dynamic_pointer_cast<Context>(context_);
    for (const auto &strategy : strategies_) {
      (*strategy.*method)(context, data);
    }
  }
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_RUNNER_H
