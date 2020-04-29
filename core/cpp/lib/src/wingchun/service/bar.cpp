
#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/service/bar.h>
#include <kungfu/yijinjing/log.h>
#include <regex>

using namespace kungfu::longfist::types;
using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::broker;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::service {
static int64_t parse_time_interval(const std::string &s) {
  std::regex r("[0-9]+");
  std::smatch m;
  std::regex_search(s, m, r);
  if (m.empty()) {
    throw std::runtime_error("invalid time interval: " + s);
  }
  int n = std::stoi(m[0]);
  if (endswith(s, "s")) {
    return n * time_unit::NANOSECONDS_PER_SECOND;
  } else if (endswith(s, "m")) {
    return n * time_unit::NANOSECONDS_PER_MINUTE;
  } else if (endswith(s, "h")) {
    return n * time_unit::NANOSECONDS_PER_HOUR;
  } else if (endswith(s, "d")) {
    return n * time_unit::NANOSECONDS_PER_DAY;
  } else {
    throw std::runtime_error("invalid time_interval: " + s);
  }
}

BarGenerator::BarGenerator(const locator_ptr &locator, mode m, bool low_latency, const std::string &json_config)
    : MarketData(low_latency, locator, "bar"), time_interval_(kungfu::yijinjing::time_unit::NANOSECONDS_PER_MINUTE) {
  log::copy_log_settings(get_home(), "bar");
  auto config = nlohmann::json::parse(json_config);
  auto source = config["source"];
  source_location_ = location::make_shared(mode::LIVE, category::MD, source, source, get_locator());
  if (config.find("time_interval") != config.end()) {
    time_interval_ = parse_time_interval(config["time_interval"]);
  }
}

void BarGenerator::on_start() {
  MarketData::on_start();
  update_broker_state(BrokerState::Ready);

  events_ | is(Register::tag) | $([&](const event_ptr &event) {
    auto register_data = event->data<Register>();
    if (register_data.location_uid == source_location_->uid) {
      request_read_from_public(now(), source_location_->uid, now());
    }
  });

  events_ | is(Quote::tag) | $([&](const event_ptr &event) {
    const auto &quote = event->data<Quote>();
    auto instrument_key = hash_instrument(quote.exchange_id, quote.instrument_id);
    auto pair = bars_.try_emplace(instrument_key);
    auto &bar = pair.first->second;
    if (pair.second) {
      strncpy(bar.instrument_id, quote.instrument_id, INSTRUMENT_ID_LEN);
      strncpy(bar.exchange_id, quote.exchange_id, EXCHANGE_ID_LEN);
      bar.start_time = event->gen_time() - event->gen_time() % time_interval_;
      bar.end_time = bar.start_time + time_interval_;
    }
    if (quote.data_time >= bar.start_time && quote.data_time <= bar.end_time) {
      if (bar.tick_count == 0) {
        bar.high = quote.last_price;
        bar.low = quote.last_price;
        bar.open = quote.last_price;
        bar.close = quote.last_price;
        bar.start_volume = quote.volume;
      }
      bar.tick_count++;
      bar.volume = quote.volume - bar.start_volume;
      bar.high = std::max(bar.high, quote.last_price);
      bar.low = std::min(bar.low, quote.last_price);
      bar.close = quote.last_price;
    }
    if (quote.data_time >= bar.end_time) {
      get_writer(location::PUBLIC)->write(event->gen_time(), Bar::tag, bar);
      bar.start_time = bar.end_time;
      while (bar.start_time + time_interval_ < quote.data_time) {
        bar.start_time += time_interval_;
      }
      bar.end_time = bar.start_time + time_interval_;
      if (bar.start_time <= quote.data_time) {
        bar.tick_count = 1;
        bar.start_volume = quote.volume;
        bar.volume = 0;
        bar.high = quote.last_price;
        bar.low = quote.last_price;
        bar.open = quote.last_price;
        bar.close = quote.last_price;
      } else {
        bar.tick_count = 0;
        bar.start_volume = 0;
        bar.volume = 0;
        bar.high = 0;
        bar.low = 0;
        bar.open = 0;
        bar.close = 0;
      }
    }
  });
}

bool BarGenerator::subscribe(const std::vector<InstrumentKey> &instrument_keys) { return false; }

bool BarGenerator::unsubscribe(const std::vector<InstrumentKey> &instrument_keys) { return false; }

bool BarGenerator::subscribe_all() { return false; }

} // namespace kungfu::wingchun::service
