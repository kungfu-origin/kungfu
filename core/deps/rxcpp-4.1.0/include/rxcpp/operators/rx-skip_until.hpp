// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-skip_until.hpp

    \brief Make new observable with items skipped until on_next occurs on the trigger observable or until the specified time.
           skip_until takes (TriggerObservable, optional Coordination) or (TimePoint, optional Coordination)

    \tparam  TriggerSource  the type of the trigger observable.
    \tparam  Coordination   the type of the scheduler (optional).

    \param  t  an observable that has to emit an item before the source observable's elements begin to be mirrored by the resulting observable.
    \param  cn  the scheduler to use for scheduling the items (optional).

    \return  An observable that skips items from the source observable until the second observable emits an item or the time runs out, then emits the remaining items.

    \sample
    \snippet skip_until.cpp skip_until sample
    \snippet output.txt skip_until sample

    \sample
    \snippet skip_until.cpp threaded skip_until sample
    \snippet output.txt threaded skip_until sample
*/

#if !defined(RXCPP_OPERATORS_RX_SKIP_UNTIL_HPP)
#define RXCPP_OPERATORS_RX_SKIP_UNTIL_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct skip_until_invalid_arguments {};

template<class... AN>
struct skip_until_invalid : public rxo::operator_base<skip_until_invalid_arguments<AN...>> {
    using type = observable<skip_until_invalid_arguments<AN...>, skip_until_invalid<AN...>>;
};
template<class... AN>
using skip_until_invalid_t = typename skip_until_invalid<AN...>::type;

template<class T, class Observable, class TriggerObservable, class Coordination>
struct skip_until : public operator_base<T>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<TriggerObservable> trigger_source_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    struct values
    {
        values(source_type s, trigger_source_type t, coordination_type sf)
            : source(std::move(s))
            , trigger(std::move(t))
            , coordination(std::move(sf))
        {
        }
        source_type source;
        trigger_source_type trigger;
        coordination_type coordination;
    };
    values initial;

    skip_until(source_type s, trigger_source_type t, coordination_type sf)
        : initial(std::move(s), std::move(t), std::move(sf))
    {
    }

    struct mode
    {
        enum type {
            skipping,  // no messages from trigger
            clear,     // trigger completed
            triggered, // trigger sent on_next
            errored,   // error either on trigger or on observable
            stopped    // observable completed
        };
    };

    template<class Subscriber>
    void on_subscribe(Subscriber s) const {

        typedef Subscriber output_type;
        struct state_type
            : public std::enable_shared_from_this<state_type>
            , public values
        {
            state_type(const values& i, coordinator_type coor, const output_type& oarg)
                : values(i)
                , mode_value(mode::skipping)
                , coordinator(std::move(coor))
                , out(oarg)
            {
                out.add(trigger_lifetime);
                out.add(source_lifetime);
            }
            typename mode::type mode_value;
            composite_subscription trigger_lifetime;
            composite_subscription source_lifetime;
            coordinator_type coordinator;
            output_type out;
        };

        auto coordinator = initial.coordination.create_coordinator();

        // take a copy of the values for each subscription
        auto state = std::make_shared<state_type>(initial, std::move(coordinator), std::move(s));

        auto trigger = on_exception(
            [&](){return state->coordinator.in(state->trigger);},
            state->out);
        if (trigger.empty()) {
            return;
        }

        auto source = on_exception(
            [&](){return state->coordinator.in(state->source);},
            state->out);
        if (source.empty()) {
            return;
        }

        auto sinkTrigger = make_subscriber<typename trigger_source_type::value_type>(
        // share parts of subscription
            state->out,
        // new lifetime
            state->trigger_lifetime,
        // on_next
            [state](const typename trigger_source_type::value_type&) {
                if (state->mode_value != mode::skipping) {
                    return;
                }
                state->mode_value = mode::triggered;
                state->trigger_lifetime.unsubscribe();
            },
        // on_error
            [state](std::exception_ptr e) {
                if (state->mode_value != mode::skipping) {
                    return;
                }
                state->mode_value = mode::errored;
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (state->mode_value != mode::skipping) {
                    return;
                }
                state->mode_value = mode::clear;
                state->trigger_lifetime.unsubscribe();
            }
        );
        auto selectedSinkTrigger = on_exception(
            [&](){return state->coordinator.out(sinkTrigger);},
            state->out);
        if (selectedSinkTrigger.empty()) {
            return;
        }
        trigger->subscribe(std::move(selectedSinkTrigger.get()));

        source.get().subscribe(
        // split subscription lifetime
            state->source_lifetime,
        // on_next
            [state](T t) {
                if (state->mode_value != mode::triggered) {
                    return;
                }
                state->out.on_next(t);
            },
        // on_error
            [state](std::exception_ptr e) {
                if (state->mode_value > mode::triggered) {
                    return;
                }
                state->mode_value = mode::errored;
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (state->mode_value != mode::triggered) {
                    return;
                }
                state->mode_value = mode::stopped;
                state->out.on_completed();
            }
        );
    }
};

}

