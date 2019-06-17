// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-timeout.hpp

    \brief Return an observable that terminates with timeout_error if a particular timespan has passed without emitting another item from the source observable.

    \tparam Duration      the type of time interval.
    \tparam Coordination  the type of the scheduler (optional).

    \param period        the period of time wait for another item from the source observable.
    \param coordination  the scheduler to manage timeout for each event (optional).

    \return  Observable that terminates with an error if a particular timespan has passed without emitting another item from the source observable.

    \sample
    \snippet timeout.cpp timeout sample
    \snippet output.txt timeout sample
*/

#if !defined(RXCPP_OPERATORS_RX_TIMEOUT_HPP)
#define RXCPP_OPERATORS_RX_TIMEOUT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

class timeout_error: public std::runtime_error
{
    public:
        explicit timeout_error(const std::string& msg):
            std::runtime_error(msg)
        {}
};

namespace operators {

namespace detail {

template<class... AN>
struct timeout_invalid_arguments {};

template<class... AN>
struct timeout_invalid : public rxo::operator_base<timeout_invalid_arguments<AN...>> {
    using type = observable<timeout_invalid_arguments<AN...>, timeout_invalid<AN...>>;
};
template<class... AN>
using timeout_invalid_t = typename timeout_invalid<AN...>::type;

template<class T, class Duration, class Coordination>
struct timeout
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct timeout_values
    {
        timeout_values(duration_type p, coordination_type c)
            : period(p)
            , coordination(c)
        {
        }

        duration_type period;
        coordination_type coordination;
    };
    timeout_values initial;

    timeout(duration_type period, coordination_type coordination)
        : initial(period, coordination)
    {
    }

    template<class Subscriber>
    struct timeout_observer
    {
        typedef timeout_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;

        struct timeout_subscriber_values : public timeout_values
        {
            timeout_subscriber_values(composite_subscription cs, dest_type d, timeout_values v, coordinator_type c)
                : timeout_values(v)
                , cs(std::move(cs))
                , dest(std::move(d))
                , coordinator(std::move(c))
                , worker(coordinator.get_worker())
                , index(0)
            {
            }

            composite_subscription cs;
            dest_type dest;
            coordinator_type coordinator;
            rxsc::worker worker;
            mutable std::size_t index;
        };
        typedef std::shared_ptr<timeout_subscriber_values> state_type;
        state_type state;

        timeout_observer(composite_subscription cs, dest_type d, timeout_values v, coordinator_type c)
            : state(std::make_shared<timeout_subscriber_values>(timeout_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
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

            auto work = [v, localState](const rxsc::schedulable&) {
                auto new_id = ++localState->index;
                auto produce_time = localState->worker.now() + localState->period;

                localState->worker.schedule(produce_time, produce_timeout(new_id, localState));
            };
            auto selectedWork = on_exception(
                [&](){return localState->coordinator.act(work);},
                localState->dest);
            if (selectedWork.empty()) {
                return;
            }
            localState->worker.schedule(selectedWork.get());
        }

        static std::function<void(const rxsc::schedulable&)> produce_timeout(std::size_t id, state_type state) {
            auto produce = [id, state](const rxsc::schedulable&) {
                if(id != state->index)
                    return;

                state->dest.on_error(std::make_exception_ptr(rxcpp::timeout_error("timeout has occurred")));
            };

            auto selectedProduce = on_exception(
                    [&](){ return state->coordinator.act(produce); },
                    state->dest);
            if (selectedProduce.empty()) {
                return std::function<void(const rxsc::schedulable&)>();
            }

            return std::function<void(const rxsc::schedulable&)>(selectedProduce.get());
        }

        void on_next(T v) const {
            auto localState = state;
            auto work = [v, localState](const rxsc::schedulable&) {
                auto new_id = ++localState->index;
                auto produce_time = localState->worker.now() + localState->period;

                localState->dest.on_next(v);
                localState->worker.schedule(produce_time, produce_timeout(new_id, localState));
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

        static subscriber<T, observer_type> make(dest_type d, timeout_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, observer_type(this_type(cs, std::move(d), std::move(v), std::move(coordinator))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(timeout_observer<Subscriber>::make(std::move(dest), initial)) {
        return      timeout_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-timeout.hpp
*/
template<class... AN>
auto timeout(AN&&... an)
    ->      operator_factory<timeout_tag, AN...> {
     return operator_factory<timeout_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<timeout_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timeout = rxo::detail::timeout<SourceValue, rxu::decay_t<Duration>, identity_one_worker>>
    static auto member(Observable&& o, Duration&& d)
        -> decltype(o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), identity_current_thread()))) {
        return      o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), identity_current_thread()));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timeout = rxo::detail::timeout<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Coordination&& cn, Duration&& d)
        -> decltype(o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Timeout = rxo::detail::timeout<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Duration&& d, Coordination&& cn)
        -> decltype(o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(Timeout(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::timeout_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "timeout takes (optional Coordination, required Duration) or (required Duration, optional Coordination)");
    }
};

}

#endif
