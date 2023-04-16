// #include <kungfu/wingchun/extension.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/wingchun/strategy/runtime.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/yijinjing/journal/assemble.h>

using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::strategy;
using namespace kungfu::yijinjing::data;
int i = 0;
// KUNGFU_MAIN_STRATEGY(KungfuStrategy101) {
class KungfuStrategy101 : public Strategy {
public:
  KungfuStrategy101() = default;
  ~KungfuStrategy101() = default;

  void pre_start(Context_ptr &context) override {
    SPDLOG_INFO("preparing strategy");
    SPDLOG_INFO("arguments: {}", context->arguments());
    context->subscribe("sim", {"600000"}, {"SSE"});
    // context->subscribe_operator("bar", "my-bar");
  }

  void post_start(Context_ptr &context) override {
    SPDLOG_INFO("strategy started");
    // auto &runtime = dynamic_cast<RuntimeContext &>(*context);
    // auto &bookkeeper = runtime.get_bookkeeper();
    // auto &books = bookkeeper.get_books();
    // for (const auto &pair : books) {
    //   auto &book = pair.second;
    //   SPDLOG_INFO("book asset: {}", book->asset.to_string());
    // }
    auto l_ptr = location::make_shared(mode::LIVE, category::MD, "sim", "sim", {});
    kungfu::yijinjing::journal::assemble asb(l_ptr, location::PUBLIC, AssembleMode::All);
    auto headers = asb.read_headers(Location{});
    for (const auto &head : headers) {
      SPDLOG_INFO("head: {}", head.to_string());
    }
    kungfu::yijinjing::journal::assemble asb2(l_ptr, location::PUBLIC, AssembleMode::All);
    auto locations = asb2.read_bytes<Location>();
    SPDLOG_INFO("locations.length: {}", locations.size());
    for (const auto &loc : locations) {
      SPDLOG_INFO("locaton byte: {}", std::string(loc.second.begin(), loc.second.end()));
    }
    kungfu::yijinjing::journal::assemble asb3(l_ptr, location::PUBLIC, AssembleMode::All);
    auto l3 = asb3.read_all<Location>();
    SPDLOG_INFO("locations.length: {}", l3.size());
    for (const auto &loc : l3) {
      SPDLOG_INFO("l3 : {}", loc.to_string());
    }
  }

  void on_quote(Context_ptr &context, const Quote &quote, const location_ptr &location) override {
    SPDLOG_INFO("on quote: {} i {} location->uid {}", quote.last_price, i, location->location_uid);
    // i++;
    // if (i == 5) {
    //   std::shared_ptr<kungfu::yijinjing::journal::assemble> p_assemble =
    //       std::make_shared<kungfu::yijinjing::journal::assemble>(std::vector<locator_ptr>{});
    //   std::shared_ptr<kungfu::yijinjing::journal::frame_reader> r = p_assemble->get_reader(location);
    //   auto f = r->current_frame();
    //   SPDLOG_INFO("f source {} dest {} data {}", f->source(), f->dest(), f->data_as_string());
    //   while (true) {
    //     auto f = r->next_frame();
    //     if (!f) {
    //       SPDLOG_INFO("f null");
    //       break;
    //     }
    //     SPDLOG_INFO("f source {} dest {} data {}", f->source(), f->dest(), f->data_as_string());
    //   }
    // }
  }

  void on_broker_state_change(Context_ptr &context, const BrokerStateUpdate &broker_state_update,
                              const location_ptr &location) override {
    SPDLOG_INFO("on broker state changed: {}", broker_state_update.to_string());
  };

  void on_tree(Context_ptr &context, const Tree &tree, const location_ptr &location) override {
    SPDLOG_INFO("on tree: {}", tree.to_string());
  }
};

int main(int argc, char **argv) {
  SPDLOG_INFO("runner1 add strategy1");
  Runner runner(std::make_shared<locator>(), "CppStrategy", "demo01exe", mode::LIVE, false);
  SPDLOG_INFO("runner");
  runner.add_strategy(std::make_shared<KungfuStrategy101>());
  SPDLOG_INFO("add_strategy");
  SPDLOG_INFO("runner1 add strategy1");
  runner.run();
  return 0;
}