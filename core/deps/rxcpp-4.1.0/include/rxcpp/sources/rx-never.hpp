// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_SOURCES_RX_NEVER_HPP)
#define RXCPP_SOURCES_RX_NEVER_HPP

#include "../rx-includes.hpp"

/*! \file rx-never.hpp

    \brief Returns an observable that never sends any items or notifications to observer.

    \tparam T  the type of (not) emitted items

    \return  Observable that never sends any items or notifications to observer.

    \sample
    \snippet never.cpp never sample
    \snippet output.txt never sample
*/

namespace rxcpp {

namespace sources {

namespace detail {

template<class T>
struct never : public source_base<T>
{
    template<class Subscriber>
    void on_subscribe(Subscriber) const {
    }
};

}

/*! @copydoc rx-never.hpp
 */
template<class T>
auto never()
    ->      observable<T, detail::never<T>> {
    return  observable<T, detail::never<T>>(detail::never<T>());
}

}

}

#endif
