// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-take_until.hpp

    \brief For each item from this observable until on_next occurs on the trigger observable or until the specified time, emit them from the new observable that is returned.
           take_until takes (TriggerObservable, optional Coordination) or (TimePoint, optional Coordination)

    \tparam  TriggerSource  the type of the trigger observable.
    \tparam  TimePoint  the type of the time interval.
    \tparam  Coordination   the type of the scheduler (optional).

    \param  t   an observable whose first emitted item will stop emitting items from the source observable.
    \param  when  a time point when the returned observable will stop emitting items.
    \param  cn  the scheduler to use for scheduling the items (optional).

    \return  An observable that emits the items emitted by the source observable until trigger observable emitted or the time runs out.

    \sample
    \snippet take_until.cpp take_until sample
    \snippet output.txt take_until sample

    \sample
    \snippet take_until.cpp threaded take_until sample
    \snippet output.txt threaded take_until sample

    \sample
    \snippet take_until.cpp take_until time sample
    \snippet output.txt take_until time sample

    \sample
    \snippet take_until.cpp threaded take_until time sample
    \snippet output.txt threaded take_until time sample
*/

#if !defined(RXCPP_OPERATORS_RX_TAKE_UNTIL_HPP)
#define RXCPP_OPERATORS_RX_TAKE_UNTIL_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct take_until_invalid_arguments {};

template<class... AN>
struct take_until_invalid : public rxo::operator_base<take_until_invalid_arguments<AN...>> {
    using type = observable<take_until_invalid_arguments<AN...>, take_until_invalid<AN...>>;
};
template<class... AN>
using take_until_invalid_t = typename take_until_invalid<AN...>::type;

template<class T, class Observable, class TriggerObservable, class Coordination>
struct take_until : public operator_base<T>
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

    take_until(source_type s, trigger_source_type t, coordination_type sf)
        : initial(std::move(s), std::move(t), std::move(sf))
    {
    }

    struct mode
    {
        enum type {
            taking,    // no messages from trigger
            clear,     // trigger completed
            triggered, // trigger sent on_next
            errored,   // error either on trigger or on observable
            stopped    // observable completed
        };
    };

    template<class Subscriber>
    void on_subscribe(Subscriber s) const {

        typedef Subscriber output_type;
        struct take_until_state_type
            : public std::enable_shared_from_this<take_until_state_type>
            , public values
        {
            take_until_state_type(const values& i, coordinator_type coor, const output_type& oarg)
                : values(i)
                , mode_value(mode::taking)
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

        auto coordinator = initial.coordination.create_coordinator(s.get_subscription());

        // take a copy of the values for each subscription
        auto state = std::make_shared<take_until_state_type>(initial, std::move(coordinator), std::move(s));

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
                if (state->mode_value != mode::taking) {return;}
                state->mode_value = mode::triggered;
                state->out.on_completed();
            },
        // on_error
            [state](std::exception_ptr e) {
                if (state->mode_value != mode::taking) {return;}
                state->mode_value = mode::errored;
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (state->mode_value != mode::taking) {return;}
                state->mode_value = mode::clear;
            }
        );
        auto selectedSinkTrigger = on_exception(
            [&](){return state->coordinator.out(sinkTrigger);},
            state->out);
        if (selectedSinkTrigger.empty()) {
            return;
        }
        trigger->subscribe(std::move(selectedSinkTrigger.get()));

        auto sinkSource = make_subscriber<T>(
        // split subscription lifetime
            state->source_lifetime,
        // on_next
            [state](T t) {
                //
                // everything is crafted to minimize the overhead of this function.
                //
                if (state->mode_value < mode::triggered) {
                    state->out.on_next(t);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                if (state->mode_value > mode::clear) {return;}
                state->mode_value = mode::errored;
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (state->mode_value > mode::clear) {return;}
                state->mode_value = mode::stopped;
                state->out.on_completed();
            }
        );
        auto selectedSinkSource = on_exception(
            [&](){return state->coordinator.out(sinkSource);},
            state->out);
        if (selectedSinkSource.empty()) {
            return;
        }
        source->subscribe(std::move(selectedSinkSource.get()));
    }
};

}

/*! @copydoc rx-take_until.hpp
*/
template<class... AN>
auto take_until(AN&&... an)
    ->     operator_factory<take_until_tag, AN...> {
    return operator_factory<take_until_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<take_until_tag>
{
    template<class Observable, class TimePoint,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<TimePoint, rxsc::scheduler::clock_type::time_point>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timer = typename rxu::defer_type<rxs::detail::timer, identity_one_worker>::type,
        class TimerValue = rxu::value_type_t<Timer>,
        class TriggerObservable = observable<TimerValue, Timer>,
        class TakeUntil = rxo::detail::take_until<SourceValue, rxu::decay_t<Observable>, TriggerObservable, identity_one_worker>,
        class Value = rxu::value_type_t<TakeUntil>,
        class Result = observable<Value, TakeUntil>>
    static Result member(Observable&& o, TimePoint&& when) {
        auto cn = identity_current_thread();
        return Result(TakeUntil(std::forward<Observable>(o), rxs::timer(std::forward<TimePoint>(when), cn), cn));
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
        class TakeUntil = rxo::detail::take_until<SourceValue, rxu::decay_t<Observable>, TriggerObservable, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<TakeUntil>,
        class Result = observable<Value, TakeUntil>>
    static Result member(Observable&& o, TimePoint&& when, Coordination cn) {
        return Result(TakeUntil(std::forward<Observable>(o), rxs::timer(std::forward<TimePoint>(when), cn), cn));
    }

    template<class Observable, class TriggerObservable,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, TriggerObservable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class TakeUntil = rxo::detail::take_until<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<TriggerObservable>, identity_one_worker>,
        class Value = rxu::value_type_t<TakeUntil>,
        class Result = observable<Value, TakeUntil>>
    static Result member(Observable&& o, TriggerObservable&& t) {
        return Result(TakeUntil(std::forward<Observable>(o), std::forward<TriggerObservable>(t), identity_current_thread()));
    }

     template<class Observable, class TriggerObservable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, TriggerObservable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class TakeUntil = rxo::detail::take_until<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<TriggerObservable>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<TakeUntil>,
        class Result = observable<Value, TakeUntil>>
    static Result member(Observable&& o, TriggerObservable&& t, Coordination&& cn) {
        return Result(TakeUntil(std::forward<Observable>(o), std::forward<TriggerObservable>(t), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::take_until_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "take_until takes (TriggerObservable, optional Coordination) or (TimePoint, optional Coordination)");
    }
};

}

#endif
