// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-window_time.hpp

    \brief Return an observable that emits observables every period time interval and collects items from this observable for period of time into each produced observable, on the specified scheduler.
           If the skip parameter is set, return an observable that emits observables every skip time interval and collects items from this observable for period of time into each produced observable, on the specified scheduler.

    \tparam Duration      the type of time intervals.
    \tparam Coordination  the type of the scheduler (optional).

    \param period        the period of time each window collects items before it is completed.
    \param skip          the period of time after which a new window will be created.
    \param coordination  the scheduler for the windows (optional).

    \return  Observable that emits observables every period time interval and collect items from this observable for period of time into each produced observable.
             If the skip parameter is set, return an Observable that emits observables every skip time interval and collect items from this observable for period of time into each produced observable.

    \sample
    \snippet window.cpp window period+skip+coordination sample
    \snippet output.txt window period+skip+coordination sample

    \sample
    \snippet window.cpp window period+skip sample
    \snippet output.txt window period+skip sample

    \sample
    \snippet window.cpp window period+coordination sample
    \snippet output.txt window period+coordination sample

    \sample
    \snippet window.cpp window period sample
    \snippet output.txt window period sample
*/

#if !defined(RXCPP_OPERATORS_RX_WINDOW_WITH_TIME_HPP)
#define RXCPP_OPERATORS_RX_WINDOW_WITH_TIME_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct window_with_time_invalid_arguments {};

template<class... AN>
struct window_with_time_invalid : public rxo::operator_base<window_with_time_invalid_arguments<AN...>> {
    using type = observable<window_with_time_invalid_arguments<AN...>, window_with_time_invalid<AN...>>;
};
template<class... AN>
using window_with_time_invalid_t = typename window_with_time_invalid<AN...>::type;

template<class T, class Duration, class Coordination>
struct window_with_time
{
    typedef rxu::decay_t<T> source_value_type;
    typedef observable<source_value_type> value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct window_with_time_values
    {
        window_with_time_values(duration_type p, duration_type s, coordination_type c)
            : period(p)
            , skip(s)
            , coordination(c)
        {
        }
        duration_type period;
        duration_type skip;
        coordination_type coordination;
    };
    window_with_time_values initial;

    window_with_time(duration_type period, duration_type skip, coordination_type coordination)
        : initial(period, skip, coordination)
    {
    }

    template<class Subscriber>
    struct window_with_time_observer
    {
        typedef window_with_time_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;

        struct window_with_time_subscriber_values : public window_with_time_values
        {
            window_with_time_subscriber_values(composite_subscription cs, dest_type d, window_with_time_values v, coordinator_type c)
                : window_with_time_values(v)
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
            mutable std::deque<rxcpp::subjects::subject<T>> subj;
            rxsc::scheduler::clock_type::time_point expected;
        };
        std::shared_ptr<window_with_time_subscriber_values> state;

        window_with_time_observer(composite_subscription cs, dest_type d, window_with_time_values v, coordinator_type c)
            : state(std::make_shared<window_with_time_subscriber_values>(window_with_time_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
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
                localState->worker.schedule(selectedDisposer.get());
            });

            //
            // The scheduler is FIFO for any time T. Since the observer is scheduling
            // on_next/on_error/oncompleted the timed schedule calls must be resheduled
            // when they occur to ensure that production happens after on_next/on_error/oncompleted
            //

            auto release_window = [localState](const rxsc::schedulable&) {
                localState->worker.schedule([localState](const rxsc::schedulable&) {
                    localState->subj[0].get_subscriber().on_completed();
                    localState->subj.pop_front();
                });
            };
            auto selectedRelease = on_exception(
                [&](){return localState->coordinator.act(release_window);},
                localState->dest);
            if (selectedRelease.empty()) {
                return;
            }

            auto create_window = [localState, selectedRelease](const rxsc::schedulable&) {
                localState->subj.push_back(rxcpp::subjects::subject<T>());
                localState->dest.on_next(localState->subj[localState->subj.size() - 1].get_observable().as_dynamic());

                auto produce_at = localState->expected + localState->period;
                localState->expected += localState->skip;
                localState->worker.schedule(produce_at, [localState, selectedRelease](const rxsc::schedulable&) {
                    localState->worker.schedule(selectedRelease.get());
                });
            };
            auto selectedCreate = on_exception(
                [&](){return localState->coordinator.act(create_window);},
                localState->dest);
            if (selectedCreate.empty()) {
                return;
            }

            state->worker.schedule_periodically(
                state->expected,
                state->skip,
                [localState, selectedCreate](const rxsc::schedulable&) {
                    localState->worker.schedule(selectedCreate.get());
                });
        }

        void on_next(T v) const {
            auto localState = state;
            auto work = [v, localState](const rxsc::schedulable&){
                for (auto s : localState->subj) {
                    s.get_subscriber().on_next(v);
                }
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        void on_error(std::exception_ptr e) const {
            auto localState = state;
            auto work = [e, localState](const rxsc::schedulable&){
                for (auto s : localState->subj) {
                    s.get_subscriber().on_error(e);
                }
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
                for (auto s : localState->subj) {
                    s.get_subscriber().on_completed();
                }
                localState->dest.on_completed();
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        static subscriber<T, observer_type> make(dest_type d, window_with_time_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, observer_type(this_type(cs, std::move(d), std::move(v), std::move(coordinator))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(window_with_time_observer<Subscriber>::make(std::move(dest), initial)) {
        return      window_with_time_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-window_time.hpp
*/
template<class... AN>
auto window_with_time(AN&&... an)
    ->      operator_factory<window_with_time_tag, AN...> {
     return operator_factory<window_with_time_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<window_with_time_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowWithTime = rxo::detail::window_with_time<SourceValue, rxu::decay_t<Duration>, identity_one_worker>,
        class Value = rxu::value_type_t<WindowWithTime>>
    static auto member(Observable&& o, Duration period)
        -> decltype(o.template lift<Value>(WindowWithTime(period, period, identity_current_thread()))) {
        return      o.template lift<Value>(WindowWithTime(period, period, identity_current_thread()));
    }

    template<class Observable, class Duration, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowWithTime = rxo::detail::window_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<WindowWithTime>>
    static auto member(Observable&& o, Duration period, Coordination&& cn)
        -> decltype(o.template lift<Value>(WindowWithTime(period, period, std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(WindowWithTime(period, period, std::forward<Coordination>(cn)));
    }

    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowWithTime = rxo::detail::window_with_time<SourceValue, rxu::decay_t<Duration>, identity_one_worker>,
        class Value = rxu::value_type_t<WindowWithTime>>
    static auto member(Observable&& o, Duration&& period, Duration&& skip)
        -> decltype(o.template lift<Value>(WindowWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), identity_current_thread()))) {
        return      o.template lift<Value>(WindowWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), identity_current_thread()));
    }

    template<class Observable, class Duration, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowWithTime = rxo::detail::window_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<WindowWithTime>>
    static auto member(Observable&& o, Duration&& period, Duration&& skip, Coordination&& cn)
        -> decltype(o.template lift<Value>(WindowWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(WindowWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::window_with_time_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "window_with_time takes (Duration, optional Duration, optional Coordination)");
    }
};

}

#endif
