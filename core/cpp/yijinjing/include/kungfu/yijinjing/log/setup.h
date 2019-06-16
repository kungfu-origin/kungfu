//
// Created by Keren Dong on 2019-06-06.
//

#ifndef KUNGFU_LOG_H
#define KUNGFU_LOG_H

#include <spdlog/spdlog.h>
#include <kungfu/yijinjing/common.h>

#define LOG_LEVEL_ENV "KF_LOG_LEVEL"
#define DEFAULT_LOG_LEVEL_NAME "info"
#define DEFAULT_LOG_PATTERN "[%Y-%m-%d %T.%F] [%^%=8l%$] [pid/tid %6P/%-6t] [%@#%!] %v"

namespace kungfu {

    namespace yijinjing {

        namespace log {
            spdlog::level::level_enum get_env_log_level();

            const std::string& setup_log(data::location_ptr location, const std::string &name);

            std::shared_ptr<spdlog::logger> get_main_logger();

            inline void copy_log_settings(data::location_ptr location, const std::string &name) {
                if(get_main_logger()->name().empty())
                {
                    setup_log(location, name);
                }
                auto logger_cloned = get_main_logger()->clone(name);
                spdlog::set_default_logger(logger_cloned);
            }
        }
    }
}

#endif //KUNGFU_LOG_H
