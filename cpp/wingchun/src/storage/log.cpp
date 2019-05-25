//
// Created by qlu on 2019/5/16.
//

#include <kungfu/wingchun/storage/log.h>

namespace kungfu
{
    namespace storage
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger)
        {
            spdlog::set_default_logger(logger->clone("storage"));
        }
    }
}
