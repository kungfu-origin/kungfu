// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-distinct.hpp

    \brief For each item from this observable, filter out repeated values and emit only items that have not already been emitted.

    \return Observable that emits those items from the source observable that are distinct.

    \note istinct keeps an unordered_set<T> of past values. Due to an issue in multiple implementations of std::hash<T>, rxcpp maintains a whitelist of hashable types. new types can be added by specializing rxcpp::filtered_hash<T>

    \sample
    \snippet distinct.cpp distinct sample
    \snippet output.txt distinct sample
*/

#if !defined(RXCPP_OPERATORS_RX_DISTINCT_HPP)
#define RXCPP_OPERATORS_RX_DISTINCT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct distinct_invalid_arguments {};

template<class... AN>
struct distinct_invalid : public rxo::operator_base<distinct_invalid_arguments<AN...>> {
    using type = observable<distinct_invalid_arguments<AN...>, distinct_invalid<AN...>>;
};
template<class... AN>
using distinct_invalid_t = typename distinct_invalid<AN...>::type;

template<class T>
struct distinct
{
    typedef rxu::decay_t<T> source_value_type;

    template<class Subscriber>
    struct distinct_observer
    {
        typedef distinct_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        mutable std::unordered_set<source_value_type, rxcpp::filtered_hash<source_value_type>> remembered;

        distinct_observer(dest_type d)
                : dest(d)
        {
        }
        void on_next(source_value_type v) const {
            if (remembered.empty() || remembered.count(v) == 0) {
                remembered.insert(v);
                dest.on_next(v);
            }
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer<value_type, this_type>> make(dest_type d) {
            return make_subscriber<value_type>(d, this_type(d));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
    -> decltype(distinct_observer<Subscriber>::make(std::move(dest))) {
        return      distinct_observer<Subscriber>::make(std::move(dest));
    }
};

}

/*! @copydoc rx-distinct.hpp
*/
template<class... AN>
auto distinct(AN&&... an)
    ->     operator_factory<distinct_tag, AN...> {
    return operator_factory<distinct_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<distinct_tag>
{
    template<class Observable,
            class SourceValue = rxu::value_type_t<Observable>,
            class Enabled = rxu::enable_if_all_true_type_t<
                is_observable<Observable>,
                is_hashable<SourceValue>>,
            class Distinct = rxo::detail::distinct<SourceValue>>
    static auto member(Observable&& o)
    -> decltype(o.template lift<SourceValue>(Distinct())) {
        return  o.template lift<SourceValue>(Distinct());
    }

    template<class... AN>
    static operators::detail::distinct_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "distinct takes no arguments");
    }
};

}

#endif
