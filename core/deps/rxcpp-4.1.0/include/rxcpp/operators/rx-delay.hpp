// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-delay.hpp

    \brief Return an observable that emits each item emitted by the source observable after the specified delay.

    \tparam Duration      the type of time interval
    \tparam Coordination  the type of the scheduler

    \param period        the period of time each item is delayed
    \param coordination  the scheduler for the delays

    \return  Observable that emits each item emitted by the source observable after the specified delay.

    \sample
    \snippet delay.cpp delay period+coordination sample
    \snippet output.txt delay period+coordination sample
*/

#if !defined(RXCPP_OPERATORS_RX_DELAY_HPP)
#define RXCPP_OPERATORS_RX_DELAY_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct delay_invalid_arguments {};

template<class... AN>
struct delay_invalid : public rxo::operator_base<delay_invalid_arguments<AN...>> {
    using type = observable<delay_invalid_arguments<AN...>, delay_invalid<AN...>>;
};
template<class... AN>
using delay_invalid_t = typename delay_invalid<AN...>::type;
    
template<class T, class Duration, class Coordination>
struct delay
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct delay_values
    {
        delay_values(duration_type p, coordination_type c)
            : period(p)
            , coordination(c)
        {
        }
        duration_type period;
        coordination_type coordination;
    };
    delay_values initial;

    delay(duration_type period, coordination_type coordination)
        : initial(period, coordination)
    {
    }

    template<class Subscriber>
    struct delay_observer
    {
        typedef delay_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;

        struct delay_subscriber_values : public delay_values
        {
            delay_subscriber_values(composite_subscription cs, dest_type d, delay_values v, coordinator_type c)
                : delay_values(v)
                , cs(std::move(cs))
                , dest(std::move(d))
                , coordinator(std::move(c))
                , worker(coordinator.get_worker())
                , expected(worker.now())
            {
            }
            composite_subscription cs;
            dest_type dest;
            coordinator_type coordinator;
            rxsc::worker worker;
            rxsc::scheduler::clock_type::time_point expected;
        };
        std::shared_ptr<delay_subscriber_values> state;

        delay_observer(composite_subscription cs, dest_type d, delay_values v, coordinator_type c)
            : state(std::make_shared<delay_subscriber_values>(delay_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
        {
            auto localState = state;

            auto disposer = [=](const rxsc::schedulable&){
                localState->cs.unsubscribe();
                localState->dest.unsubscribe();
                localState->worker.unsubscribe();
            };
            auto selectedDisposer = on_exception(
                [&](){return localState->coordinator.act(disposer);},
                localState->dest);
            if (selectedDisposer.empty()) {
                return;
            }

            localState->dest.add([=](){
                localState->worker.schedule(selectedDisposer.get());
            });
            localState->cs.add([=](){
                localState->worker.schedule(localState->worker.now() + localState->period, selectedDisposer.get());
            });
        }

        void on_next(T v) const {
            auto localState = state;
            auto work = [v, localState](const rxsc::schedulable&){
                localState->dest.on_next(v);
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(localState->worker.now() + localState->period, selectedWork.get());
        }

        void on_error(std::exception_ptr e) const {
            auto localState = state;
            auto work = [e, localState](const rxsc::schedulable&){
                localState->dest.on_error(e);
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        void on_completed() const {
            auto localState = state;
            auto work = [localState](const rxsc::schedulable&){
                localState->dest.on_completed();
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(localState->worker.now() + localState->period, selectedWork.get());
        }

        static subscriber<T, observer_type> make(dest_type d, delay_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, observer_type(this_type(cs, std::move(d), std::move(v), std::move(coordinator))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(delay_observer<Subscriber>::make(std::move(dest), initial)) {
        return      delay_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-delay.hpp
*/
template<class... AN>
auto delay(AN&&... an)
    ->      operator_factory<delay_tag, AN...> {
     return operator_factory<delay_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<delay_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class delay = rxo::detail::delay<SourceValue, rxu::decay_t<Duration>, identity_one_worker>>
    static auto member(Observable&& o, Duration&& d)
        -> decltype(o.template lift<SourceValue>(delay(std::forward<Duration>(d), identity_current_thread()))) {
        return      o.template lift<SourceValue>(delay(std::forward<Duration>(d), identity_current_thread()));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class delay = rxo::detail::delay<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Coordination&& cn, Duration&& d)
        -> decltype(o.template lift<SourceValue>(delay(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(delay(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class delay = rxo::detail::delay<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Duration&& d, Coordination&& cn)
        -> decltype(o.template lift<SourceValue>(delay(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(delay(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::delay_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "delay takes (optional Coordination, required Duration) or (required Duration, optional Coordination)");
    }
}; 
    
}

#endif
