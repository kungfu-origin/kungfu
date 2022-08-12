//
// Created by Keren Dong on 2019-06-06.
//

#ifndef KUNGFU_LOG_H
#define KUNGFU_LOG_H

#include <kungfu/common.h>
#include <kungfu/yijinjing/common.h>

#define LOG_LEVEL_ENV "KF_LOG_LEVEL"
#define DEFAULT_LOG_LEVEL_NAME "info"
#define TS_PATTERN "[%m/%d %H:%M:%S.%N] "
#define LOG_PATTERN "[%^%=8l%$] [%6P/%-6t] [%s:%##%!] %v"

namespace kungfu::yijinjing::log {

std::shared_ptr<spdlog::logger> get_main_logger();

spdlog::level::level_enum get_env_log_level(const data::locator_ptr &locator);

const std::string &setup_log(const data::location_ptr &location, const std::string &name);

void emit_log(const std::string &source_file, int &source_line, const std::string &funcname,
              const std::string &logger_name, int log_level, const std::string &msg);

inline void copy_log_settings(const data::location_ptr &location, const std::string &name) {
  if (get_main_logger()->name().empty()) {
    setup_log(location, name);
  }
  auto logger_cloned = get_main_logger()->clone(name);
  spdlog::set_default_logger(logger_cloned);
}
} // namespace kungfu::yijinjing::log

#endif // KUNGFU_LOG_H
