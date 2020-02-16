//
// Created by Keren Dong on 2020/1/26.
//

#ifndef KUNGFU_LONGFIST_H
#define KUNGFU_LONGFIST_H

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <kungfu/common.h>
#include <kungfu/longfist/enums.h>
#include <kungfu/longfist/types.h>

#define TYPE_PAIR(DataType) boost::hana::make_pair(HANA_STR(#DataType), boost::hana::type_c<types::DataType>)

namespace kungfu::longfist
{
    constexpr auto StateDataTypes = boost::hana::make_map(
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

    constexpr auto build_state_map = [](auto types)
    {
        auto maps = boost::hana::transform(boost::hana::values(types), [](auto value)
        {
            using DataType = typename decltype(+value)::type;
            return boost::hana::make_pair(value, std::unordered_map<uint64_t, DataType>());
        });
        return boost::hana::unpack(maps, boost::hana::make_map);
    };
    using StateMapType = decltype(build_state_map(longfist::StateDataTypes));
    DECLARE_PTR(StateMapType)

    class recover
    {
    public:
        explicit recover(StateMapType &state_map) : state_map_(state_map)
        {}

        template<typename DataType>
        void operator()(const std::string &name, boost::hana::basic_type<DataType> type, const event_ptr &event)
        {
            auto data = event->data<DataType>();
            state_map_[boost::hana::type_c<DataType>][data.uid()] = data;
        }

    private:
        StateMapType &state_map_;
    };

    constexpr auto cast_type_invoke = [](const std::string &type, auto &data, auto &handler)
    {
        boost::hana::for_each(StateDataTypes, [&](auto it)
        {
            if (strcmp(boost::hana::first(it).c_str(), type.c_str()) == 0)
            {
                using T = typename decltype(+boost::hana::second(it))::type;
                handler(boost::hana::first(it).c_str(), boost::hana::second(it), data);
            }
        });
    };

    constexpr auto cast_event_invoke = [](const event_ptr &event, auto &handler)
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
