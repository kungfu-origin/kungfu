// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-subscribe.hpp

    \brief Subscribe will cause the source observable to emit values to the provided subscriber.

    \tparam ArgN  types of the subscriber parameters

    \param an  the parameters for making a subscriber

    \return  A subscription with which the observer can stop receiving items before the observable has finished sending them.

    The arguments of subscribe are forwarded to rxcpp::make_subscriber function. Some possible alternatives are:

    - Pass an already composed rxcpp::subscriber:
    \snippet subscribe.cpp subscribe by subscriber
    \snippet output.txt subscribe by subscriber

    - Pass an rxcpp::observer. This allows subscribing the same subscriber to several observables:
    \snippet subscribe.cpp subscribe by observer
    \snippet output.txt subscribe by observer

    - Pass an `on_next` handler:
    \snippet subscribe.cpp subscribe by on_next
    \snippet output.txt subscribe by on_next

    - Pass `on_next` and `on_error` handlers:
    \snippet subscribe.cpp subscribe by on_next and on_error
    \snippet output.txt subscribe by on_next and on_error

    - Pass `on_next` and `on_completed` handlers:
    \snippet subscribe.cpp subscribe by on_next and on_completed
    \snippet output.txt subscribe by on_next and on_completed

    - Pass `on_next`, `on_error`, and `on_completed` handlers:
    \snippet subscribe.cpp subscribe by on_next, on_error, and on_completed
    \snippet output.txt subscribe by on_next, on_error, and on_completed
    .

    All the alternatives above also support passing rxcpp::composite_subscription instance. For example:
    \snippet subscribe.cpp subscribe by subscription, on_next, and on_completed
    \snippet output.txt subscribe by subscription, on_next, and on_completed

    If neither subscription nor subscriber are provided, then a new subscription is created and returned as a result:
    \snippet subscribe.cpp subscribe unsubscribe
    \snippet output.txt subscribe unsubscribe

    For more details, see rxcpp::make_subscriber function description.
*/

#if !defined(RXCPP_OPERATORS_RX_SUBSCRIBE_HPP)
#define RXCPP_OPERATORS_RX_SUBSCRIBE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class Subscriber>
class subscribe_factory;

template<class T, class I>
class subscribe_factory<subscriber<T, I>>
{
    subscriber<T, I> scrbr;
public:
    subscribe_factory(subscriber<T, I> s)
        : scrbr(std::move(s))
    {}
    template<class Observable>
    auto operator()(Observable&& source)
        -> decltype(std::forward<Observable>(source).subscribe(std::move(scrbr))) {
        return      std::forward<Observable>(source).subscribe(std::move(scrbr));
    }
};

}

/*! @copydoc rx-subscribe.hpp
*/
template<class T, class... ArgN>
auto subscribe(ArgN&&... an)
    ->      detail::subscribe_factory<decltype  (make_subscriber<T>(std::forward<ArgN>(an)...))> {
    return  detail::subscribe_factory<decltype  (make_subscriber<T>(std::forward<ArgN>(an)...))>
                                                (make_subscriber<T>(std::forward<ArgN>(an)...));
}

namespace detail {

class dynamic_factory
{
public:
    template<class Observable>
    auto operator()(Observable&& source)
        ->      observable<rxu::value_type_t<rxu::decay_t<Observable>>> {
        return  observable<rxu::value_type_t<rxu::decay_t<Observable>>>(std::forward<Observable>(source));
    }
};

}

/*! Return a new observable that performs type-forgetting conversion of this observable.

    \return  The source observable converted to observable<T>.

    \note This operator could be useful to workaround lambda deduction bug on msvc 2013.

    \sample
    \snippet as_dynamic.cpp as_dynamic sample
    \snippet output.txt as_dynamic sample
*/
inline auto as_dynamic()
    ->      detail::dynamic_factory {
    return  detail::dynamic_factory();
}

namespace detail {

class blocking_factory
{
public:
    template<class Observable>
    auto operator()(Observable&& source)
        -> decltype(std::forward<Observable>(source).as_blocking()) {
        return      std::forward<Observable>(source).as_blocking();
    }
};

}

/*! Return a new observable that contains the blocking methods for this observable.

    \return  An observable that contains the blocking methods for this observable.

    \sample
    \snippet from.cpp threaded from sample
    \snippet output.txt threaded from sample
*/
inline auto as_blocking()
    ->      detail::blocking_factory {
    return  detail::blocking_factory();
}


}

}

#endif
