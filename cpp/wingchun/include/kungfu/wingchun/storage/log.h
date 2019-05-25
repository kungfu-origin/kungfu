//
// Created by qlu on 2019/5/16.
//

#ifndef PROJECT_LOG_H
#define PROJECT_LOG_H

#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace storage
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger);
    }
}

#endif //PROJECT_LOG_H
