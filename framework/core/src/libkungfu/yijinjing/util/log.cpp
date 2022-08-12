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

class emitable_logger : public spdlog::logger {
public:
  emitable_logger(std::string name, spdlog::sink_ptr single_sink)
      : spdlog::logger(std::move(name), std::move(single_sink)) {}

  emitable_logger(std::string name, spdlog::sinks_init_list sinks) : spdlog::logger(std::move(name), sinks) {}

  emitable_logger(const logger &other) : spdlog::logger(other) {}

  std::shared_ptr<logger> clone(std::string logger_name) override {
    auto cloned = std::make_shared<emitable_logger>(*this);
    cloned->name_ = std::move(logger_name);
    return cloned;
  }

  void emit(const std::string &source_file, int &source_line, const std::string &funcname,
            const std::string &logger_name, int log_level, const std::string &msg) {
    spdlog::source_loc source_loc(source_file.c_str(), source_line, funcname.c_str());
    spdlog::details::log_msg record(source_loc, logger_name, static_cast<spdlog::level::level_enum>(log_level), msg);
    sink_it_(record);
  }
};

spdlog::level::level_enum get_env_log_level(const data::locator_ptr &locator) {
  auto level_name = locator->has_env(LOG_LEVEL_ENV) ? locator->get_env(LOG_LEVEL_ENV) : DEFAULT_LOG_LEVEL_NAME;
  return spdlog::level::from_str(level_name);
}

std::shared_ptr<spdlog::logger> get_main_logger() { return spdlog::default_logger(); }

const std::string &setup_log(const data::location_ptr &location, const std::string &name) {
  if (spdlog::default_logger()->name().empty()) {
    std::shared_ptr<emitable_logger> logger;
    std::string log_file = location->locator->layout_file(location, longfist::enums::layout::LOG, name);
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file, 0, 0);

    if (location->group != "node") {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      spdlog::sinks_init_list log_sinks = {console_sink, daily_sink};
      logger = std::make_shared<emitable_logger>(name, log_sinks);
    } else {
      logger = std::make_shared<emitable_logger>(name, daily_sink);
    }

    logger->set_formatter(spdlog::details::make_unique<pattern_formatter>());
    logger->set_level(get_env_log_level(location->locator));
    logger->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(std::static_pointer_cast<spdlog::logger>(logger));
  } else {
    SPDLOG_WARN("Setup log for {} more than once", name);
  }
  return name;
}

void emit_log(const std::string &source_file, int &source_line, const std::string &funcname,
              const std::string &logger_name, int log_level, const std::string &msg) {
  try {
    auto logger = std::dynamic_pointer_cast<emitable_logger>(get_main_logger());
    logger->emit(source_file, source_line, funcname, logger_name, log_level, msg);
  } catch (const std::exception &e) {
    SPDLOG_ERROR("Failed to emit log: {}", e.what());
  }
}
} // namespace kungfu::yijinjing::log
