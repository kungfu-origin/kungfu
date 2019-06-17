// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-time_interval.hpp

    \brief Returns an observable that emits indications of the amount of time lapsed between consecutive emissions of the source observable.
           The first emission from this new Observable indicates the amount of time lapsed between the time when the observer subscribed to the Observable and the time when the source Observable emitted its first item.

    \tparam Coordination  the type of the scheduler.

    \param coordination  the scheduler for time intervals.

    \return  Observable that emits a time_duration to indicate the amount of time lapsed between pairs of emissions.

    \sample
    \snippet time_interval.cpp time_interval sample
    \snippet output.txt time_interval sample
*/

#if !defined(RXCPP_OPERATORS_RX_TIME_INTERVAL_HPP)
#define RXCPP_OPERATORS_RX_TIME_INTERVAL_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct time_interval_invalid_arguments {};

template<class... AN>
struct time_interval_invalid : public rxo::operator_base<time_interval_invalid_arguments<AN...>> {
    using type = observable<time_interval_invalid_arguments<AN...>, time_interval_invalid<AN...>>;
};
template<class... AN>
using time_interval_invalid_t = typename time_interval_invalid<AN...>::type;

template<class T, class Coordination>
struct time_interval
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    struct time_interval_values {
        time_interval_values(coordination_type c)
            : coordination(c)
        {
        }

        coordination_type coordination;
    };
    time_interval_values initial;

    time_interval(coordination_type coordination)
        : initial(coordination)
    {
    }

    template<class Subscriber>
    struct time_interval_observer
    {
        typedef time_interval_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        typedef rxsc::scheduler::clock_type::time_point time_point;
        dest_type dest;
        coordination_type coord;
        mutable time_point last;

        time_interval_observer(dest_type d, coordination_type coordination)
            : dest(std::move(d)),
              coord(std::move(coordination)),
              last(coord.now())
        {
        }

        void on_next(source_value_type) const {
            time_point now = coord.now();
            dest.on_next(now - last);
            last = now;
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer_type> make(dest_type d, time_interval_values v) {
            return make_subscriber<value_type>(d, this_type(d, v.coordination));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(time_interval_observer<Subscriber>::make(std::move(dest), initial)) {
        return      time_interval_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-time_interval.hpp
*/
template<class... AN>
auto time_interval(AN&&... an)
    ->      operator_factory<time_interval_tag, AN...> {
     return operator_factory<time_interval_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<time_interval_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class TimeInterval = rxo::detail::time_interval<SourceValue, identity_one_worker>,
        class Value = typename rxsc::scheduler::clock_type::time_point::duration>
    static auto member(Observable&& o)
        -> decltype(o.template lift<Value>(TimeInterval(identity_current_thread()))) {
        return      o.template lift<Value>(TimeInterval(identity_current_thread()));
    }

    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class TimeInterval = rxo::detail::time_interval<SourceValue, rxu::decay_t<Coordination>>,
        class Value = typename rxsc::scheduler::clock_type::time_point::duration>
    static auto member(Observable&& o, Coordination&& cn)
        -> decltype(o.template lift<Value>(TimeInterval(std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(TimeInterval(std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::time_interval_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "time_interval takes (optional Coordination)");
    }
};

}

#endif
