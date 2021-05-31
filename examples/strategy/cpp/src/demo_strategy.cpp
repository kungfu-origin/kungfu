#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/strategy.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::strategy;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::log;

class DemoStrategy : public Strategy {
public:
  DemoStrategy(location_ptr home) {
  };

  void pre_start(Context_ptr &context) override {
    printf("cpp demo pre start");
  };

  void on_quote(Context_ptr &context, const Quote &quote) override {
    printf("cpp demo on quote");
  };
};

int main() {
  printf("kungfu demo");
}