// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-concat.hpp

    \brief For each item from this observable subscribe to one at a time, in the order received.
           For each item from all of the given observables deliver from the new observable that is returned.

           There are 2 variants of the operator:
           - The source observable emits nested observables, nested observables are concatenated.
           - The source observable and the arguments v0...vn are used to provide the observables to concatenate. 
 
    \tparam Coordination  the type of the scheduler (optional).
    \tparam Value0  ... (optional).
    \tparam ValueN  types of source observables (optional).
        
    \param  cn  the scheduler to synchronize sources from different contexts (optional).
    \param  v0  ... (optional).
    \param  vn  source observables (optional).
 
    \return  Observable that emits the items emitted by each of the Observables emitted by the source observable, one after the other, without interleaving them.

    \sample
    \snippet concat.cpp implicit concat sample
    \snippet output.txt implicit concat sample
    
    \sample
    \snippet concat.cpp threaded implicit concat sample
    \snippet output.txt threaded implicit concat sample
    
    \sample
    \snippet concat.cpp concat sample
    \snippet output.txt concat sample    
    
    \sample
    \snippet concat.cpp threaded concat sample
    \snippet output.txt threaded concat sample    
*/

#if !defined(RXCPP_OPERATORS_RX_CONCAT_HPP)
#define RXCPP_OPERATORS_RX_CONCAT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct concat_invalid_arguments {};

template<class... AN>
struct concat_invalid : public rxo::operator_base<concat_invalid_arguments<AN...>> {
    using type = observable<concat_invalid_arguments<AN...>, concat_invalid<AN...>>;
};
template<class... AN>
using concat_invalid_t = typename concat_invalid<AN...>::type;
    
