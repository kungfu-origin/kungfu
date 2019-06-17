// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-skip_while.hpp

    \brief Discards the first items fulfilling the predicate from this observable emit them from the new observable that is returned.

    \tparam Predicate  the type of the predicate

    \param t  the predicate

    \return		 An observable that discards the first items until condition emitted by the source Observable is not fulfilling the predicate, or all of the items from the source observable if the predicate never returns false

    \sample
    \snippet skip_while.cpp skip_while sample
    \snippet output.txt skip_while sample
*/

#if !defined(RXCPP_OPERATORS_RX_SKIP_WHILE_HPP)
#define RXCPP_OPERATORS_RX_SKIP_WHILE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct skip_while_invalid_arguments {};

template<class... AN>
struct skip_while_invalid : public rxo::operator_base<skip_while_invalid_arguments<AN...>> {
    using type = observable<skip_while_invalid_arguments<AN...>, skip_while_invalid<AN...>>;
};
template<class... AN>
using skip_while_invalid_t = typename skip_while_invalid<AN...>::type;

template<class T, class Predicate>
struct skip_while
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Predicate> test_type;
    test_type test;


    skip_while(test_type t)
        : test(std::move(t))
    {
    }

    template<class Subscriber>
    struct skip_while_observer
    {
        typedef skip_while_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        test_type test;
        bool pass;

        skip_while_observer(dest_type d, test_type t)
                : dest(std::move(d))
                , test(std::move(t)),
                  pass(false)
        {
        }
        void on_next(source_value_type v) {
            if(pass || !test(v))
            {
                pass = true;
                dest.on_next(v);
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
    -> decltype(skip_while_observer<Subscriber>::make(std::move(dest), test)) {
        return	skip_while_observer<Subscriber>::make(std::move(dest), test);
    }
};

}

/*! @copydoc rx-skip_while.hpp
*/
template<class... AN>
auto skip_while(AN&&... an)
    ->		operator_factory<skip_while_tag, AN...> {
        return operator_factory<skip_while_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
    }

}

template<>
struct member_overload<skip_while_tag>
{
    template<class Observable, class Predicate,
            class SourceValue = rxu::value_type_t<Observable>,
            class TakeWhile = rxo::detail::skip_while<SourceValue, rxu::decay_t<Predicate>>>
    static auto member(Observable&& o, Predicate&& p)
    -> decltype(o.template lift<SourceValue>(TakeWhile(std::forward<Predicate>(p)))) {
        return		o.template lift<SourceValue>(TakeWhile(std::forward<Predicate>(p)));
    }

    template<class... AN>
    static operators::detail::skip_while_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "skip_while takes (Predicate)");
    }
};


}

#endif
