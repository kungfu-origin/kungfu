// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-connect_forever.hpp

    \brief  takes a connectable_observable source and calls connect during the construction of the expression.
            This means that the source starts running without any subscribers and continues running after all subscriptions have been unsubscribed.

    \return An observable that emitting the items from its source.
 */

#if !defined(RXCPP_OPERATORS_RX_CONNECT_FOREVER_HPP)
#define RXCPP_OPERATORS_RX_CONNECT_FOREVER_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct connect_forever_invalid_arguments {};

template<class... AN>
struct connect_forever_invalid : public rxo::operator_base<connect_forever_invalid_arguments<AN...>> {
    using type = observable<connect_forever_invalid_arguments<AN...>, connect_forever_invalid<AN...>>;
};
template<class... AN>
using connect_forever_invalid_t = typename connect_forever_invalid<AN...>::type;

template<class T, class ConnectableObservable>
struct connect_forever : public operator_base<T>
{
    typedef rxu::decay_t<ConnectableObservable> source_type;

    source_type source;

    explicit connect_forever(source_type o)
        : source(std::move(o))
    {
        source.connect();
    }

    template<class Subscriber>
    void on_subscribe(Subscriber&& o) const {
        source.subscribe(std::forward<Subscriber>(o));
    }
};

}

/*! @copydoc rx-connect_forever.hpp
*/
template<class... AN>
auto connect_forever(AN&&... an)
->     operator_factory<connect_forever_tag, AN...> {
    return operator_factory<connect_forever_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<connect_forever_tag>
{
    template<class ConnectableObservable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_connectable_observable<ConnectableObservable>>,
        class SourceValue = rxu::value_type_t<ConnectableObservable>,
        class ConnectForever = rxo::detail::connect_forever<SourceValue, rxu::decay_t<ConnectableObservable>>,
        class Value = rxu::value_type_t<ConnectForever>,
        class Result = observable<Value, ConnectForever>
        >
    static Result member(ConnectableObservable&& o) {
        return Result(ConnectForever(std::forward<ConnectableObservable>(o)));
    }

    template<class... AN>
    static operators::detail::connect_forever_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "connect_forever takes no arguments");
    }
};

}

#endif
