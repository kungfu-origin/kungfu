// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-window_toggle.hpp

    \brief Return an observable that emits observables every period time interval and collects items from this observable for period of time into each produced observable, on the specified scheduler.

    \tparam Openings        observable<OT>
    \tparam ClosingSelector a function of type observable<CT>(OT)
    \tparam Coordination    the type of the scheduler (optional).

    \param opens         each value from this observable opens a new window.
    \param closes        this function is called for each opened window and returns an observable. the first value from the returned observable will close the window.
    \param coordination  the scheduler for the windows (optional).

    \return  Observable that emits an observable for each opened window.

    \sample
    \snippet window.cpp window toggle+coordination sample
    \snippet output.txt window toggle+coordination sample

    \sample
    \snippet window.cpp window toggle sample
    \snippet output.txt window toggle sample
*/

#if !defined(RXCPP_OPERATORS_RX_WINDOW_TOGGLE_HPP)
#define RXCPP_OPERATORS_RX_WINDOW_TOGGLE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct window_toggle_invalid_arguments {};

template<class... AN>
struct window_toggle_invalid : public rxo::operator_base<window_toggle_invalid_arguments<AN...>> {
    using type = observable<window_toggle_invalid_arguments<AN...>, window_toggle_invalid<AN...>>;
};
template<class... AN>
using window_toggle_invalid_t = typename window_toggle_invalid<AN...>::type;

template<class T, class Openings, class ClosingSelector, class Coordination>
struct window_toggle
{
    typedef window_toggle<T, Openings, ClosingSelector, Coordination> this_type;

    using source_value_type = rxu::decay_t<T>;
    using coordination_type = rxu::decay_t<Coordination>;
    using coordinator_type = typename coordination_type::coordinator_type;
    using openings_type = rxu::decay_t<Openings>;
    using openings_value_type = typename openings_type::value_type;
    using closing_selector_type = rxu::decay_t<ClosingSelector>;
    using closings_type = rxu::result_of_t<closing_selector_type(openings_value_type)>;
    using closings_value_type = typename closings_type::value_type;

    struct window_toggle_values
    {
        window_toggle_values(openings_type opens, closing_selector_type closes, coordination_type c)
            : openings(opens)
            , closingSelector(closes)
            , coordination(c)
        {
        }
        openings_type openings;
        mutable closing_selector_type closingSelector;
        coordination_type coordination;
    };
    window_toggle_values initial;

    window_toggle(openings_type opens, closing_selector_type closes, coordination_type coordination)
        : initial(opens, closes, coordination)
    {
    }

    template<class Subscriber>
    struct window_toggle_observer
    {
        typedef window_toggle_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;

        struct window_toggle_subscriber_values : public window_toggle_values
        {
            window_toggle_subscriber_values(composite_subscription cs, dest_type d, window_toggle_values v, coordinator_type c)
                : window_toggle_values(v)
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
            mutable std::list<rxcpp::subjects::subject<T>> subj;
        };
        std::shared_ptr<window_toggle_subscriber_values> state;

