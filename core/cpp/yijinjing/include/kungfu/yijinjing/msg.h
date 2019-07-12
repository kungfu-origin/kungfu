//
// Created by Keren Dong on 2019-06-19.
//

#ifndef KUNGFU_YIJINJING_MSG_H
#define KUNGFU_YIJINJING_MSG_H

#include <kungfu/yijinjing/common.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace msg
        {
            namespace type
            {
                enum
                {
                    PageEnd = 10000,
                    SessionStart = 10001,
                    SessionEnd = 10002,
                    Time = 10003,
                    TimeRequest = 10004,
                    Register = 10011,
                    Deregister = 10012,
                    RequestReadFrom = 10021,
                    RequestReadFromPublic = 10022,
                    RequestWriteTo = 10023,
                    RequestUnpublish = 10024,
                    RequestStart = 10025,
                    Location = 10026,
                    TradingDay = 10027,
                };
            }

            namespace data
            {
                struct TimeRequest
                {
                    int32_t id;
                    int64_t duration;
                    int64_t repeat;
#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                struct RequestReadFrom
                {
                    uint32_t source_id;
                    int64_t from_time;

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                struct RequestWriteTo
                {
                    uint32_t dest_id;

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif
            }
        }
    }
}
#endif //KUNGFU_YIJINJING_MSG_H
