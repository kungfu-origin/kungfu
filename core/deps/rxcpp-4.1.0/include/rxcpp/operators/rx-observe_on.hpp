// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-observe_on.hpp

    \brief All values are queued and delivered using the scheduler from the supplied coordination.

    \tparam Coordination  the type of the scheduler.

    \param  cn  the scheduler to notify observers on.

    \return  The source observable modified so that its observers are notified on the specified scheduler.

    \sample
    \snippet observe_on.cpp observe_on sample
    \snippet output.txt observe_on sample

    Invoking rxcpp::observable::subscribe_on operator, instead of observe_on, gives following results:
    \snippet output.txt subscribe_on sample
*/

#if !defined(RXCPP_OPERATORS_RX_OBSERVE_ON_HPP)
#define RXCPP_OPERATORS_RX_OBSERVE_ON_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct observe_on_invalid_arguments {};

template<class... AN>
struct observe_on_invalid : public rxo::operator_base<observe_on_invalid_arguments<AN...>> {
    using type = observable<observe_on_invalid_arguments<AN...>, observe_on_invalid<AN...>>;
};
template<class... AN>
using observe_on_invalid_t = typename observe_on_invalid<AN...>::type;

template<class T, class Coordination>
struct observe_on
{
    typedef rxu::decay_t<T> source_value_type;

    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;

    coordination_type coordination;

    observe_on(coordination_type cn)
        : coordination(std::move(cn))
    {
    }

    template<class Subscriber>
    struct observe_on_observer
    {
        typedef observe_on_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;

        typedef rxn::notification<T> notification_type;
        typedef typename notification_type::type base_notification_type;
        typedef std::deque<base_notification_type> queue_type;

        struct mode
        {
            enum type {
                Invalid = 0,
                Processing,
                Empty,
                Disposed,
                Errored
            };
        };
        struct observe_on_state : std::enable_shared_from_this<observe_on_state>
        {
            mutable std::mutex lock;
            mutable queue_type fill_queue;
            mutable queue_type drain_queue;
            composite_subscription lifetime;
            mutable typename mode::type current;
            coordinator_type coordinator;
            dest_type destination;

            observe_on_state(dest_type d, coordinator_type coor, composite_subscription cs)
                : lifetime(std::move(cs))
                , current(mode::Empty)
                , coordinator(std::move(coor))
                , destination(std::move(d))
            {
            }

            void finish(std::unique_lock<std::mutex>& guard, typename mode::type end) const {
                if (!guard.owns_lock()) {
                    std::terminate();
                }
                if (current == mode::Errored || current == mode::Disposed) {return;}
                current = end;
                queue_type fill_expired;
                swap(fill_expired, fill_queue);
                queue_type drain_expired;
                swap(drain_expired, drain_queue);
                RXCPP_UNWIND_AUTO([&](){guard.lock();});
                guard.unlock();
                lifetime.unsubscribe();
                destination.unsubscribe();
            }

            void ensure_processing(std::unique_lock<std::mutex>& guard) const {
                if (!guard.owns_lock()) {
                    std::terminate();
                }
                if (current == mode::Empty) {
                    current = mode::Processing;

                    if (!lifetime.is_subscribed() && fill_queue.empty() && drain_queue.empty()) {
                        finish(guard, mode::Disposed);
                    }

                    auto keepAlive = this->shared_from_this();

                    auto drain = [keepAlive, this](const rxsc::schedulable& self){
                        using std::swap;
                        try {
                            for (;;) {
                                if (drain_queue.empty() || !destination.is_subscribed()) {
                                    std::unique_lock<std::mutex> guard(lock);
                                    if (!destination.is_subscribed() ||
                                        (!lifetime.is_subscribed() && fill_queue.empty() && drain_queue.empty())) {
                                        finish(guard, mode::Disposed);
                                        return;
                                    }
                                    if (drain_queue.empty()) {
                                        if (fill_queue.empty()) {
                                            current = mode::Empty;
                                            return;
                                        }
                                        swap(fill_queue, drain_queue);
                                    }
                                }
                                auto notification = std::move(drain_queue.front());
                                drain_queue.pop_front();
                                notification->accept(destination);
                                std::unique_lock<std::mutex> guard(lock);
                                self();
                                if (lifetime.is_subscribed()) break;
                            }
                        } catch(...) {
                            destination.on_error(std::current_exception());
                            std::unique_lock<std::mutex> guard(lock);
                            finish(guard, mode::Errored);
                        }
                    };

                    auto selectedDrain = on_exception(
                        [&](){return coordinator.act(drain);},
                        destination);
                    if (selectedDrain.empty()) {
                        finish(guard, mode::Errored);
                        return;
                    }

                    auto processor = coordinator.get_worker();

                    RXCPP_UNWIND_AUTO([&](){guard.lock();});
                    guard.unlock();

                    processor.schedule(selectedDrain.get());
                }
            }
        };
        std::shared_ptr<observe_on_state> state;

