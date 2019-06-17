// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-combine_latest.hpp

    \brief For each item from all of the observables select a value to emit from the new observable that is returned.

    \tparam AN  types of scheduler (optional), aggregate function (optional), and source observables

    \param  an  scheduler (optional), aggregation function (optional), and source observables

    \return  Observable that emits items that are the result of combining the items emitted by the source observables.

    If scheduler is omitted, identity_current_thread is used.

    If aggregation function is omitted, the resulting observable returns tuples of emitted items.

    \sample

    Neither scheduler nor aggregation function are present:
    \snippet combine_latest.cpp combine_latest sample
    \snippet output.txt combine_latest sample

    Only scheduler is present:
    \snippet combine_latest.cpp Coordination combine_latest sample
    \snippet output.txt Coordination combine_latest sample

    Only aggregation function is present:
    \snippet combine_latest.cpp Selector combine_latest sample
    \snippet output.txt Selector combine_latest sample

    Both scheduler and aggregation function are present:
    \snippet combine_latest.cpp Coordination+Selector combine_latest sample
    \snippet output.txt Coordination+Selector combine_latest sample
*/

#if !defined(RXCPP_OPERATORS_RX_COMBINE_LATEST_HPP)
#define RXCPP_OPERATORS_RX_COMBINE_LATEST_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct combine_latest_invalid_arguments {};

template<class... AN>
struct combine_latest_invalid : public rxo::operator_base<combine_latest_invalid_arguments<AN...>> {
    using type = observable<combine_latest_invalid_arguments<AN...>, combine_latest_invalid<AN...>>;
};
template<class... AN>
using combine_latest_invalid_t = typename combine_latest_invalid<AN...>::type;

template<class Selector, class... ObservableN>
struct is_combine_latest_selector_check {
    typedef rxu::decay_t<Selector> selector_type;

    struct tag_not_valid;
    template<class CS, class... CON>
    static auto check(int) -> decltype((*(CS*)nullptr)((*(typename CON::value_type*)nullptr)...));
    template<class CS, class... CON>
    static tag_not_valid check(...);

    using type = decltype(check<selector_type, rxu::decay_t<ObservableN>...>(0));

    static const bool value = !std::is_same<type, tag_not_valid>::value;
};

template<class Selector, class... ObservableN>
struct invalid_combine_latest_selector {
    static const bool value = false;
};

template<class Selector, class... ObservableN>
struct is_combine_latest_selector : public std::conditional<
    is_combine_latest_selector_check<Selector, ObservableN...>::value, 
    is_combine_latest_selector_check<Selector, ObservableN...>, 
    invalid_combine_latest_selector<Selector, ObservableN...>>::type {
};

template<class Selector, class... ON>
using result_combine_latest_selector_t = typename is_combine_latest_selector<Selector, ON...>::type;

template<class Coordination, class Selector, class... ObservableN>
struct combine_latest_traits {

    typedef std::tuple<ObservableN...> tuple_source_type;
    typedef std::tuple<rxu::detail::maybe<typename ObservableN::value_type>...> tuple_source_value_type;

    typedef rxu::decay_t<Selector> selector_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    typedef typename is_combine_latest_selector<selector_type, ObservableN...>::type value_type;
};

template<class Coordination, class Selector, class... ObservableN>
struct combine_latest : public operator_base<rxu::value_type_t<combine_latest_traits<Coordination, Selector, ObservableN...>>>
{
    typedef combine_latest<Coordination, Selector, ObservableN...> this_type;

    typedef combine_latest_traits<Coordination, Selector, ObservableN...> traits;

    typedef typename traits::tuple_source_type tuple_source_type;
    typedef typename traits::tuple_source_value_type tuple_source_value_type;

    typedef typename traits::selector_type selector_type;

    typedef typename traits::coordination_type coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;

    struct values
    {
        values(tuple_source_type o, selector_type s, coordination_type sf)
            : source(std::move(o))
            , selector(std::move(s))
            , coordination(std::move(sf))
        {
        }
        tuple_source_type source;
        selector_type selector;
        coordination_type coordination;
    };
    values initial;

    combine_latest(coordination_type sf, selector_type s, tuple_source_type ts)
        : initial(std::move(ts), std::move(s), std::move(sf))
    {
    }