template<class T, class Observable, class Coordination>
struct concat
    : public operator_base<rxu::value_type_t<rxu::decay_t<T>>>
{
    typedef concat<T, Observable, Coordination> this_type;

    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Coordination> coordination_type;

    typedef typename coordination_type::coordinator_type coordinator_type;

    typedef typename source_type::source_operator_type source_operator_type;
    typedef source_value_type collection_type;
    typedef typename collection_type::value_type value_type;

    struct values
    {
        values(source_operator_type o, coordination_type sf)
            : source_operator(std::move(o))
            , coordination(std::move(sf))
        {
        }
        source_operator_type source_operator;
        coordination_type coordination;
    };
    values initial;

    concat(const source_type& o, coordination_type sf)
        : initial(o.source_operator, std::move(sf))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber scbr) const {
        static_assert(is_subscriber<Subscriber>::value, "subscribe must be passed a subscriber");

        typedef Subscriber output_type;

        struct concat_state_type
            : public std::enable_shared_from_this<concat_state_type>
            , public values
        {
            concat_state_type(values i, coordinator_type coor, output_type oarg)
                : values(i)
                , source(i.source_operator)
                , sourceLifetime(composite_subscription::empty())
                , collectionLifetime(composite_subscription::empty())
                , coordinator(std::move(coor))
                , out(std::move(oarg))
            {
            }

            void subscribe_to(collection_type st)
            {
                auto state = this->shared_from_this();

                collectionLifetime = composite_subscription();

                // when the out observer is unsubscribed all the
                // inner subscriptions are unsubscribed as well
                auto innercstoken = state->out.add(collectionLifetime);

                collectionLifetime.add(make_subscription([state, innercstoken](){
                    state->out.remove(innercstoken);
                }));

                auto selectedSource = on_exception(
                    [&](){return state->coordinator.in(std::move(st));},
                    state->out);
                if (selectedSource.empty()) {
                    return;
                }

                // this subscribe does not share the out subscription
                // so that when it is unsubscribed the out will continue
                auto sinkInner = make_subscriber<value_type>(
                    state->out,
                    collectionLifetime,
                // on_next
                    [state, st](value_type ct) {
                        state->out.on_next(ct);
                    },
                // on_error
                    [state](std::exception_ptr e) {
                        state->out.on_error(e);
                    },
                //on_completed
                    [state](){
                        if (!state->selectedCollections.empty()) {
                            auto value = state->selectedCollections.front();
                            state->selectedCollections.pop_front();
                            state->collectionLifetime.unsubscribe();
                            state->subscribe_to(value);
                        } else if (!state->sourceLifetime.is_subscribed()) {
                            state->out.on_completed();
                        }
                    }
                );
                auto selectedSinkInner = on_exception(
                    [&](){return state->coordinator.out(sinkInner);},
                    state->out);
                if (selectedSinkInner.empty()) {
                    return;
                }
                selectedSource->subscribe(std::move(selectedSinkInner.get()));
            }
            observable<source_value_type, source_operator_type> source;
            composite_subscription sourceLifetime;
            composite_subscription collectionLifetime;
            std::deque<collection_type> selectedCollections;
            coordinator_type coordinator;
            output_type out;
        };

        auto coordinator = initial.coordination.create_coordinator(scbr.get_subscription());

        // take a copy of the values for each subscription
        auto state = std::make_shared<concat_state_type>(initial, std::move(coordinator), std::move(scbr));

        state->sourceLifetime = composite_subscription();

        // when the out observer is unsubscribed all the
        // inner subscriptions are unsubscribed as well
        state->out.add(state->sourceLifetime);

        auto source = on_exception(
            [&](){return state->coordinator.in(state->source);},
            state->out);
        if (source.empty()) {
            return;
        }

        // this subscribe does not share the observer subscription
        // so that when it is unsubscribed the observer can be called
        // until the inner subscriptions have finished
        auto sink = make_subscriber<collection_type>(
            state->out,
            state->sourceLifetime,
        // on_next
            [state](collection_type st) {
                if (state->collectionLifetime.is_subscribed()) {
                    state->selectedCollections.push_back(st);
                } else if (state->selectedCollections.empty()) {
                    state->subscribe_to(st);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                if (!state->collectionLifetime.is_subscribed() && state->selectedCollections.empty()) {
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

/*! @copydoc rx-concat.hpp
*/
template<class... AN>
auto concat(AN&&... an)
    ->     operator_factory<concat_tag, AN...> {
    return operator_factory<concat_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}    

}

template<>
struct member_overload<concat_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Concat = rxo::detail::concat<SourceValue, rxu::decay_t<Observable>, identity_one_worker>,
        class Value = rxu::value_type_t<SourceValue>,
        class Result = observable<Value, Concat>
    >
    static Result member(Observable&& o) {
        return Result(Concat(std::forward<Observable>(o), identity_current_thread()));
    }

    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Concat = rxo::detail::concat<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<SourceValue>,
        class Result = observable<Value, Concat>
    >
    static Result member(Observable&& o, Coordination&& cn) {
        return Result(Concat(std::forward<Observable>(o), std::forward<Coordination>(cn)));
    }

    template<class Observable, class Value0, class... ValueN,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, Value0, ValueN...>>,
        class EmittedValue = rxu::value_type_t<Observable>,
        class SourceValue = observable<EmittedValue>,
        class ObservableObservable = observable<SourceValue>,
        class Concat = typename rxu::defer_type<rxo::detail::concat, SourceValue, ObservableObservable, identity_one_worker>::type,
        class Value = rxu::value_type_t<Concat>,
        class Result = observable<Value, Concat>
    >
    static Result member(Observable&& o, Value0&& v0, ValueN&&... vn) {
        return Result(Concat(rxs::from(o.as_dynamic(), v0.as_dynamic(), vn.as_dynamic()...), identity_current_thread()));
    }

    template<class Observable, class Coordination, class Value0, class... ValueN,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, Value0, ValueN...>,
            is_coordination<Coordination>>,
        class EmittedValue = rxu::value_type_t<Observable>,
        class SourceValue = observable<EmittedValue>,
        class ObservableObservable = observable<SourceValue>,
        class Concat = typename rxu::defer_type<rxo::detail::concat, SourceValue, ObservableObservable, rxu::decay_t<Coordination>>::type,
        class Value = rxu::value_type_t<Concat>,
        class Result = observable<Value, Concat>
    >
    static Result member(Observable&& o, Coordination&& cn, Value0&& v0, ValueN&&... vn) {
        return Result(Concat(rxs::from(o.as_dynamic(), v0.as_dynamic(), vn.as_dynamic()...), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::concat_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "concat takes (optional Coordination, optional Value0, optional ValueN...)");
    }
};

}

#endif
