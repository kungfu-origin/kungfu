// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-sample_time.hpp

    \brief Return an Observable that emits the most recent items emitted by the source Observable within periodic time intervals.

    \tparam Duration      the type of time interval.
    \tparam Coordination  the type of the scheduler (optional).

    \param period  the period of time to sample the source observable.
    \param coordination  the scheduler for the items (optional).

    \return  Observable that emits the most recently emitted item since the previous sampling.

    \sample
    \snippet sample.cpp sample period sample
    \snippet output.txt sample period sample
*/

#if !defined(RXCPP_OPERATORS_RX_SAMPLE_WITH_TIME_HPP)
#define RXCPP_OPERATORS_RX_SAMPLE_WITH_TIME_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct sample_with_time_invalid_arguments {};

template<class... AN>
struct sample_with_time_invalid : public rxo::operator_base<sample_with_time_invalid_arguments<AN...>> {
    using type = observable<sample_with_time_invalid_arguments<AN...>, sample_with_time_invalid<AN...>>;
};
template<class... AN>
using sample_with_time_invalid_t = typename sample_with_time_invalid<AN...>::type;

template<class T, class Duration, class Coordination>
struct sample_with_time
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct sample_with_time_value
    {
        sample_with_time_value(duration_type p, coordination_type c)
            : period(p)
            , coordination(c)
        {
        }
        duration_type period;
        coordination_type coordination;
    };
    sample_with_time_value initial;

    sample_with_time(duration_type period, coordination_type coordination)
        : initial(period, coordination)
    {
    }

    template<class Subscriber>
    struct sample_with_time_observer
    {
        typedef sample_with_time_observer<Subscriber> this_type;
        typedef T value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;

        struct sample_with_time_subscriber_value : public sample_with_time_value
        {
            sample_with_time_subscriber_value(composite_subscription cs, dest_type d, sample_with_time_value v, coordinator_type c)
                : sample_with_time_value(v)
                , cs(std::move(cs))
                , dest(std::move(d))
                , coordinator(std::move(c))
                , worker(coordinator.get_worker())
            {
            }
            composite_subscription cs;
            dest_type dest;
            coordinator_type coordinator;
            rxsc::worker worker;
            mutable rxu::maybe<value_type> value;
        };
        std::shared_ptr<sample_with_time_subscriber_value> state;

        sample_with_time_observer(composite_subscription cs, dest_type d, sample_with_time_value v, coordinator_type c)
            : state(std::make_shared<sample_with_time_subscriber_value>(sample_with_time_subscriber_value(std::move(cs), std::move(d), v, std::move(c))))
        {
            auto localState = state;

            auto disposer = [=](const rxsc::schedulable&){
                localState->cs.unsubscribe();
                localState->dest.unsubscribe();
                localState->worker.unsubscribe();
            };
            auto selectedDisposer = on_exception(
                [&](){ return localState->coordinator.act(disposer); },
                localState->dest);
            if (selectedDisposer.empty()) {
                return;
            }

            localState->dest.add([=](){
                localState->worker.schedule(selectedDisposer.get());
            });
            localState->cs.add([=](){
                localState->worker.schedule(selectedDisposer.get());
            });

            auto produce_sample = [localState](const rxsc::schedulable&) {
                if(!localState->value.empty()) {
                    localState->dest.on_next(*localState->value);
                    localState->value.reset();
                }
            };
            auto selectedProduce = on_exception(
                [&](){ return localState->coordinator.act(produce_sample); },
                localState->dest);
            if (selectedProduce.empty()) {
                return;
            }

            state->worker.schedule_periodically(
                localState->worker.now(),
                localState->period,
                [localState, selectedProduce](const rxsc::schedulable&) {
                    localState->worker.schedule(selectedProduce.get());
                });
        }
        
        void on_next(T v) const {
            auto localState = state;
            auto work = [v, localState](const rxsc::schedulable&) {
                localState->value.reset(v);
            };
            auto selectedWork = on_exception(
                [&](){ return localState->coordinator.act(work); },
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        void on_error(std::exception_ptr e) const {
            auto localState = state;
            auto work = [e, localState](const rxsc::schedulable&) {
                localState->dest.on_error(e);
            };
            auto selectedWork = on_exception(
                [&](){ return localState->coordinator.act(work); },
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        void on_completed() const {
            auto localState = state;
            auto work = [localState](const rxsc::schedulable&) {
                localState->dest.on_completed();
            };
            auto selectedWork = on_exception(
                [&](){ return localState->coordinator.act(work); },
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        static subscriber<T, observer<T, this_type>> make(dest_type d, sample_with_time_value v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, this_type(cs, std::move(d), std::move(v), std::move(coordinator)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(sample_with_time_observer<Subscriber>::make(std::move(dest), initial)) {
        return      sample_with_time_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-sample_time.hpp
*/
template<class... AN>
auto sample_with_time(AN&&... an)
    ->      operator_factory<sample_with_time_tag, AN...> {
     return operator_factory<sample_with_time_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<sample_with_time_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SampleWithTime = rxo::detail::sample_with_time<SourceValue, rxu::decay_t<Duration>, identity_one_worker>>
    static auto member(Observable&& o, Duration&& d)
        -> decltype(o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), identity_current_thread()))) {
        return      o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), identity_current_thread()));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SampleWithTime = rxo::detail::sample_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Coordination&& cn, Duration&& d)
        -> decltype(o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SampleWithTime = rxo::detail::sample_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Duration&& d, Coordination&& cn)
        -> decltype(o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(SampleWithTime(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::sample_with_time_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "sample_with_time takes (optional Coordination, required Duration) or (required Duration, optional Coordination)");
    }
};

}

#endif
