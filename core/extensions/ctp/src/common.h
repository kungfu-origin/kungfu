//
// Created by qlu on 2019/8/5.
//

#ifndef KUNGFU_CTP_EXT_COMMON_H
#define KUNGFU_CTP_EXT_COMMON_H

#include <string>

namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            inline std::string disconnected_reason(int reason)
            {
                switch (reason)
                {
                    case 0x1001:
                        return "网络读失败";
                    case 0x1002:
                        return "网络写失败";
                    case 0x2001:
                        return "接收心跳超时";
                    case 0x2002:
                        return "发送心跳失败";
                    case 0x2003:
                        return "收到错误报文";
                    default:
                        return "Unknown";
                }
            }
        }
    }
}

#endif //KUNGFU_CTP_EXT_COMMON_H
