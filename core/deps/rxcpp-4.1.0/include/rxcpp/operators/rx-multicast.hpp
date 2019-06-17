// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-multicast.hpp

    \brief allows connections to the source to be independent of subscriptions.

    \tparam Subject the subject to multicast the source Observable.

    \param sub  the subject.
*/

#if !defined(RXCPP_OPERATORS_RX_MULTICAST_HPP)
#define RXCPP_OPERATORS_RX_MULTICAST_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct multicast_invalid_arguments {};

template<class... AN>
struct multicast_invalid : public rxo::operator_base<multicast_invalid_arguments<AN...>> {
    using type = observable<multicast_invalid_arguments<AN...>, multicast_invalid<AN...>>;
};
template<class... AN>
using multicast_invalid_t = typename multicast_invalid<AN...>::type;

template<class T, class Observable, class Subject>
struct multicast : public operator_base<T>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Subject> subject_type;

    struct multicast_state : public std::enable_shared_from_this<multicast_state>
    {
        multicast_state(source_type o, subject_type sub)
            : source(std::move(o))
            , subject_value(std::move(sub))
        {
        }
        source_type source;
        subject_type subject_value;
        rxu::detail::maybe<typename composite_subscription::weak_subscription> connection;
    };

    std::shared_ptr<multicast_state> state;

    multicast(source_type o, subject_type sub)
        : state(std::make_shared<multicast_state>(std::move(o), std::move(sub)))
    {
    }
    template<class Subscriber>
    void on_subscribe(Subscriber&& o) const {
        state->subject_value.get_observable().subscribe(std::forward<Subscriber>(o));
    }
    void on_connect(composite_subscription cs) const {
        if (state->connection.empty()) {
            auto destination = state->subject_value.get_subscriber();

            // the lifetime of each connect is nested in the subject lifetime
            state->connection.reset(destination.add(cs));

            auto localState = state;

            // when the connection is finished it should shutdown the connection
            cs.add(
                [destination, localState](){
                    if (!localState->connection.empty()) {
                        destination.remove(localState->connection.get());
                        localState->connection.reset();
                    }
                });

            // use cs not destination for lifetime of subscribe.
            state->source.subscribe(cs, destination);
        }
    }
};

}

/*! @copydoc rx-multicast.hpp
*/
template<class... AN>
auto multicast(AN&&... an)
    ->      operator_factory<multicast_tag, AN...> {
     return operator_factory<multicast_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<multicast_tag>
{
    template<class Observable, class Subject,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Multicast = rxo::detail::multicast<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Subject>>,
        class Value = rxu::value_type_t<Multicast>,
        class Result = connectable_observable<Value, Multicast>>
    static Result member(Observable&& o, Subject&& sub) {
        return Result(Multicast(std::forward<Observable>(o), std::forward<Subject>(sub)));
    }

    template<class... AN>
    static operators::detail::multicast_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "multicast takes (Subject)");
    }
};

}

#endif
