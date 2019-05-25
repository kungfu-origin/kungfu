//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_COMMON_H
#define KUNGFU_COMMON_H

#include <stdlib.h>
#include <string>

namespace kungfu
{
    inline std::string get_kungfu_home()
    {
        const char *name = "KF_HOME";
        return std::string(getenv(name));
    }
}

#endif //KUNGFU_COMMON_H
