#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/wingchun/strategy/strategy.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::strategy;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::log;

class DemoStrategy : public Strategy {
public:
  DemoStrategy() = default;

  void pre_start(Context_ptr &context) override {
    context->add_account("xtp", "15014990", 0);
    context->subscribe("xtp", {"600548"}, {"SSE"});
  };

  void on_quote(Context_ptr &context, const Quote &quote,
                const kungfu::yijinjing::data::location_ptr &location) override {
    std::cout << quote.instrument_id.to_string() << std::endl;
  };
};

int main() {
  Runner runner(std::make_shared<locator>(), "default", "testcpp", mode::LIVE,
                false);
  runner.add_strategy(std::make_shared<DemoStrategy>());
  runner.run();
  return 0;
}