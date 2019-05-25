//
// Created by qlu on 2019/3/19.
//

#ifndef KUNGFU_REQRSP_H
#define KUNGFU_REQRSP_H

#include <kungfu/wingchun/md_struct.h>

namespace kungfu
{
    struct LoginRequest
    {
        std::string sender;
        std::string recipient;
    };

    struct SubscribeRequest
    {
        std::string sender;
        std::string recipient;
        bool is_level2;
        std::vector<journal::Instrument> instruments;
    };

    struct SubscribeRsp
    {
        std::string sender;
    };
}
#endif //KUNGFU_REQRSP_H
