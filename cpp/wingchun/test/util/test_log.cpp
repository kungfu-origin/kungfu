//
// Created by qlu on 2019/4/8.
//

#include <spdlog/spdlog.h>
#include <iostream>
#include <kungfu/log/spdlog_config.h>

int main() {
    kungfu::yijinjing::KungfuLog::setup_log("test_logger");
    kungfu::yijinjing::KungfuLog::set_log_level(0);
    SPDLOG_TRACE("hello, world");
    SPDLOG_INFO("hello, world");
}