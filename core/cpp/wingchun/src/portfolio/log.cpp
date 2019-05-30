//
// Created by qlu on 2019/5/16.
//

#include <kungfu/wingchun/portfolio/pnl_def.h>

namespace kungfu
{
    namespace portfolio_util
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger)
        {
            spdlog::set_default_logger(logger->clone("portfolio"));
        }
    }
}