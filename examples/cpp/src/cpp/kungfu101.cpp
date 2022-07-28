#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/extension.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::strategy;
using namespace kungfu::yijinjing::data;

class KungfuStrategy101 : public Strategy {
public:
  KungfuStrategy101() {}

  void pre_start(Context_ptr &context) override {
    context->add_account("sim", "123456");
    context->subscribe("sim", {"600548"}, {"SSE"});
  };

  void on_quote(Context_ptr &context, const Quote &quote,
                const kungfu::yijinjing::data::location_ptr &location) override {
    std::cout << quote.instrument_id.to_string() << std::endl;
  };
};

KUNGFU_EXTENSION(kungfu101) { KUNGFU_DEFINE_STRATEGY(KungfuStrategy101); }