        observe_on_observer(dest_type d, coordinator_type coor, composite_subscription cs)
            : state(std::make_shared<observe_on_state>(std::move(d), std::move(coor), std::move(cs)))
        {
        }

        void on_next(source_value_type v) const {
            std::unique_lock<std::mutex> guard(state->lock);
            if (state->current == mode::Errored || state->current == mode::Disposed) { return; }
            state->fill_queue.push_back(notification_type::on_next(std::move(v)));
            state->ensure_processing(guard);
        }
        void on_error(std::exception_ptr e) const {
            std::unique_lock<std::mutex> guard(state->lock);
            if (state->current == mode::Errored || state->current == mode::Disposed) { return; }
            state->fill_queue.push_back(notification_type::on_error(e));
            state->ensure_processing(guard);
        }
        void on_completed() const {
            std::unique_lock<std::mutex> guard(state->lock);
            if (state->current == mode::Errored || state->current == mode::Disposed) { return; }
            state->fill_queue.push_back(notification_type::on_completed());
            state->ensure_processing(guard);
        }

        static subscriber<value_type, observer<value_type, this_type>> make(dest_type d, coordination_type cn, composite_subscription cs = composite_subscription()) {
            auto coor = cn.create_coordinator(d.get_subscription());
            d.add(cs);

            this_type o(d, std::move(coor), cs);
            auto keepAlive = o.state;
            cs.add([=](){
                std::unique_lock<std::mutex> guard(keepAlive->lock);
                keepAlive->ensure_processing(guard);
            });

            return make_subscriber<value_type>(d, cs, make_observer<value_type>(std::move(o)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(observe_on_observer<decltype(dest.as_dynamic())>::make(dest.as_dynamic(), coordination)) {
        return      observe_on_observer<decltype(dest.as_dynamic())>::make(dest.as_dynamic(), coordination);
    }
};

}

/*! @copydoc rx-observe_on.hpp
*/
template<class... AN>
auto observe_on(AN&&... an)
    ->      operator_factory<observe_on_tag, AN...> {
     return operator_factory<observe_on_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<observe_on_tag>
{
    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class ObserveOn = rxo::detail::observe_on<SourceValue, rxu::decay_t<Coordination>>>
    static auto member(Observable&& o, Coordination&& cn)
        -> decltype(o.template lift<SourceValue>(ObserveOn(std::forward<Coordination>(cn)))) {
        return      o.template lift<SourceValue>(ObserveOn(std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::observe_on_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "observe_on takes (Coordination)");
    }
};

class observe_on_one_worker : public coordination_base
{
    rxsc::scheduler factory;

    class input_type
    {
        rxsc::worker controller;
        rxsc::scheduler factory;
        identity_one_worker coordination;
    public:
        explicit input_type(rxsc::worker w)
            : controller(w)
            , factory(rxsc::make_same_worker(w))
            , coordination(factory)
        {
        }
        inline rxsc::worker get_worker() const {
            return controller;
        }
        inline rxsc::scheduler get_scheduler() const {
            return factory;
        }
        inline rxsc::scheduler::clock_type::time_point now() const {
            return factory.now();
        }
        template<class Observable>
        auto in(Observable o) const
            -> decltype(o.observe_on(coordination)) {
            return      o.observe_on(coordination);
        }
        template<class Subscriber>
        auto out(Subscriber s) const
            -> Subscriber {
            return s;
        }
        template<class F>
        auto act(F f) const
            -> F {
            return f;
        }
    };

public:

    explicit observe_on_one_worker(rxsc::scheduler sc) : factory(sc) {}

    typedef coordinator<input_type> coordinator_type;

    inline rxsc::scheduler::clock_type::time_point now() const {
        return factory.now();
    }

    inline coordinator_type create_coordinator(composite_subscription cs = composite_subscription()) const {
        auto w = factory.create_worker(std::move(cs));
        return coordinator_type(input_type(std::move(w)));
    }
};

inline observe_on_one_worker observe_on_run_loop(const rxsc::run_loop& rl) {
    static observe_on_one_worker r(rxsc::make_run_loop(rl));
    return r;
}

inline observe_on_one_worker observe_on_event_loop() {
    static observe_on_one_worker r(rxsc::make_event_loop());
    return r;
}

inline observe_on_one_worker observe_on_new_thread() {
    static observe_on_one_worker r(rxsc::make_new_thread());
    return r;
}

}

#endif
