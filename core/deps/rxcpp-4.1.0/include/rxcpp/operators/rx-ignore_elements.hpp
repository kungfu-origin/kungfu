// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-ignore_elements.hpp

    \brief Do not emit any items from the source Observable, but allow termination notification (either onError or onCompleted) to pass through unchanged.

    \return Observable that emits termination notification from the source observable.

    \sample
    \snippet ignore_elements.cpp ignore_elements sample
    \snippet output.txt ignore_elements sample
*/

#if !defined(RXCPP_OPERATORS_RX_IGNORE_ELEMENTS_HPP)
#define RXCPP_OPERATORS_RX_IGNORE_ELEMENTS_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct ignore_elements_invalid_arguments {};

template<class... AN>
struct ignore_elements_invalid : public rxo::operator_base<ignore_elements_invalid_arguments<AN...>> {
    using type = observable<ignore_elements_invalid_arguments<AN...>, ignore_elements_invalid<AN...>>;
};
template<class... AN>
using ignore_elements_invalid_t = typename ignore_elements_invalid<AN...>::type;

template<class T>
struct ignore_elements {
    typedef rxu::decay_t<T> source_value_type;

    template<class Subscriber>
    struct ignore_elements_observer
    {
        typedef ignore_elements_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;

        ignore_elements_observer(dest_type d)
            : dest(d)
        {
        }

        void on_next(source_value_type) const {
            // no-op; ignore element
        }

        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }

        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer_type> make(dest_type d) {
            return make_subscriber<value_type>(d, this_type(d));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(ignore_elements_observer<Subscriber>::make(std::move(dest))) {
        return      ignore_elements_observer<Subscriber>::make(std::move(dest));
    }
};

}

/*! @copydoc rx-ignore_elements.hpp
*/
template<class... AN>
auto ignore_elements(AN&&... an)
->     operator_factory<ignore_elements_tag, AN...> {
    return operator_factory<ignore_elements_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<ignore_elements_tag>
{
    template<class Observable,
            class SourceValue = rxu::value_type_t<Observable>,
            class Enabled = rxu::enable_if_all_true_type_t<
                is_observable<Observable>>,
            class IgnoreElements = rxo::detail::ignore_elements<SourceValue>>
    static auto member(Observable&& o)
    -> decltype(o.template lift<SourceValue>(IgnoreElements())) {
        return  o.template lift<SourceValue>(IgnoreElements());
    }

    template<class... AN>
    static operators::detail::ignore_elements_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "ignore_elements takes no arguments");
    }
};

}

#endif
