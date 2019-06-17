// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-flat_map.hpp

    \brief For each item from this observable use the CollectionSelector to produce an observable and subscribe to that observable.
           For each item from all of the produced observables use the ResultSelector to produce a value to emit from the new observable that is returned.

    \tparam CollectionSelector  the type of the observable producing function. CollectionSelector must be a function with the signature observable(flat_map::source_value_type)
    \tparam ResultSelector      the type of the aggregation function (optional). ResultSelector must be a function with the signature flat_map::value_type(flat_map::source_value_type, flat_map::collection_value_type).
    \tparam Coordination        the type of the scheduler (optional).

    \param  s   a function that returns an observable for each item emitted by the source observable.
    \param  rs  a function that combines one item emitted by each of the source and collection observables and returns an item to be emitted by the resulting observable (optional).
    \param  cn  the scheduler to synchronize sources from different contexts (optional).

    \return  Observable that emits the results of applying a function to a pair of values emitted by the source observable and the collection observable.

    Observables, produced by the CollectionSelector, are merged. There is another operator rxcpp::observable<T,SourceType>::flat_map that works similar but concatenates the observables.

    \sample
    \snippet flat_map.cpp flat_map sample
    \snippet output.txt flat_map sample

    \sample
    \snippet flat_map.cpp threaded flat_map sample
    \snippet output.txt threaded flat_map sample
*/

#if !defined(RXCPP_OPERATORS_RX_FLATMAP_HPP)
#define RXCPP_OPERATORS_RX_FLATMAP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct flat_map_invalid_arguments {};

template<class... AN>
struct flat_map_invalid : public rxo::operator_base<flat_map_invalid_arguments<AN...>> {
    using type = observable<flat_map_invalid_arguments<AN...>, flat_map_invalid<AN...>>;
};
template<class... AN>
using flat_map_invalid_t = typename flat_map_invalid<AN...>::type;

template<class Observable, class CollectionSelector, class ResultSelector, class Coordination>
struct flat_map_traits {
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<CollectionSelector> collection_selector_type;
    typedef rxu::decay_t<ResultSelector> result_selector_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    typedef typename source_type::value_type source_value_type;

    struct tag_not_valid {};
    template<class CV, class CCS>
    static auto collection_check(int) -> decltype((*(CCS*)nullptr)(*(CV*)nullptr));
    template<class CV, class CCS>
    static tag_not_valid collection_check(...);

    static_assert(!std::is_same<decltype(collection_check<source_value_type, collection_selector_type>(0)), tag_not_valid>::value, "flat_map CollectionSelector must be a function with the signature observable(flat_map::source_value_type)");

    typedef rxu::decay_t<decltype((*(collection_selector_type*)nullptr)((*(source_value_type*)nullptr)))> collection_type;

    static_assert(is_observable<collection_type>::value, "flat_map CollectionSelector must return an observable");

    typedef typename collection_type::value_type collection_value_type;

    template<class CV, class CCV, class CRS>
    static auto result_check(int) -> decltype((*(CRS*)nullptr)(*(CV*)nullptr, *(CCV*)nullptr));
    template<class CV, class CCV, class CRS>
    static tag_not_valid result_check(...);

    static_assert(!std::is_same<decltype(result_check<source_value_type, collection_value_type, result_selector_type>(0)), tag_not_valid>::value, "flat_map ResultSelector must be a function with the signature flat_map::value_type(flat_map::source_value_type, flat_map::collection_value_type)");

    typedef rxu::decay_t<decltype((*(result_selector_type*)nullptr)(*(source_value_type*)nullptr, *(collection_value_type*)nullptr))> value_type;
};

