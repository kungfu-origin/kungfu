// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-on_error_resume_next.hpp

    \brief If an error occurs, take the result from the Selector and subscribe to that instead.

    \tparam Selector the actual type of a function of the form `observable<T>(std::exception_ptr)`

    \param s the function of the form `observable<T>(std::exception_ptr)`

    \return Observable that emits the items from the source observable and switches to a new observable on error.

    \sample
    \snippet on_error_resume_next.cpp on_error_resume_next sample
    \snippet output.txt on_error_resume_next sample
*/

#if !defined(RXCPP_OPERATORS_RX_ON_ERROR_RESUME_NEXT_HPP)
#define RXCPP_OPERATORS_RX_ON_ERROR_RESUME_NEXT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct on_error_resume_next_invalid_arguments {};

template<class... AN>
struct on_error_resume_next_invalid : public rxo::operator_base<on_error_resume_next_invalid_arguments<AN...>> {
    using type = observable<on_error_resume_next_invalid_arguments<AN...>, on_error_resume_next_invalid<AN...>>;
};
template<class... AN>
using on_error_resume_next_invalid_t = typename on_error_resume_next_invalid<AN...>::type;


template<class T, class Selector>
struct on_error_resume_next
{
    typedef rxu::decay_t<T> value_type;
    typedef rxu::decay_t<Selector> select_type;
    typedef decltype((*(select_type*)nullptr)(std::exception_ptr())) fallback_type;
    select_type selector;

    on_error_resume_next(select_type s)
        : selector(std::move(s))
    {
    }

    template<class Subscriber>
    struct on_error_resume_next_observer
    {
        typedef on_error_resume_next_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Selector> select_type;
        typedef decltype((*(select_type*)nullptr)(std::exception_ptr())) fallback_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;
        dest_type dest;
        composite_subscription lifetime;
        select_type selector;

        on_error_resume_next_observer(dest_type d, composite_subscription cs, select_type s)
            : dest(std::move(d))
            , lifetime(std::move(cs))
            , selector(std::move(s))
        {
            dest.add(lifetime);
        }
        void on_next(value_type v) const {
            dest.on_next(std::move(v));
        }
        void on_error(std::exception_ptr e) const {
            auto selected = on_exception(
                [&](){
                    return this->selector(std::move(e));},
                dest);
            if (selected.empty()) {
                return;
            }
            selected->subscribe(dest);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<T, observer_type> make(dest_type d, select_type s) {
            auto cs = composite_subscription();
            return make_subscriber<T>(cs, observer_type(this_type(std::move(d), cs, std::move(s))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(on_error_resume_next_observer<Subscriber>::make(std::move(dest), selector)) {
        return      on_error_resume_next_observer<Subscriber>::make(std::move(dest), selector);
    }
};

}

/*! @copydoc rx-on_error_resume_next.hpp
*/
template<class... AN>
auto on_error_resume_next(AN&&... an)
    ->     operator_factory<on_error_resume_next_tag, AN...> {
    return operator_factory<on_error_resume_next_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! @copydoc rx-on_error_resume_next.hpp
*/
template<class... AN>
auto switch_on_error(AN&&... an)
    ->     operator_factory<on_error_resume_next_tag, AN...> {
    return operator_factory<on_error_resume_next_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<on_error_resume_next_tag>
{
    template<class Observable, class Selector,
            class SourceValue = rxu::value_type_t<Observable>,
            class OnErrorResumeNext = rxo::detail::on_error_resume_next<SourceValue, rxu::decay_t<Selector>>,
            class Value = rxu::value_type_t<OnErrorResumeNext>>
    static auto member(Observable&& o, Selector&& p)
    -> decltype(o.template lift<Value>(OnErrorResumeNext(std::forward<Selector>(p)))) {
        return      o.template lift<Value>(OnErrorResumeNext(std::forward<Selector>(p)));
    }

    template<class... AN>
    static operators::detail::on_error_resume_next_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "on_error_resume_next takes (Selector)");
    }
};

}

#endif
