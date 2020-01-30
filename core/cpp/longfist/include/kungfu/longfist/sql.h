//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_SQL_H
#define KUNGFU_LONGFIST_SQL_H

#include <spdlog/spdlog.h>
#include <boost/hana.hpp>
#include <kungfu/common.h>

namespace kungfu::longfist
{

    struct ToSql
    {
        template<typename T>
        void operator()(boost::hana::basic_type<T> t, const event_ptr &event) const
        {
            SPDLOG_WARN("to sql");
            auto data = event->data<T>();
            boost::hana::for_each(boost::hana::accessors<T>(), [&](auto accessor)
            {
//                SPDLOG_WARN("{} -> {}", boost::hana::first(accessor).c_str(), boost::hana::second(accessor)(data));
            });
        }
    };

    constexpr auto to_sql = ToSql{};
}

#endif //KUNGFU_LONGFIST_SQL_H
