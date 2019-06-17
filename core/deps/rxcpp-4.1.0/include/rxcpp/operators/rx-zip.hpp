// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_OPERATORS_RX_ZIP_HPP)
#define RXCPP_OPERATORS_RX_ZIP_HPP

#include "../rx-includes.hpp"

/*! \file rx-zip.hpp

    \brief Bring by one item from all given observables and select a value to emit from the new observable that is returned.

    \tparam AN  types of scheduler (optional), aggregate function (optional), and source observables

    \param  an  scheduler (optional), aggregation function (optional), and source observables

    \return  Observable that emits the result of combining the items emitted and brought by one from each of the source observables.

    If scheduler is omitted, identity_current_thread is used.

    If aggregation function is omitted, the resulting observable returns tuples of emitted items.

    \sample

    Neither scheduler nor aggregation function are present:
    \snippet zip.cpp zip sample
    \snippet output.txt zip sample

    Only scheduler is present:
    \snippet zip.cpp Coordination zip sample
    \snippet output.txt Coordination zip sample

    Only aggregation function is present:
    \snippet zip.cpp Selector zip sample
    \snippet output.txt Selector zip sample

    Both scheduler and aggregation function are present:
    \snippet zip.cpp Coordination+Selector zip sample
    \snippet output.txt Coordination+Selector zip sample
*/

namespace rxcpp {

namespace operators {

namespace detail {

template<class Observable>
struct zip_source_state
{
    using value_type = rxu::value_type_t<Observable>;
    zip_source_state() 
        : completed(false) 
    {
    }
    std::list<value_type> values;
    bool completed;
};

struct values_not_empty {
    template<class Observable>
    bool operator()(zip_source_state<Observable>& source) const {
        return !source.values.empty();
    }
};

struct source_completed_values_empty {
    template<class Observable>
    bool operator()(zip_source_state<Observable>& source) const {
        return source.completed && source.values.empty();
    }
};

struct extract_value_front {
    template<class Observable, class Value = rxu::value_type_t<Observable>>
    Value operator()(zip_source_state<Observable>& source) const {
        auto val = std::move(source.values.front());
        source.values.pop_front();
        return val;
    }
};

template<class... AN>
struct zip_invalid_arguments {};

template<class... AN>
struct zip_invalid : public rxo::operator_base<zip_invalid_arguments<AN...>> {
    using type = observable<zip_invalid_arguments<AN...>, zip_invalid<AN...>>;
};
template<class... AN>
using zip_invalid_t = typename zip_invalid<AN...>::type;

template<class Selector, class... ObservableN>
struct is_zip_selector_check {
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
struct invalid_zip_selector {
    static const bool value = false;
};

template<class Selector, class... ObservableN>
struct is_zip_selector : public std::conditional<
    is_zip_selector_check<Selector, ObservableN...>::value, 
    is_zip_selector_check<Selector, ObservableN...>, 
    invalid_zip_selector<Selector, ObservableN...>>::type {
};

template<class Selector, class... ON>
using result_zip_selector_t = typename is_zip_selector<Selector, ON...>::type;

template<class Coordination, class Selector, class... ObservableN>
struct zip_traits {
    typedef std::tuple<rxu::decay_t<ObservableN>...> tuple_source_type;
    typedef std::tuple<zip_source_state<ObservableN>...> tuple_source_values_type;

    typedef rxu::decay_t<Selector> selector_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    typedef typename is_zip_selector<selector_type, ObservableN...>::type value_type;
};

template<class Coordination, class Selector, class... ObservableN>
struct zip : public operator_base<rxu::value_type_t<zip_traits<Coordination, Selector, ObservableN...>>>
{
    typedef zip<Coordination, Selector, ObservableN...> this_type;

    typedef zip_traits<Coordination, Selector, ObservableN...> traits;

    typedef typename traits::tuple_source_type tuple_source_type;
    typedef typename traits::tuple_source_values_type tuple_source_values_type;

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

