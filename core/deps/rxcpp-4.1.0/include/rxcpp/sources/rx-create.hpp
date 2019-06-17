// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_SOURCES_RX_CREATE_HPP)
#define RXCPP_SOURCES_RX_CREATE_HPP

#include "../rx-includes.hpp"

/*! \file rx-create.hpp

    \brief Returns an observable that executes the specified function when a subscriber subscribes to it.

    \tparam T  the type of the items that this observable emits
    \tparam OnSubscribe  the type of OnSubscribe handler function

    \param  os  OnSubscribe event handler

    \return  Observable that executes the specified function when a Subscriber subscribes to it.

    \sample
    \snippet create.cpp Create sample
    \snippet output.txt Create sample

    \warning
    It is good practice to check the observer's is_subscribed state from within the function you pass to create
    so that your observable can stop emitting items or doing expensive calculations when there is no longer an interested observer.

    \badcode
    \snippet create.cpp Create bad code
    \snippet output.txt Create bad code

    \goodcode
    \snippet create.cpp Create good code
    \snippet output.txt Create good code

    \warning
    It is good practice to use operators like observable::take to control lifetime rather than use the subscription explicitly.

    \goodcode
    \snippet create.cpp Create great code
    \snippet output.txt Create great code
*/

namespace rxcpp {

namespace sources {

namespace detail {

template<class T, class OnSubscribe>
struct create : public source_base<T>
{
    typedef create<T, OnSubscribe> this_type;

    typedef rxu::decay_t<OnSubscribe> on_subscribe_type;

    on_subscribe_type on_subscribe_function;

    create(on_subscribe_type os)
        : on_subscribe_function(std::move(os))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber o) const {

        on_exception(
            [&](){
                this->on_subscribe_function(o);
                return true;
            },
            o);
    }
};

}

/*! @copydoc rx-create.hpp
    */
template<class T, class OnSubscribe>
auto create(OnSubscribe os)
    ->      observable<T,   detail::create<T, OnSubscribe>> {
    return  observable<T,   detail::create<T, OnSubscribe>>(
                            detail::create<T, OnSubscribe>(std::move(os)));
}

}

}

#endif
