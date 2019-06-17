// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-filter.hpp

    \brief For each item from this observable use Predicate to select which items to emit from the new observable that is returned.

    \tparam Predicate  the type of the filter function

    \param p  the filter function

    \return  Observable that emits only those items emitted by the source observable that the filter evaluates as true.

    \sample
    \snippet filter.cpp filter sample
    \snippet output.txt filter sample
*/

#if !defined(RXCPP_OPERATORS_RX_FILTER_HPP)
#define RXCPP_OPERATORS_RX_FILTER_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct filter_invalid_arguments {};

template<class... AN>
struct filter_invalid : public rxo::operator_base<filter_invalid_arguments<AN...>> {
    using type = observable<filter_invalid_arguments<AN...>, filter_invalid<AN...>>;
};
template<class... AN>
using filter_invalid_t = typename filter_invalid<AN...>::type;

template<class T, class Predicate>
struct filter
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Predicate> test_type;
    test_type test;

    filter(test_type t)
        : test(std::move(t))
    {
    }

    template<class Subscriber>
    struct filter_observer
    {
        typedef filter_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        mutable test_type test;

        filter_observer(dest_type d, test_type t)
            : dest(std::move(d))
            , test(std::move(t))
        {
        }

        template <class Value>
        void on_next(Value&& v) const {
            auto filtered = on_exception([&](){
                    return !this->test(rxu::as_const(v));
                },
                dest);
            if (filtered.empty()) {
                return;
            }
            if (!filtered.get()) {
                dest.on_next(std::forward<Value>(v));
            }
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer_type> make(dest_type d, test_type t) {
            return make_subscriber<value_type>(d, this_type(d, std::move(t)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(filter_observer<Subscriber>::make(std::move(dest), test)) {
        return      filter_observer<Subscriber>::make(std::move(dest), test);
    }
};

}

/*! @copydoc rx-filter.hpp
*/
template<class... AN>
auto filter(AN&&... an)
    ->      operator_factory<filter_tag, AN...> {
     return operator_factory<filter_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<filter_tag>
{
    template<class Observable, class Predicate,
        class SourceValue = rxu::value_type_t<Observable>,
        class Filter = rxo::detail::filter<SourceValue, rxu::decay_t<Predicate>>>
    static auto member(Observable&& o, Predicate&& p)
        -> decltype(o.template lift<SourceValue>(Filter(std::forward<Predicate>(p)))) {
        return      o.template lift<SourceValue>(Filter(std::forward<Predicate>(p)));
    }

    template<class... AN>
    static operators::detail::filter_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "filter takes (Predicate)");
    }
};

}

#endif