        window_toggle_observer(composite_subscription cs, dest_type d, window_toggle_values v, coordinator_type c)
            : state(std::make_shared<window_toggle_subscriber_values>(window_toggle_subscriber_values(std::move(cs), std::move(d), v, std::move(c))))
        {
            auto localState = state;

            composite_subscription innercs;

            // when the out observer is unsubscribed all the
            // inner subscriptions are unsubscribed as well
            auto innerscope = localState->dest.add(innercs);

            innercs.add([=](){
                localState->dest.remove(innerscope);
            });

            localState->dest.add(localState->cs);

            auto source = on_exception(
                [&](){return localState->coordinator.in(localState->openings);},
                localState->dest);
            if (source.empty()) {
                return;
            }

            // this subscribe does not share the observer subscription
            // so that when it is unsubscribed the observer can be called
            // until the inner subscriptions have finished
            auto sink = make_subscriber<openings_value_type>(
                localState->dest,
                innercs,
            // on_next
                [localState](const openings_value_type& ov) {
                    auto closer = localState->closingSelector(ov);

                    auto it = localState->subj.insert(localState->subj.end(), rxcpp::subjects::subject<T>());
                    localState->dest.on_next(it->get_observable().as_dynamic());

                    composite_subscription innercs;

                    // when the out observer is unsubscribed all the
                    // inner subscriptions are unsubscribed as well
                    auto innerscope = localState->dest.add(innercs);

                    innercs.add([=](){
                        localState->dest.remove(innerscope);
                    });

                    auto source = localState->coordinator.in(closer);

                    auto sit = std::make_shared<decltype(it)>(it);
                    auto close = [localState, sit]() {
                        auto it = *sit;
                        *sit = localState->subj.end();
                        if (it != localState->subj.end()) {
                            it->get_subscriber().on_completed();
                            localState->subj.erase(it);
                        }
                    };

                    // this subscribe does not share the observer subscription
                    // so that when it is unsubscribed the observer can be called
                    // until the inner subscriptions have finished
                    auto sink = make_subscriber<closings_value_type>(
                        localState->dest,
                        innercs,
                    // on_next
                        [close, innercs](closings_value_type) {
                            close();
                            innercs.unsubscribe();
                        },
                    // on_error
                        [localState](std::exception_ptr e) {
                            localState->dest.on_error(e);
                        },
                    // on_completed
                        close
                    );
                    auto selectedSink = localState->coordinator.out(sink);
                    source.subscribe(std::move(selectedSink));
                },
            // on_error
                [localState](std::exception_ptr e) {
                    localState->dest.on_error(e);
                },
            // on_completed
                []() {
                }
            );
            auto selectedSink = on_exception(
                [&](){return localState->coordinator.out(sink);},
                localState->dest);
            if (selectedSink.empty()) {
                return;
            }
            source->subscribe(std::move(selectedSink.get()));
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

        static subscriber<T, observer_type> make(dest_type d, window_toggle_values v) {
            auto cs = composite_subscription();
            auto coordinator = v.coordination.create_coordinator(d.get_subscription());

            return make_subscriber<T>(cs, observer_type(this_type(cs, std::move(d), std::move(v), std::move(coordinator))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(window_toggle_observer<Subscriber>::make(std::move(dest), initial)) {
        return      window_toggle_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-window_toggle.hpp
*/
template<class... AN>
auto window_toggle(AN&&... an)
    ->      operator_factory<window_toggle_tag, AN...> {
     return operator_factory<window_toggle_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<window_toggle_tag>
{
    template<class Observable, class Openings, class ClosingSelector,
        class ClosingSelectorType = rxu::decay_t<ClosingSelector>,
        class OpeningsType = rxu::decay_t<Openings>,
        class OpeningsValueType = typename OpeningsType::value_type,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, Openings, rxu::result_of_t<ClosingSelectorType(OpeningsValueType)>>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowToggle = rxo::detail::window_toggle<SourceValue, rxu::decay_t<Openings>, rxu::decay_t<ClosingSelector>, identity_one_worker>,
        class Value = observable<SourceValue>>
    static auto member(Observable&& o, Openings&& openings, ClosingSelector&& closingSelector)
        -> decltype(o.template lift<Value>(WindowToggle(std::forward<Openings>(openings), std::forward<ClosingSelector>(closingSelector), identity_immediate()))) {
        return      o.template lift<Value>(WindowToggle(std::forward<Openings>(openings), std::forward<ClosingSelector>(closingSelector), identity_immediate()));
    }

    template<class Observable, class Openings, class ClosingSelector, class Coordination,
        class ClosingSelectorType = rxu::decay_t<ClosingSelector>,
        class OpeningsType = rxu::decay_t<Openings>,
        class OpeningsValueType = typename OpeningsType::value_type,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, Openings, rxu::result_of_t<ClosingSelectorType(OpeningsValueType)>>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class WindowToggle = rxo::detail::window_toggle<SourceValue, rxu::decay_t<Openings>, rxu::decay_t<ClosingSelector>, rxu::decay_t<Coordination>>,
        class Value = observable<SourceValue>>
    static auto member(Observable&& o, Openings&& openings, ClosingSelector&& closingSelector, Coordination&& cn)
        -> decltype(o.template lift<Value>(WindowToggle(std::forward<Openings>(openings), std::forward<ClosingSelector>(closingSelector), std::forward<Coordination>(cn)))) {
        return      o.template lift<Value>(WindowToggle(std::forward<Openings>(openings), std::forward<ClosingSelector>(closingSelector), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::window_toggle_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "window_toggle takes (Openings, ClosingSelector, optional Coordination)");
    }
};

}

#endif
