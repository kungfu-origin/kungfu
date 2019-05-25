//
// Created by qlu on 2019/5/16.
//

#include "log.h"

namespace kungfu
{
    namespace commission_util
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger)
        {
            spdlog::set_default_logger(logger->clone("commission"));
        }
    }
}
