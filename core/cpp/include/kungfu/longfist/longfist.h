//
// Created by Keren Dong on 2020/1/26.
//

#ifndef KUNGFU_LONGFIST_H
#define KUNGFU_LONGFIST_H

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <kungfu/common.h>
#include <kungfu/longfist/enum.h>
#include <kungfu/longfist/types.h>

#define TYPE_PAIR(DataType) boost::hana::make_pair(HANA_STR(#DataType), boost::hana::type_c<types::DataType>)

namespace kungfu::longfist
{
    static constexpr auto StateDataTypes = boost::hana::make_map(
            TYPE_PAIR(Config),
            TYPE_PAIR(Instrument),
            TYPE_PAIR(Quote),
            TYPE_PAIR(Entrust),
            TYPE_PAIR(Transaction),
            TYPE_PAIR(Bar),
            TYPE_PAIR(OrderInput),
            TYPE_PAIR(OrderAction),
            TYPE_PAIR(OrderActionError),
            TYPE_PAIR(Order),
            TYPE_PAIR(Trade),
            TYPE_PAIR(Position),
            TYPE_PAIR(PositionDetail),
            TYPE_PAIR(OrderStat)
    );
    using StateDataTypesT = decltype(StateDataTypes);

    static constexpr auto cast_invoke = [](const event_ptr &event, auto &handler)
    {
        boost::hana::for_each(StateDataTypes, [&](auto it)
        {
            using T = typename decltype(+boost::hana::second(it))::type;
            if (T::tag == event->msg_type())
            {
                handler(boost::hana::first(it).c_str(), boost::hana::second(it), event);
            }
        });
    };
};

#endif //KUNGFU_LONGFIST_H
