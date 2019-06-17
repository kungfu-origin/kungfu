// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_SOURCES_RX_EMPTY_HPP)
#define RXCPP_SOURCES_RX_EMPTY_HPP

#include "../rx-includes.hpp"

/*! \file rx-empty.hpp

    \brief Returns an observable that sends no items to observer and immediately completes, on the specified scheduler.

    \tparam T             the type of (not) emitted items
    \tparam Coordination  the type of the scheduler (optional)

    \param  cn  the scheduler to use for scheduling the items (optional)

    \return  Observable that sends no items to observer and immediately completes.

    \sample
    \snippet empty.cpp empty sample
    \snippet output.txt empty sample

    \sample
    \snippet empty.cpp threaded empty sample
    \snippet output.txt threaded empty sample
*/

namespace rxcpp {

namespace sources {

/*! @copydoc rx-empty.hpp
 */
template<class T>
auto empty()
    -> decltype(from<T>()) {
    return      from<T>();
}
/*! @copydoc rx-empty.hpp
 */
template<class T, class Coordination>
auto empty(Coordination cn)
    -> decltype(from<T>(std::move(cn))) {
    return      from<T>(std::move(cn));
}

}

}

#endif
