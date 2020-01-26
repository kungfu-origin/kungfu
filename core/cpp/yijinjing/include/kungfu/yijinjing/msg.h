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
                    Channel = 10028
                };
            }

            namespace data
            {
                YJJ_DEFINE_DATA_STRUCT(
                        TimeRequest, 10004,
                        (int32_t, id),
                        (int64_t, duration),
                        (int64_t, repeat)
                );

                YJJ_DEFINE_DATA_STRUCT(
                        RequestReadFrom, 10021,
                        (uint32_t, source_id),
                        (int64_t, from_time)
                );

                YJJ_DEFINE_DATA_STRUCT(
                        RequestReadFromPublic, 10022,
                        (uint32_t, source_id),
                        (int64_t, from_time)
                );

                YJJ_DEFINE_DATA_STRUCT(
                        RequestWriteTo, 10023,
                        (uint32_t, dest_id)
                );

                YJJ_DEFINE_DATA_STRUCT(
                        Channel, 10028,
                        (uint32_t, source_id),
                        (uint32_t, dest_id)
                );
            }

            constexpr auto DATA_TYPE_MAP = hana::make_map(
                    hana::make_pair(hana::int_c<data::TimeRequest::type>, hana::type_c<data::TimeRequest>),
                    hana::make_pair(hana::int_c<data::RequestReadFrom::type>, hana::type_c<data::RequestReadFrom>),
                    hana::make_pair(hana::int_c<data::RequestWriteTo::type>, hana::type_c<data::RequestWriteTo>),
                    hana::make_pair(hana::int_c<data::Channel::type>, hana::type_c<data::Channel>)
            );
        }
    }
}
#endif //KUNGFU_YIJINJING_MSG_H
