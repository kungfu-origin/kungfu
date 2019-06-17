// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-ref_count.hpp

    \brief  takes a connectable_observable source and uses a ref_count of the subscribers to control the connection to the published source.
            The first subscription will cause a call to connect() and the last unsubscribe will unsubscribe the connection.

    \return An observable that emitting the items from its source.
 */

#if !defined(RXCPP_OPERATORS_RX_REF_COUNT_HPP)
#define RXCPP_OPERATORS_RX_REF_COUNT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct ref_count_invalid_arguments {};

template<class... AN>
struct ref_count_invalid : public rxo::operator_base<ref_count_invalid_arguments<AN...>> {
    using type = observable<ref_count_invalid_arguments<AN...>, ref_count_invalid<AN...>>;
};
template<class... AN>
using ref_count_invalid_t = typename ref_count_invalid<AN...>::type;
    
template<class T, class ConnectableObservable>
struct ref_count : public operator_base<T>
{
    typedef rxu::decay_t<ConnectableObservable> source_type;

    struct ref_count_state : public std::enable_shared_from_this<ref_count_state>
    {
        explicit ref_count_state(source_type o)
            : source(std::move(o))
            , subscribers(0)
        {
        }

        source_type source;
        std::mutex lock;
        long subscribers;
        composite_subscription connection;
    };
    std::shared_ptr<ref_count_state> state;

    explicit ref_count(source_type o)
        : state(std::make_shared<ref_count_state>(std::move(o)))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber&& o) const {
        std::unique_lock<std::mutex> guard(state->lock);
        auto needConnect = ++state->subscribers == 1;
        auto keepAlive = state;
        guard.unlock();
        o.add(
            [keepAlive](){
                std::unique_lock<std::mutex> guard_unsubscribe(keepAlive->lock);
                if (--keepAlive->subscribers == 0) {
                    keepAlive->connection.unsubscribe();
                    keepAlive->connection = composite_subscription();
                }
            });
        keepAlive->source.subscribe(std::forward<Subscriber>(o));
        if (needConnect) {
            keepAlive->source.connect(keepAlive->connection);
        }
    }
};

}

/*! @copydoc rx-ref_count.hpp
*/
template<class... AN>
auto ref_count(AN&&... an)
    ->     operator_factory<ref_count_tag, AN...> {
    return operator_factory<ref_count_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}   
    
}

template<>
struct member_overload<ref_count_tag>
{
    template<class ConnectableObservable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_connectable_observable<ConnectableObservable>>,
        class SourceValue = rxu::value_type_t<ConnectableObservable>,
        class RefCount = rxo::detail::ref_count<SourceValue, rxu::decay_t<ConnectableObservable>>,
        class Value = rxu::value_type_t<RefCount>,
        class Result = observable<Value, RefCount>
        >
    static Result member(ConnectableObservable&& o) {
        return Result(RefCount(std::forward<ConnectableObservable>(o)));
    }

    template<class... AN>
    static operators::detail::ref_count_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "ref_count takes no arguments");
    }
};
    
}

#endif