    zip(coordination_type sf, selector_type s, tuple_source_type ts)
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
                auto& values = std::get<Index>(state->pending).values;
                values.push_back(st);
                if (rxu::apply_to_each(state->pending, values_not_empty(), rxu::all_values_true())) {
                    auto selectedResult = rxu::apply_to_each(state->pending, extract_value_front(), state->selector);
                    state->out.on_next(selectedResult);
                }
                if (rxu::apply_to_each(state->pending, source_completed_values_empty(), rxu::any_value_true())) {
                    state->out.on_completed();
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                auto& completed = std::get<Index>(state->pending).completed;
                completed = true;
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

        struct zip_state_type
            : public std::enable_shared_from_this<zip_state_type>
            , public values
        {
            zip_state_type(values i, coordinator_type coor, output_type oarg)
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
            mutable tuple_source_values_type pending;
            coordinator_type coordinator;
            output_type out;
        };

        auto coordinator = initial.coordination.create_coordinator(scbr.get_subscription());

        // take a copy of the values for each subscription
        auto state = std::make_shared<zip_state_type>(initial, std::move(coordinator), std::move(scbr));

        subscribe_all(state, typename rxu::values_from<int, sizeof...(ObservableN)>::type());
    }
};

}

/*! @copydoc rx-zip.hpp
*/
template<class... AN>
auto zip(AN&&... an) 
    ->     operator_factory<zip_tag, AN...> {
    return operator_factory<zip_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<> 
struct member_overload<zip_tag>
{
    template<class Observable, class... ObservableN, 
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, ObservableN...>>,
        class Zip = rxo::detail::zip<identity_one_worker, rxu::detail::pack, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<Zip>,
        class Result = observable<Value, Zip>>
    static Result member(Observable&& o, ObservableN&&... on)
    {
        return Result(Zip(identity_current_thread(), rxu::pack(), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Observable, class Selector, class... ObservableN,
        class Enabled = rxu::enable_if_all_true_type_t<
            operators::detail::is_zip_selector<Selector, Observable, ObservableN...>,
            all_observables<Observable, ObservableN...>>,
        class ResolvedSelector = rxu::decay_t<Selector>,
        class Zip = rxo::detail::zip<identity_one_worker, ResolvedSelector, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<Zip>,
        class Result = observable<Value, Zip>>
    static Result member(Observable&& o, Selector&& s, ObservableN&&... on)
    {
        return Result(Zip(identity_current_thread(), std::forward<Selector>(s), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Coordination, class Observable, class... ObservableN, 
        class Enabled = rxu::enable_if_all_true_type_t<
            is_coordination<Coordination>,
            all_observables<Observable, ObservableN...>>,
        class Zip = rxo::detail::zip<Coordination, rxu::detail::pack, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<Zip>,
        class Result = observable<Value, Zip>>
    static Result member(Observable&& o, Coordination&& cn, ObservableN&&... on)
    {
        return Result(Zip(std::forward<Coordination>(cn), rxu::pack(), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class Coordination, class Selector, class Observable, class... ObservableN,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_coordination<Coordination>,
            operators::detail::is_zip_selector<Selector, Observable, ObservableN...>,
            all_observables<Observable, ObservableN...>>,
        class ResolvedSelector = rxu::decay_t<Selector>,
        class Zip = rxo::detail::zip<Coordination, ResolvedSelector, rxu::decay_t<Observable>, rxu::decay_t<ObservableN>...>,
        class Value = rxu::value_type_t<Zip>,
        class Result = observable<Value, Zip>>
    static Result member(Observable&& o, Coordination&& cn, Selector&& s, ObservableN&&... on)
    {
        return Result(Zip(std::forward<Coordination>(cn), std::forward<Selector>(s), std::make_tuple(std::forward<Observable>(o), std::forward<ObservableN>(on)...)));
    }

    template<class... AN>
    static operators::detail::zip_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "zip takes (optional Coordination, optional Selector, required Observable, optional Observable...), Selector takes (Observable::value_type...)");
    } 
};

}

#endif