    template<int Index, class State>
    void subscribe_one(std::shared_ptr<State> state) const {

        typedef typename std::tuple_element<Index, tuple_source_type>::type::value_type source_value_type;

        composite_subscription innercs;

        // when the out observer is unsubscribed all the
        // inner subscriptions are unsubscribed as well
        state->out.add(innercs);

        auto source = on_exception(
            [&](){return state->coordinator.in(std::get<Index>(state->source));},
            state->out);
        if (source.empty()) {
            return;
        }

        // this subscribe does not share the observer subscription
        // so that when it is unsubscribed the observer can be called
        // until the inner subscriptions have finished
        auto sink = make_subscriber<source_value_type>(
            state->out,
            innercs,
        // on_next
            [state](source_value_type st) {
                auto& value = std::get<Index>(state->latest);

                if (value.empty()) {
                    ++state->valuesSet;
                }

                value.reset(st);

                if (state->valuesSet == sizeof... (ObservableN)) {
                    auto values = rxu::surely(state->latest);
                    auto selectedResult = rxu::apply(values, state->selector);
                    state->out.on_next(selectedResult);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (--state->pendingCompletions == 0) {
                    state->out.on_completed();
                }
            }
        );
        auto selectedSink = on_exception(
            [&](){return state->coordinator.out(sink);},
            state->out);
        if (selectedSink.empty()) {
            return;
        }
        source->subscribe(std::move(selectedSink.get()));
    }
    template<class State, int... IndexN>
    void subscribe_all(std::shared_ptr<State> state, rxu::values<int, IndexN...>) const {
        bool subscribed[] = {(subscribe_one<IndexN>(state), true)...};
        subscribed[0] = (*subscribed); // silence warning
    }

    template<class Subscriber>
    void on_subscribe(Subscriber scbr) const {
        static_assert(is_subscriber<Subscriber>::value, "subscribe must be passed a subscriber");

        typedef Subscriber output_type;

        struct combine_latest_state_type
            : public std::enable_shared_from_this<combine_latest_state_type>
            , public values
        {
            combine_latest_state_type(values i, coordinator_type coor, output_type oarg)
                : values(std::move(i))
                , pendingCompletions(sizeof... (ObservableN))
                , valuesSet(0)
                , coordinator(std::move(coor))
                , out(std::move(oarg))
            {
            }

            // on_completed on the output must wait until all the
            // subscriptions have received on_completed
            mutable int pendingCompletions;
            mutable int valuesSet;
            mutable tuple_source_value_type latest;
            coordinator_type coordinator;
            output_type out;
        };

        auto coordinator = initial.coordination.create_coordinator(scbr.get_subscription());

        // take a copy of the values for each subscription
        auto state = std::make_shared<combine_latest_state_type>(initial, std::move(coordinator), std::move(scbr));

        subscribe_all(state, typename rxu::values_from<int, sizeof...(ObservableN)>::type());
    }
};

}

/*! @copydoc rx-combine_latest.hpp
*/
template<class... AN>
auto combine_latest(AN&&... an) 
    ->     operator_factory<combine_latest_tag, AN...> {
    return operator_factory<combine_latest_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<> 
struct member_overload<combine_latest_tag>
{
    template<class Observable, class... ObservableN, 
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, ObservableN...>>,
        class combine_latest = rxo::detail::combine_latest<identity_one_worker, rxu::detail::pack, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<combine_latest>,
        class Result = observable<Value, combine_latest>>
    static Result member(Observable&& o, ObservableN&&... on)
    {
        return Result(combine_latest(identity_current_thread(), rxu::pack(), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Observable, class Selector, class... ObservableN,
        class Enabled = rxu::enable_if_all_true_type_t<
            operators::detail::is_combine_latest_selector<Selector, Observable, ObservableN...>,
            all_observables<Observable, ObservableN...>>,
        class ResolvedSelector = rxu::decay_t<Selector>,
        class combine_latest = rxo::detail::combine_latest<identity_one_worker, ResolvedSelector, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<combine_latest>,
        class Result = observable<Value, combine_latest>>
    static Result member(Observable&& o, Selector&& s, ObservableN&&... on)
    {
        return Result(combine_latest(identity_current_thread(), std::forward<Selector>(s), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Coordination, class Observable, class... ObservableN, 
        class Enabled = rxu::enable_if_all_true_type_t<
            is_coordination<Coordination>,
            all_observables<Observable, ObservableN...>>,
        class combine_latest = rxo::detail::combine_latest<Coordination, rxu::detail::pack, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<combine_latest>,
        class Result = observable<Value, combine_latest>>
    static Result member(Observable&& o, Coordination&& cn, ObservableN&&... on)
    {
        return Result(combine_latest(std::forward<Coordination>(cn), rxu::pack(), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Coordination, class Selector, class Observable, class... ObservableN,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_coordination<Coordination>,
            operators::detail::is_combine_latest_selector<Selector, Observable, ObservableN...>,
            all_observables<Observable, ObservableN...>>,
        class ResolvedSelector = rxu::decay_t<Selector>,
        class combine_latest = rxo::detail::combine_latest<Coordination, ResolvedSelector, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<combine_latest>,
        class Result = observable<Value, combine_latest>>
    static Result member(Observable&& o, Coordination&& cn, Selector&& s, ObservableN&&... on)
    {
        return Result(combine_latest(std::forward<Coordination>(cn), std::forward<Selector>(s), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class... AN>
    static operators::detail::combine_latest_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "combine_latest takes (optional Coordination, optional Selector, required Observable, optional Observable...), Selector takes (Observable::value_type...)");
    } 
};

}

#endif
