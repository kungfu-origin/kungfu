#include <kungfu/wingchun/extension.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runtime.h>
#include <kungfu/wingchun/strategy/strategy.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::strategy;
using namespace kungfu::yijinjing::data;

KUNGFU_MAIN_STRATEGY(KungfuStrategy101) {
public:
  KungfuStrategy101() = default;
  ~KungfuStrategy101() = default;

  void pre_start(Context_ptr & context) override {
    SPDLOG_INFO("preparing strategy");
    SPDLOG_INFO("arguments: {}", context->arguments());
    context->add_account("sim", "123456");
    context->subscribe("sim", {"600000"}, {"SSE"});
  }

  void post_start(Context_ptr & context) override {
    SPDLOG_INFO("strategy started");
    auto &runtime = dynamic_cast<RuntimeContext &>(*context);
    auto &bookkeeper = runtime.get_bookkeeper();
    auto &books = bookkeeper.get_books();
    for (const auto &pair : books) {
      auto &book = pair.second;
      SPDLOG_INFO("book asset: {}", book->asset.to_string());
    }
  }

  void on_quote(Context_ptr & context, const Quote &quote, const location_ptr &location) override {
    SPDLOG_INFO("on quote: {}", quote.to_string());
  }
};
