// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-publish.hpp

    \brief Turn a cold observable hot and allow connections to the source to be independent of subscriptions.
           Turn a cold observable hot, send the most recent value to any new subscriber, and allow connections to the source to be independent of subscriptions.

    \tparam  T  the type of the emitted item (optional).

    \param  first  an initial item to be emitted by the resulting observable at connection time before emitting the items from the source observable; not emitted to observers that subscribe after the time of connection (optional).
    \param  cs  the subscription to control lifetime (optional).

    \return  rxcpp::connectable_observable that upon connection causes the source observable to emit items to its observers.

    \sample
    \snippet publish.cpp publish subject sample
    \snippet output.txt publish subject sample

    \sample
    \snippet publish.cpp publish behavior sample
    \snippet output.txt publish behavior sample
*/

#if !defined(RXCPP_OPERATORS_RX_PUBLISH_HPP)
#define RXCPP_OPERATORS_RX_PUBLISH_HPP

#include "../rx-includes.hpp"
#include "./rx-multicast.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct publish_invalid_arguments {};

template<class... AN>
struct publish_invalid : public rxo::operator_base<publish_invalid_arguments<AN...>> {
    using type = observable<publish_invalid_arguments<AN...>, publish_invalid<AN...>>;
};
template<class... AN>
using publish_invalid_t = typename publish_invalid<AN...>::type;

}

/*! @copydoc rx-publish.hpp
*/
template<class... AN>
auto publish(AN&&... an)
    ->      operator_factory<publish_tag, AN...> {
     return operator_factory<publish_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! \brief Turn a cold observable hot and allow connections to the source to be independent of subscriptions.

    \tparam  Coordination  the type of the scheduler.

    \param  cn  a scheduler all values are queued and delivered on.
    \param  cs  the subscription to control lifetime (optional).

    \return  rxcpp::connectable_observable that upon connection causes the source observable to emit items to its observers, on the specified scheduler.

    \sample
    \snippet publish.cpp publish_synchronized sample
    \snippet output.txt publish_synchronized sample
*/
template<class... AN>
auto publish_synchronized(AN&&... an)
    ->      operator_factory<publish_synchronized_tag, AN...> {
     return operator_factory<publish_synchronized_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<publish_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::subject<SourceValue>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o) {
        return Result(Multicast(std::forward<Observable>(o), Subject(composite_subscription())));
    }

    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::subject<SourceValue>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, composite_subscription cs) {
        return Result(Multicast(std::forward<Observable>(o), Subject(cs)));
    }

    template<class Observable, class T,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::behavior<SourceValue>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, T first, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(first, cs)));
    }

    template<class... AN>
    static operators::detail::publish_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "publish takes (optional CompositeSubscription) or (T, optional CompositeSubscription)");
    }
};

template<>
struct member_overload<publish_synchronized_tag>
{
    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::synchronize<SourceValue, rxu::decay_t<Coordination>>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, Coordination&& cn, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(std::forward<Coordination>(cn), cs)));
    }

    template<class... AN>
    static operators::detail::publish_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "publish_synchronized takes (Coordination, optional CompositeSubscription)");
    }
};

}

#endif
