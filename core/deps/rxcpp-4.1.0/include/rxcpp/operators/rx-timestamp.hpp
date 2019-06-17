// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-timestamp.hpp

    \brief Returns an observable that attaches a timestamp to each item emitted by the source observable indicating when it was emitted.

    \tparam Coordination  the type of the scheduler (optional).

    \param coordination  the scheduler to manage timeout for each event (optional).

    \return  Observable that emits a pair: { item emitted by the source observable, time_point representing the current value of the clock }.

    \sample
    \snippet timestamp.cpp timestamp sample
    \snippet output.txt timestamp sample
*/

#if !defined(RXCPP_OPERATORS_RX_TIMESTAMP_HPP)
#define RXCPP_OPERATORS_RX_TIMESTAMP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct timestamp_invalid_arguments {};

template<class... AN>
struct timestamp_invalid : public rxo::operator_base<timestamp_invalid_arguments<AN...>> {
    using type = observable<timestamp_invalid_arguments<AN...>, timestamp_invalid<AN...>>;
};
template<class... AN>
using timestamp_invalid_t = typename timestamp_invalid<AN...>::type;

template<class T, class Coordination>
struct timestamp
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    struct timestamp_values {
        timestamp_values(coordination_type c)
            : coordination(c)
        {
        }

        coordination_type coordination;
    };
    timestamp_values initial;

    timestamp(coordination_type coordination)
        : initial(coordination)
    {
    }

    template<class Subscriber>
    struct timestamp_observer
    {
        typedef timestamp_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        coordination_type coord;

        timestamp_observer(dest_type d, coordination_type coordination)
            : dest(std::move(d)),
              coord(std::move(coordination))
        {
        }

        void on_next(source_value_type v) const {
            dest.on_next(std::make_pair(v, coord.now()));
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer_type> make(dest_type d, timestamp_values v) {
            return make_subscriber<value_type>(d, this_type(d, v.coordination));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(timestamp_observer<Subscriber>::make(std::move(dest), initial)) {
        return      timestamp_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-timestamp.hpp
*/
template<class... AN>
auto timestamp(AN&&... an)
    ->      operator_factory<timestamp_tag, AN...> {
     return operator_factory<timestamp_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<timestamp_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timestamp = rxo::detail::timestamp<SourceValue, identity_one_worker>,
        class Clock = typename rxsc::scheduler::clock_type::time_point,
        class Value = std::pair<SourceValue, Clock>>
    static auto member(Observable&& o)
        -> decltype(o.template lift<Value>(Timestamp(identity_current_thread()))) {
        return      o.template lift<Value>(Timestamp(identity_current_thread()));
    }

    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timestamp = rxo::detail::timestamp<SourceValue, rxu::decay_t<Coordination>>,
        class Clock = typename rxsc::scheduler::clock_type::time_point,
        class Value = std::pair<SourceValue, Clock>>
    static auto member(Observable&& o, Coordination&& cn)
        -> decltype(o.template lift<Value>(Timestamp(std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(Timestamp(std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::timestamp_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "timestamp takes (optional Coordination)");
    }
};

}

#endif
