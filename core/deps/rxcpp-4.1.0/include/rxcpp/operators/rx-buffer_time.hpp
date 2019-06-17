// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-buffer_time.hpp

    \brief Return an observable that emits buffers every period time interval and collects items from this observable for period of time into each produced buffer.
           If the skip parameter is set, Return an observable that emits buffers every skip time interval and collects items from this observable for period of time into each produced buffer, on the specified scheduler.

    \tparam Duration      the type of the time interval
    \tparam Coordination  the type of the scheduler (optional).

    \param period        the period of time each buffer collects items before it is emitted.
    \param skip          the period of time after which a new buffer will be created (optional).
    \param coordination  the scheduler for the buffers (optional).

    \return  Observable that emits buffers every period time interval and collect items from this observable for period of time into each produced buffer.
             If the skip parameter is set, return an Observable that emits buffers every skip time interval and collect items from this observable for period of time into each produced buffer.

    \sample
    \snippet buffer.cpp buffer period+skip+coordination sample
    \snippet output.txt buffer period+skip+coordination sample

    \sample
    \snippet buffer.cpp buffer period+skip sample
    \snippet output.txt buffer period+skip sample

    Overlapping buffers are allowed:
    \snippet buffer.cpp buffer period+skip overlapping sample
    \snippet output.txt buffer period+skip overlapping sample

    If no items are emitted, an empty buffer is returned:
    \snippet buffer.cpp buffer period+skip empty sample
    \snippet output.txt buffer period+skip empty sample

    \sample
    \snippet buffer.cpp buffer period+coordination sample
    \snippet output.txt buffer period+coordination sample

    \sample
    \snippet buffer.cpp buffer period sample
    \snippet output.txt buffer period sample
*/

#if !defined(RXCPP_OPERATORS_RX_BUFFER_WITH_TIME_HPP)
#define RXCPP_OPERATORS_RX_BUFFER_WITH_TIME_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct buffer_with_time_invalid_arguments {};

template<class... AN>
struct buffer_with_time_invalid : public rxo::operator_base<buffer_with_time_invalid_arguments<AN...>> {
    using type = observable<buffer_with_time_invalid_arguments<AN...>, buffer_with_time_invalid<AN...>>;
};
template<class... AN>
using buffer_with_time_invalid_t = typename buffer_with_time_invalid<AN...>::type;

template<class T, class Duration, class Coordination>
struct buffer_with_time
{
    typedef rxu::decay_t<T> source_value_type;
    typedef std::vector<source_value_type> value_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    typedef rxu::decay_t<Duration> duration_type;

    struct buffer_with_time_values
    {
        buffer_with_time_values(duration_type p, duration_type s, coordination_type c)
            : period(p)
            , skip(s)
            , coordination(c)
        {
        }
        duration_type period;
        duration_type skip;
        coordination_type coordination;
    };
    buffer_with_time_values initial;

    buffer_with_time(duration_type period, duration_type skip, coordination_type coordination)
        : initial(period, skip, coordination)
    {
    }

    template<class Subscriber>
    struct buffer_with_time_observer
    {
        typedef buffer_with_time_observer<Subscriber> this_type;
        typedef std::vector<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;

        struct buffer_with_time_subscriber_values : public buffer_with_time_values
        {
            buffer_with_time_subscriber_values(composite_subscription cs, dest_type d, buffer_with_time_values v, coordinator_type c)
                : buffer_with_time_values(v)
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
            mutable std::deque<value_type> chunks;
            rxsc::scheduler::clock_type::time_point expected;
        };
        std::shared_ptr<buffer_with_time_subscriber_values> state;

        buffer_with_time_observer(composite_subscription cs, dest_type d, buffer_with_time_values v, coordinator_type c)
            : state(std::make_shared<buffer_with_time_subscriber_values>(buffer_with_time_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
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

            auto produce_buffer = [localState](const rxsc::schedulable&) {
                localState->dest.on_next(std::move(localState->chunks.front()));
                localState->chunks.pop_front();
            };
            auto selectedProduce = on_exception(
                [&](){return localState->coordinator.act(produce_buffer);},
                localState->dest);
            if (selectedProduce.empty()) {
                return;
            }

            auto create_buffer = [localState, selectedProduce](const rxsc::schedulable&) {
                localState->chunks.emplace_back();
                auto produce_at = localState->expected + localState->period;
                localState->expected += localState->skip;
                localState->worker.schedule(produce_at, [localState, selectedProduce](const rxsc::schedulable&) {
                    localState->worker.schedule(selectedProduce.get());
                });
            };
            auto selectedCreate = on_exception(
                [&](){return localState->coordinator.act(create_buffer);},
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
                for(auto& chunk : localState->chunks) {
                    chunk.push_back(v);
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
                on_exception(
                    [&](){
                        while (!localState->chunks.empty()) {
                            localState->dest.on_next(std::move(localState->chunks.front()));
                            localState->chunks.pop_front();
                        }
                        return true;
                    },
                    localState->dest);
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

        static subscriber<T, observer<T, this_type>> make(dest_type d, buffer_with_time_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator();

            return make_subscriber<T>(cs, this_type(cs, std::move(d), std::move(v), std::move(coordinator)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(buffer_with_time_observer<Subscriber>::make(std::move(dest), initial)) {
        return      buffer_with_time_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-buffer_time.hpp
*/
template<class... AN>
auto buffer_with_time(AN&&... an)
    ->      operator_factory<buffer_with_time_tag, AN...> {
     return operator_factory<buffer_with_time_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<buffer_with_time_tag>
{
    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferWithTime = rxo::detail::buffer_with_time<SourceValue, rxu::decay_t<Duration>, identity_one_worker>,
        class Value = rxu::value_type_t<BufferWithTime>>
    static auto member(Observable&& o, Duration period)
        -> decltype(o.template lift<Value>(BufferWithTime(period, period, identity_current_thread()))) {
        return      o.template lift<Value>(BufferWithTime(period, period, identity_current_thread()));
    }

    template<class Observable, class Duration, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferWithTime = rxo::detail::buffer_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<BufferWithTime>>
    static auto member(Observable&& o, Duration period, Coordination&& cn)
        -> decltype(o.template lift<Value>(BufferWithTime(period, period, std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(BufferWithTime(period, period, std::forward<Coordination>(cn)));
    }

    template<class Observable, class Duration,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferWithTime = rxo::detail::buffer_with_time<SourceValue, rxu::decay_t<Duration>, identity_one_worker>,
        class Value = rxu::value_type_t<BufferWithTime>>
    static auto member(Observable&& o, Duration&& period, Duration&& skip)
        -> decltype(o.template lift<Value>(BufferWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), identity_current_thread()))) {
        return      o.template lift<Value>(BufferWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), identity_current_thread()));
    }

    template<class Observable, class Duration, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            std::is_convertible<Duration, rxsc::scheduler::clock_type::duration>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferWithTime = rxo::detail::buffer_with_time<SourceValue, rxu::decay_t<Duration>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<BufferWithTime>>
    static auto member(Observable&& o, Duration&& period, Duration&& skip, Coordination&& cn)
        -> decltype(o.template lift<Value>(BufferWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(BufferWithTime(std::forward<Duration>(period), std::forward<Duration>(skip), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::buffer_with_time_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "buffer_with_time takes (Duration, optional Duration, optional Coordination)");
    }
};

}

#endif
