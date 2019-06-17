// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-start_with.hpp

    \brief Start with the supplied values, then concatenate this observable.

    \tparam Value0      ...
    \tparam ValueN      the type of sending values

    \param  v0  ...
    \param  vn  values to send

    \return  Observable that emits the specified items and then emits the items emitted by the source observable.

    \sample
    \snippet start_with.cpp short start_with sample
    \snippet output.txt short start_with sample

    Another form of this operator, rxcpp::observable<void, void>::start_with, gets the source observable as a parameter:
    \snippet start_with.cpp full start_with sample
    \snippet output.txt full start_with sample
*/

#if !defined(RXCPP_OPERATORS_RX_START_WITH_HPP)
#define RXCPP_OPERATORS_RX_START_WITH_HPP

#include "../rx-includes.hpp"
#include "./rx-concat.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct start_with_invalid_arguments {};

template<class... AN>
struct start_with_invalid : public rxo::operator_base<start_with_invalid_arguments<AN...>> {
    using type = observable<start_with_invalid_arguments<AN...>, start_with_invalid<AN...>>;
};
template<class... AN>
using start_with_invalid_t = typename start_with_invalid<AN...>::type;

}

/*! @copydoc rx-start_with.hpp
*/
template<class... AN>
auto start_with(AN&&... an)
    ->      operator_factory<start_with_tag, AN...> {
     return operator_factory<start_with_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<start_with_tag>
{
    template<class Observable, class Value0, class... ValueN,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class From = decltype(rxs::from(rxu::decay_t<Value0>(std::declval<Value0>()), rxu::decay_t<ValueN>(std::declval<ValueN>())...))
    >
    static auto member(Observable&& o, Value0&& v0, ValueN&&... vn)
     -> decltype(member_overload<concat_tag>::member(std::declval<From>(), std::forward<Observable>(o))) {
        auto first = rxs::from(rxu::decay_t<Value0>(v0), rxu::decay_t<ValueN>(vn)...);
        return member_overload<concat_tag>::member(first, std::forward<Observable>(o));
    }

    template<class... AN>
    static operators::detail::start_with_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "start_with takes (Value0, optional ValueN...)");
    }
};

}

#endif
