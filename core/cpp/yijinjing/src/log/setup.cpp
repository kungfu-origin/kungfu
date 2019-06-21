//
// Created by Keren Dong on 2019-06-06.
//

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/log/setup.h>

namespace kungfu {

    namespace yijinjing {

        namespace log {

            spdlog::level::level_enum get_env_log_level() {
                std::string level_name = DEFAULT_LOG_LEVEL_NAME;
                const char *level_env_name = LOG_LEVEL_ENV;
                char *level_env_value = getenv(level_env_name);
                if (level_env_value != NULL) {
                    level_name = std::string(level_env_value);
                }
                return spdlog::level::from_str(level_name);
            }

            std::shared_ptr<spdlog::logger> get_main_logger() {
                return spdlog::default_logger();
            }

            const std::string& setup_log(data::location_ptr location, const std::string &name) {
                if (spdlog::default_logger()->name().empty()) {
                    auto locator = location->locator;
                    std::string log_file = locator->layout_file(location, data::layout::LOG, name);

                    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file, 0, 0);
                    spdlog::sinks_init_list log_sinks = {console_sink, daily_sink};
                    auto logger = std::make_shared<spdlog::logger>(name, log_sinks);
                    logger->set_pattern(DEFAULT_LOG_PATTERN);
                    spdlog::level::level_enum env_log_level = get_env_log_level();
                    logger->set_level(env_log_level);
                    spdlog::set_default_logger(logger);
                } else {
                    SPDLOG_WARN("Setup log for {} more than once", name);
                }
                return name;
            }
        }
    }
}
