// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-replay.hpp
   
    \brief 1) replay(optional Coordination, optional CompositeSubscription)
              Turn a cold observable hot, send all earlier emitted values to any new subscriber, and allow connections to the source to be independent of subscriptions.

           2) replay(Count, optional Coordination, optional CompositeSubscription)
              Turn a cold observable hot, send at most count of earlier emitted values to any new subscriber, and allow connections to the source to be independent of subscriptions.

           3) replay(Duration, optional Coordination, optional CompositeSubscription)
              Turn a cold observable hot, send values emitted within a specified time window to any new subscriber, and allow connections to the source to be independent of subscriptions.

           4) replay(Count, Duration, optional Coordination, optional CompositeSubscription)
              Turn a cold observable hot, send at most count of values emitted within a specified time window to any new subscriber, and allow connections to the source to be independent of subscriptions.

    \tparam Duration  the type of the time interval (optional).
    \tparam Count  the type of the maximum number of the most recent items sent to new observers (optional).
    \tparam Coordination  the type of the scheduler (optional).

    \param count  the maximum number of the most recent items sent to new observers (optional).
    \param d  the duration of the window in which the replayed items must be emitted
    \param cn  a scheduler all values are queued and delivered on (optional).
    \param cs  the subscription to control lifetime (optional).

    \return  rxcpp::connectable_observable that shares a single subscription to the underlying observable that will replay all of its items and notifications to any future observer.

    \sample
    \snippet replay.cpp replay sample
    \snippet output.txt replay sample

    \sample
    \snippet replay.cpp threaded replay sample
    \snippet output.txt threaded replay sample

    \sample
    \snippet replay.cpp replay count sample
    \snippet output.txt replay count sample

    \sample
    \snippet replay.cpp threaded replay count sample
    \snippet output.txt threaded replay count sample

    \sample
    \snippet replay.cpp replay period sample
    \snippet output.txt replay period sample

    \sample
    \snippet replay.cpp threaded replay period sample
    \snippet output.txt threaded replay period sample

    \sample
    \snippet replay.cpp replay count+period sample
    \snippet output.txt replay count+period sample

    \sample
    \snippet replay.cpp threaded replay count+period sample
    \snippet output.txt threaded replay count+period sample
*/

#if !defined(RXCPP_OPERATORS_RX_REPLAY_HPP)
#define RXCPP_OPERATORS_RX_REPLAY_HPP

#include "../rx-includes.hpp"
#include "./rx-multicast.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct replay_invalid_arguments {};

template<class... AN>
struct replay_invalid : public rxo::operator_base<replay_invalid_arguments<AN...>> {
    using type = observable<replay_invalid_arguments<AN...>, replay_invalid<AN...>>;
};
template<class... AN>
using replay_invalid_t = typename replay_invalid<AN...>::type;

}

/*! @copydoc rx-replay.hpp
*/
template<class... AN>
auto replay(AN&&... an)
    ->      operator_factory<replay_tag, AN...> {
     return operator_factory<replay_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}    
    
}

 template<>
struct member_overload<replay_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, identity_one_worker>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o) {
        return Result(Multicast(std::forward<Observable>(o), Subject(identity_current_thread(), composite_subscription())));
    }

    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, identity_one_worker>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, composite_subscription cs) {
        return Result(Multicast(std::forward<Observable>(o), Subject(identity_current_thread(), cs)));
    }

    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, rxu::decay_t<Coordination>>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, Coordination&& cn, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(std::forward<Coordination>(cn), cs)));
    }

    template<class Observable, class Count,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_integral<Count>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, identity_one_worker>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
    >
    static Result member(Observable&& o, Count count, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(count, identity_current_thread(), cs)));
    }

    template<class Observable, class Count, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_integral<Count>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, rxu::decay_t<Coordination>>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, Count count, Coordination&& cn, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(count, std::forward<Coordination>(cn), cs)));
    }

    template<class Observable, class Duration,
        class IsDuration = rxu::is_duration<Duration>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            IsDuration>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, identity_one_worker>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
    >
    static Result member(Observable&& o, Duration&& d, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(std::forward<Duration>(d), identity_current_thread(), cs)));
    }

    template<class Observable, class Duration, class Coordination,
        class IsDuration = rxu::is_duration<Duration>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            IsDuration,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, rxu::decay_t<Coordination>>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
        >
    static Result member(Observable&& o, Duration&& d, Coordination&& cn, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(std::forward<Duration>(d), std::forward<Coordination>(cn), cs)));
    }

    template<class Observable, class Count, class Duration,
        class IsDuration = rxu::is_duration<Duration>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_integral<Count>,
            IsDuration>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, identity_one_worker>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
    >
    static Result member(Observable&& o, Count count, Duration&& d, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(count, std::forward<Duration>(d), identity_current_thread(), cs)));
    }

    template<class Observable, class Count, class Duration, class Coordination,
        class IsDuration = rxu::is_duration<Duration>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_integral<Count>,
            IsDuration,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Subject = rxsub::replay<SourceValue, rxu::decay_t<Coordination>>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, Subject>,
        class Result = connectable_observable<SourceValue, Multicast>
    >
    static Result member(Observable&& o, Count count, Duration&& d, Coordination&& cn, composite_subscription cs = composite_subscription()) {
        return Result(Multicast(std::forward<Observable>(o), Subject(count, std::forward<Duration>(d), std::forward<Coordination>(cn), cs)));
    }

    template<class... AN>
    static operators::detail::replay_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "replay takes (optional Count, optional Duration, optional Coordination, optional CompositeSubscription)");
    }
};   
    
}

#endif