template<class Observable, class CollectionSelector, class ResultSelector, class Coordination>
struct flat_map
    : public operator_base<rxu::value_type_t<flat_map_traits<Observable, CollectionSelector, ResultSelector, Coordination>>>
{
    typedef flat_map<Observable, CollectionSelector, ResultSelector, Coordination> this_type;
    typedef flat_map_traits<Observable, CollectionSelector, ResultSelector, Coordination> traits;

    typedef typename traits::source_type source_type;
    typedef typename traits::collection_selector_type collection_selector_type;
    typedef typename traits::result_selector_type result_selector_type;

    typedef typename traits::source_value_type source_value_type;
    typedef typename traits::collection_type collection_type;
    typedef typename traits::collection_value_type collection_value_type;

    typedef typename traits::coordination_type coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;

    struct values
    {
        values(source_type o, collection_selector_type s, result_selector_type rs, coordination_type sf)
            : source(std::move(o))
            , selectCollection(std::move(s))
            , selectResult(std::move(rs))
            , coordination(std::move(sf))
        {
        }
        source_type source;
        collection_selector_type selectCollection;
        result_selector_type selectResult;
        coordination_type coordination;
    };
    values initial;

    flat_map(source_type o, collection_selector_type s, result_selector_type rs, coordination_type sf)
        : initial(std::move(o), std::move(s), std::move(rs), std::move(sf))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber scbr) const {
        static_assert(is_subscriber<Subscriber>::value, "subscribe must be passed a subscriber");

        typedef Subscriber output_type;

        struct state_type
            : public std::enable_shared_from_this<state_type>
            , public values
        {
            state_type(values i, coordinator_type coor, output_type oarg)
                : values(std::move(i))
                , pendingCompletions(0)
                , coordinator(std::move(coor))
                , out(std::move(oarg))
            {
            }
            // on_completed on the output must wait until all the
            // subscriptions have received on_completed
            int pendingCompletions;
            coordinator_type coordinator;
            output_type out;
        };

        auto coordinator = initial.coordination.create_coordinator(scbr.get_subscription());

        // take a copy of the values for each subscription
        auto state = std::make_shared<state_type>(initial, std::move(coordinator), std::move(scbr));

        composite_subscription outercs;

        // when the out observer is unsubscribed all the
        // inner subscriptions are unsubscribed as well
        state->out.add(outercs);

        auto source = on_exception(
            [&](){return state->coordinator.in(state->source);},
            state->out);
        if (source.empty()) {
            return;
        }

        ++state->pendingCompletions;
        // this subscribe does not share the observer subscription
        // so that when it is unsubscribed the observer can be called
        // until the inner subscriptions have finished
        auto sink = make_subscriber<source_value_type>(
            state->out,
            outercs,
        // on_next
            [state](source_value_type st) {

                composite_subscription innercs;

                // when the out observer is unsubscribed all the
                // inner subscriptions are unsubscribed as well
                auto innercstoken = state->out.add(innercs);

                innercs.add(make_subscription([state, innercstoken](){
                    state->out.remove(innercstoken);
                }));

                auto selectedCollection = state->selectCollection(st);
                auto selectedSource = state->coordinator.in(selectedCollection);

                ++state->pendingCompletions;
                // this subscribe does not share the source subscription
                // so that when it is unsubscribed the source will continue
                auto sinkInner = make_subscriber<collection_value_type>(
                    state->out,
                    innercs,
                // on_next
                    [state, st](collection_value_type ct) {
                        auto selectedResult = state->selectResult(st, std::move(ct));
                        state->out.on_next(std::move(selectedResult));
                    },
                // on_error
                    [state](std::exception_ptr e) {
                        state->out.on_error(e);
                    },
                //on_completed
                    [state](){
                        if (--state->pendingCompletions == 0) {
                            state->out.on_completed();
                        }
                    }
                );

                auto selectedSinkInner = state->coordinator.out(sinkInner);
                selectedSource.subscribe(std::move(selectedSinkInner));
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
};

}

/*! @copydoc rx-flat_map.hpp
*/
template<class... AN>
auto flat_map(AN&&... an)
->     operator_factory<flat_map_tag, AN...> {
    return operator_factory<flat_map_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! @copydoc rx-flat_map.hpp
*/
template<class... AN>
auto merge_transform(AN&&... an)
->     operator_factory<flat_map_tag, AN...> {
    return operator_factory<flat_map_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<flat_map_tag>
{
    template<class Observable, class CollectionSelector,
        class CollectionSelectorType = rxu::decay_t<CollectionSelector>,
        class SourceValue = rxu::value_type_t<Observable>,
        class CollectionType = rxu::result_of_t<CollectionSelectorType(SourceValue)>,
        class ResultSelectorType = rxu::detail::take_at<1>,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, CollectionType>>,
        class FlatMap = rxo::detail::flat_map<rxu::decay_t<Observable>, rxu::decay_t<CollectionSelector>, ResultSelectorType, identity_one_worker>,
        class CollectionValueType = rxu::value_type_t<CollectionType>,
        class Value = rxu::result_of_t<ResultSelectorType(SourceValue, CollectionValueType)>,
        class Result = observable<Value, FlatMap>
    >
    static Result member(Observable&& o, CollectionSelector&& s) {
        return Result(FlatMap(std::forward<Observable>(o), std::forward<CollectionSelector>(s), ResultSelectorType(), identity_current_thread()));
    }

    template<class Observable, class CollectionSelector, class Coordination,
        class CollectionSelectorType = rxu::decay_t<CollectionSelector>,
        class SourceValue = rxu::value_type_t<Observable>,
        class CollectionType = rxu::result_of_t<CollectionSelectorType(SourceValue)>,
        class ResultSelectorType = rxu::detail::take_at<1>,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, CollectionType>,
            is_coordination<Coordination>>,
        class FlatMap = rxo::detail::flat_map<rxu::decay_t<Observable>, rxu::decay_t<CollectionSelector>, ResultSelectorType, rxu::decay_t<Coordination>>,
        class CollectionValueType = rxu::value_type_t<CollectionType>,
        class Value = rxu::result_of_t<ResultSelectorType(SourceValue, CollectionValueType)>,
        class Result = observable<Value, FlatMap>
    >
    static Result member(Observable&& o, CollectionSelector&& s, Coordination&& cn) {
        return Result(FlatMap(std::forward<Observable>(o), std::forward<CollectionSelector>(s), ResultSelectorType(), std::forward<Coordination>(cn)));
    }

    template<class Observable, class CollectionSelector, class ResultSelector,
        class IsCoordination = is_coordination<ResultSelector>,
        class CollectionSelectorType = rxu::decay_t<CollectionSelector>,
        class SourceValue = rxu::value_type_t<Observable>,
        class CollectionType = rxu::result_of_t<CollectionSelectorType(SourceValue)>,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, CollectionType>,
            rxu::negation<IsCoordination>>,
        class FlatMap = rxo::detail::flat_map<rxu::decay_t<Observable>, rxu::decay_t<CollectionSelector>, rxu::decay_t<ResultSelector>, identity_one_worker>,
        class CollectionValueType = rxu::value_type_t<CollectionType>,
        class ResultSelectorType = rxu::decay_t<ResultSelector>,
        class Value = rxu::result_of_t<ResultSelectorType(SourceValue, CollectionValueType)>,
        class Result = observable<Value, FlatMap>
    >
    static Result member(Observable&& o, CollectionSelector&& s, ResultSelector&& rs) {
        return Result(FlatMap(std::forward<Observable>(o), std::forward<CollectionSelector>(s), std::forward<ResultSelector>(rs), identity_current_thread()));
    }

    template<class Observable, class CollectionSelector, class ResultSelector, class Coordination,
        class CollectionSelectorType = rxu::decay_t<CollectionSelector>,
        class SourceValue = rxu::value_type_t<Observable>,
        class CollectionType = rxu::result_of_t<CollectionSelectorType(SourceValue)>,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, CollectionType>,
            is_coordination<Coordination>>,
        class FlatMap = rxo::detail::flat_map<rxu::decay_t<Observable>, rxu::decay_t<CollectionSelector>, rxu::decay_t<ResultSelector>, rxu::decay_t<Coordination>>,
        class CollectionValueType = rxu::value_type_t<CollectionType>,
        class ResultSelectorType = rxu::decay_t<ResultSelector>,
        class Value = rxu::result_of_t<ResultSelectorType(SourceValue, CollectionValueType)>,
        class Result = observable<Value, FlatMap>
    >
    static Result member(Observable&& o, CollectionSelector&& s, ResultSelector&& rs, Coordination&& cn) {
        return Result(FlatMap(std::forward<Observable>(o), std::forward<CollectionSelector>(s), std::forward<ResultSelector>(rs), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::flat_map_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "flat_map takes (CollectionSelector, optional ResultSelector, optional Coordination)");
    }
};

}

#endif