/*! @copydoc rx-skip_until.hpp
*/
template<class... AN>
auto skip_until(AN&&... an)
    ->     operator_factory<skip_until_tag, AN...> {
    return operator_factory<skip_until_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<skip_until_tag>
{
    template<class Observable, class TimePoint,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<TimePoint, rxsc::scheduler::clock_type::time_point>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timer = typename rxu::defer_type<rxs::detail::timer, identity_one_worker>::type,
        class TimerValue = rxu::value_type_t<Timer>,
        class TriggerObservable = observable<TimerValue, Timer>,
        class SkipUntil = rxo::detail::skip_until<SourceValue, rxu::decay_t<Observable>, TriggerObservable, identity_one_worker>,
        class Value = rxu::value_type_t<SkipUntil>,
        class Result = observable<Value, SkipUntil>>
    static Result member(Observable&& o, TimePoint&& when) {
        auto cn = identity_current_thread();
        return Result(SkipUntil(std::forward<Observable>(o), rxs::timer(std::forward<TimePoint>(when), cn), cn));
    }

    template<class Observable, class TimePoint, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            std::is_convertible<TimePoint, rxsc::scheduler::clock_type::time_point>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timer = typename rxu::defer_type<rxs::detail::timer, rxu::decay_t<Coordination>>::type,
        class TimerValue = rxu::value_type_t<Timer>,
        class TriggerObservable = observable<TimerValue, Timer>,
        class SkipUntil = rxo::detail::skip_until<SourceValue, rxu::decay_t<Observable>, TriggerObservable, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<SkipUntil>,
        class Result = observable<Value, SkipUntil>>
    static Result member(Observable&& o, TimePoint&& when, Coordination cn) {
        return Result(SkipUntil(std::forward<Observable>(o), rxs::timer(std::forward<TimePoint>(when), cn), cn));
    }

    template<class Observable, class TriggerObservable,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, TriggerObservable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SkipUntil = rxo::detail::skip_until<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<TriggerObservable>, identity_one_worker>,
        class Value = rxu::value_type_t<SkipUntil>,
        class Result = observable<Value, SkipUntil>>
    static Result member(Observable&& o, TriggerObservable&& t) {
        return Result(SkipUntil(std::forward<Observable>(o), std::forward<TriggerObservable>(t), identity_current_thread()));
    }

     template<class Observable, class TriggerObservable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, TriggerObservable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SkipUntil = rxo::detail::skip_until<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<TriggerObservable>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<SkipUntil>,
        class Result = observable<Value, SkipUntil>>
    static Result member(Observable&& o, TriggerObservable&& t, Coordination&& cn) {
        return Result(SkipUntil(std::forward<Observable>(o), std::forward<TriggerObservable>(t), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::skip_until_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "skip_until takes (TriggerObservable, optional Coordination) or (TimePoint, optional Coordination)");
    }
};

}

#endif
