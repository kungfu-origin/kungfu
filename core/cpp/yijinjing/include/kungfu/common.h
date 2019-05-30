//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_COMMON_H
#define KUNGFU_COMMON_H

#include <stdlib.h>
#include <string>
#include <unordered_map>

#define KF_ENV_HOME             kungfu::get_env("KF_HOME")
#define KF_ENV_LOG_LEVEL        kungfu::get_env("KF_LOG_LEVEL")
#define KF_ENV_NO_EXT           kungfu::get_env("KF_NO_EXT")

namespace kungfu
{
    inline std::string get_env(std::string name)
    {
        return std::string(getenv(name.c_str()));
    }

    inline std::string get_kungfu_home()
    {
        const char *name = "KF_HOME";
        return std::string(getenv(name));
    }

    inline std::unordered_map<std::string, std::string> get_kungfu_env()
    {
        std::unordered_map<std::string, std::string> env = {};
        return env;
    }
}

#endif //KUNGFU_COMMON_H
