// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-debounce.hpp

    \brief  Return an observable that emits an item if a particular timespan has passed without emitting another item from the source observable.

    \tparam Duration      the type of the time interval
    \tparam Coordination  the type of the scheduler

    \param period        the period of time to suppress any emitted items
    \param coordination  the scheduler to manage timeout for each event

    \return  Observable that emits an item if a particular timespan has passed without emitting another item from the source observable.

    \sample
    \snippet debounce.cpp debounce sample
    \snippet output.txt debounce sample
*/

#if !defined(RXCPP_OPERATORS_RX_DEBOUNCE_HPP)
#define RXCPP_OPERATORS_RX_DEBOUNCE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct debounce_invalid_arguments {};

template<class... AN>
struct debounce_invalid : public rxo::operator_base<debounce_invalid_arguments<AN...>> {
    using type = observable<debounce_invalid_arguments<AN...>, debounce_invalid<AN...>>;
};
template<class... AN>
using debounce_invalid_t = typename debounce_invalid<AN...>::type;

template<class T, class Duration, class Coordination>
struct debounce
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct debounce_values
    {
        debounce_values(duration_type p, coordination_type c)
            : period(p)
            , coordination(c)
        {
        }

        duration_type period;
        coordination_type coordination;
    };
    debounce_values initial;

    debounce(duration_type period, coordination_type coordination)
        : initial(period, coordination)
    {
    }

    template<class Subscriber>
    struct debounce_observer
    {
        typedef debounce_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;

        struct debounce_subscriber_values : public debounce_values
        {
            debounce_subscriber_values(composite_subscription cs, dest_type d, debounce_values v, coordinator_type c)
                : debounce_values(v)
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
            mutable rxu::maybe<value_type> value;
        };
        typedef std::shared_ptr<debounce_subscriber_values> state_type;
        state_type state;

        debounce_observer(composite_subscription cs, dest_type d, debounce_values v, coordinator_type c)
            : state(std::make_shared<debounce_subscriber_values>(debounce_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
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
        }

        static std::function<void(const rxsc::schedulable&)> produce_item(std::size_t id, state_type state) {
            auto produce = [id, state](const rxsc::schedulable&) {
                if(id != state->index)
                    return;

                state->dest.on_next(*state->value);
                state->value.reset();
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

                localState->value.reset(v);
                localState->worker.schedule(produce_time, produce_item(new_id, localState));
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
                localState->value.reset();
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
                if(!localState->value.empty()) {
                    localState->dest.on_next(*localState->value);
                }
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

        static subscriber<T, observer_type> make(dest_type d, debounce_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, observer_type(this_type(cs, std::move(d), std::move(v), std::move(coordinator))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(debounce_observer<Subscriber>::make(std::move(dest), initial)) {
        return      debounce_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-debounce.hpp
*/
template<class... AN>
auto debounce(AN&&... an)
    ->      operator_factory<debounce_tag, AN...> {
     return operator_factory<debounce_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<debounce_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Debounce = rxo::detail::debounce<SourceValue, rxu::decay_t<Duration>, identity_one_worker>>
    static auto member(Observable&& o, Duration&& d)
        -> decltype(o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), identity_current_thread()))) {
        return      o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), identity_current_thread()));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Debounce = rxo::detail::debounce<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Coordination&& cn, Duration&& d)
        -> decltype(o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class Observable, class Coordination, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>,
            rxu::is_duration<Duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Debounce = rxo::detail::debounce<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Duration&& d, Coordination&& cn)
        -> decltype(o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(Debounce(std::forward<Duration>(d), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::debounce_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "debounce takes (optional Coordination, required Duration) or (required Duration, optional Coordination)");
    }
};

}

#endif
