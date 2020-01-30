//
// Created by Keren Dong on 2020/1/26.
//

#ifndef KUNGFU_LONGFIST_H
#define KUNGFU_LONGFIST_H

#include <boost/hana.hpp>
#include <fmt/format.h>

#include <kungfu/common.h>
#include <kungfu/longfist/enum.h>
#include <kungfu/longfist/types.h>

namespace kungfu::longfist
{

    class longfist_error : public std::runtime_error
    {
    public:
        explicit longfist_error(const std::string &message) : runtime_error(message)
        {}
    };

    namespace types
    {
        static constexpr auto ALL = boost::hana::make_tuple(
                boost::hana::type_c<TimeRequest>,
                boost::hana::type_c<RequestReadFrom>,
                boost::hana::type_c<RequestWriteTo>,
                boost::hana::type_c<Channel>,
                boost::hana::type_c<Instrument>,
                boost::hana::type_c<Quote>,
                boost::hana::type_c<Entrust>,
                boost::hana::type_c<Transaction>,
                boost::hana::type_c<Bar>,
                boost::hana::type_c<OrderInput>,
                boost::hana::type_c<OrderAction>,
                boost::hana::type_c<OrderActionError>,
                boost::hana::type_c<Order>,
                boost::hana::type_c<Trade>,
                boost::hana::type_c<Position>,
                boost::hana::type_c<PositionEnd>,
                boost::hana::type_c<PositionDetail>,
                boost::hana::type_c<PositionDetailEnd>,
                boost::hana::type_c<Report>
        );
    }

    static constexpr auto cast_invoke = [](const event_ptr &event, auto handler)
    {
        boost::hana::for_each(types::ALL, [&](auto type)
        {
            using T = typename decltype(+type)::type;
            if (T::tag == event->msg_type())
            {
                handler(type, event);
                return;
            }
        });
        throw longfist_error(fmt::format("Invalid message type {}", event->msg_type()));
    };
};

#endif //KUNGFU_LONGFIST_H
