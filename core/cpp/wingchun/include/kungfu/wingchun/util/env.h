//
// Created by qlu on 2019/3/11.
//

#ifndef KUNGFU_ENV_H
#define KUNGFU_ENV_H

#include <stdlib.h>
#include <string>

namespace kungfu
{
    inline char* get_base_dir()
    {
        const char *name = "KF_HOME";
        char *value;
        value = getenv(name);
        return value;
    }

    inline void set_base_dir(const std::string& base_dir)
    {
        putenv((char*)base_dir.c_str());
    }

}
#endif //KUNGFU_ENV_H
