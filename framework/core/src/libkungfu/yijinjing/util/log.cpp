//
// Created by Keren Dong on 2019-06-06.
//

#include <kungfu/common.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/time.h>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace kungfu::yijinjing::log {
class pattern_formatter : public spdlog::formatter {
public:
  pattern_formatter() : spdlog_formatter(LOG_PATTERN) {}

  pattern_formatter(const pattern_formatter &other) = delete;
  pattern_formatter &operator=(const pattern_formatter &other) = delete;

  [[nodiscard]] std::unique_ptr<formatter> clone() const override {
    return spdlog::details::make_unique<pattern_formatter>();
  }

  void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override {
    spdlog::details::fmt_helper::append_string_view(time::strftime(time::now_in_nano(), TS_PATTERN), dest);
    spdlog_formatter.format(msg, dest);
  }

private:
  spdlog::pattern_formatter spdlog_formatter;
};

spdlog::level::level_enum get_env_log_level(const data::locator_ptr &locator) {
  auto level_name = locator->has_env(LOG_LEVEL_ENV) ? locator->get_env(LOG_LEVEL_ENV) : DEFAULT_LOG_LEVEL_NAME;
  return spdlog::level::from_str(level_name);
}

std::shared_ptr<spdlog::logger> get_main_logger() { return spdlog::default_logger(); }

const std::string &setup_log(const data::location_ptr &location, const std::string &name) {
  if (spdlog::default_logger()->name().empty()) {
    std::shared_ptr<spdlog::logger> logger;
    std::string log_file = location->locator->layout_file(location, longfist::enums::layout::LOG, name);
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file, 0, 0);

    if (location->group != "node") {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      spdlog::sinks_init_list log_sinks = {console_sink, daily_sink};
      logger = std::make_shared<spdlog::logger>(name, log_sinks);
    } else {
      logger = std::make_shared<spdlog::logger>(name, daily_sink);
    }

    logger->set_formatter(spdlog::details::make_unique<pattern_formatter>());
    logger->set_level(get_env_log_level(location->locator));
    logger->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(logger);
  } else {
    SPDLOG_WARN("Setup log for {} more than once", name);
  }
  return name;
}
} // namespace kungfu::yijinjing::log
