// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-any.hpp

    \brief Returns an Observable that emits true if any item emitted by the source Observable satisfies a specified condition, otherwise false. Emits false if the source Observable terminates without emitting any item.

    \tparam Predicate the type of the test function.

    \param p the test function to test items emitted by the source Observable.

    \return  An observable that emits true if any item emitted by the source observable satisfies a specified condition, otherwise false.

    Some basic any- operators have already been implemented:
    - rxcpp::operators::exists
    - rxcpp::operators::contains

    \sample
    \snippet exists.cpp exists sample
    \snippet output.txt exists sample

    \sample
    \snippet contains.cpp contains sample
    \snippet output.txt contains sample
*/


#if !defined(RXCPP_OPERATORS_RX_ANY_HPP)
#define RXCPP_OPERATORS_RX_ANY_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct any_invalid_arguments {};

template<class... AN>
struct any_invalid : public rxo::operator_base<any_invalid_arguments<AN...>> {
    using type = observable<any_invalid_arguments<AN...>, any_invalid<AN...>>;
};
template<class... AN>
using any_invalid_t = typename any_invalid<AN...>::type;

template<class T, class Predicate>
struct any
{
    typedef rxu::decay_t<T> source_value_type;
    typedef bool value_type;
    typedef rxu::decay_t<Predicate> test_type;
    test_type test;

    any(test_type t)
        : test(std::move(t))
    {
    }

    template<class Subscriber>
    struct any_observer
    {
        typedef any_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        test_type test;
        mutable bool done;

        any_observer(dest_type d, test_type t)
            : dest(std::move(d))
            , test(std::move(t)),
              done(false)
        {
        }
        void on_next(source_value_type v) const {
            auto filtered = on_exception([&]() {
                return !this->test(v); },
                dest);
            if (filtered.empty()) {
                return;
            }
            if (!filtered.get() && !done) {
                done = true;
                dest.on_next(true);
                dest.on_completed();
            }
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            if(!done) {
                done = true;
                dest.on_next(false);
                dest.on_completed();
            }
        }

        static subscriber<value_type, observer_type> make(dest_type d, test_type t) {
            return make_subscriber<value_type>(d, this_type(d, std::move(t)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(any_observer<Subscriber>::make(std::move(dest), test)) {
        return      any_observer<Subscriber>::make(std::move(dest), test);
    }
};

}

/*! @copydoc rx-any.hpp
*/
template<class... AN>
auto any(AN&&... an)
    ->     operator_factory<any_tag, AN...> {
    return operator_factory<any_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! \brief Returns an Observable that emits true if any item emitted by the source Observable satisfies a specified condition, otherwise false. Emits false if the source Observable terminates without emitting any item.

    \tparam Predicate the type of the test function.

    \param p the test function to test items emitted by the source Observable.

    \return  An observable that emits true if any item emitted by the source observable satisfies a specified condition, otherwise false.

    \sample
    \snippet exists.cpp exists sample
    \snippet output.txt exists sample
*/
template<class... AN>
auto exists(AN&&... an)
    ->     operator_factory<exists_tag, AN...> {
    return operator_factory<exists_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! \brief Returns an Observable that emits true if the source Observable emitted a specified item, otherwise false. Emits false if the source Observable terminates without emitting any item.

    \tparam T the type of the item to search for.

    \param value the item to search for.

    \return An observable that emits true if the source Observable emitted a specified item, otherwise false.

    \sample
    \snippet contains.cpp contains sample
    \snippet output.txt contains sample
*/
template<class... AN>
auto contains(AN&&... an)
->     operator_factory<contains_tag, AN...> {
    return operator_factory<contains_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<any_tag>
{
    template<class Observable, class Predicate,
        class SourceValue = rxu::value_type_t<Observable>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class Any = rxo::detail::any<SourceValue, rxu::decay_t<Predicate>>,
        class Value = rxu::value_type_t<Any>>
    static auto member(Observable&& o, Predicate&& p)
    -> decltype(o.template lift<Value>(Any(std::forward<Predicate>(p)))) {
        return  o.template lift<Value>(Any(std::forward<Predicate>(p)));
    }

    template<class... AN>
    static operators::detail::any_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "any takes (Predicate)");
    }
};

template<>
struct member_overload<exists_tag>
    : member_overload<any_tag>
{
    using member_overload<any_tag>::member;

    template<class... AN>
    static operators::detail::any_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "exists takes (Predicate)");
    }
};

template<>
struct member_overload<contains_tag>
{
    template<class Observable, class T,
        class SourceValue = rxu::value_type_t<Observable>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class Predicate = std::function<bool(T)>,
        class Any = rxo::detail::any<SourceValue, rxu::decay_t<Predicate>>,
        class Value = rxu::value_type_t<Any>>
    static auto member(Observable&& o, T&& value)
    -> decltype(o.template lift<Value>(Any(nullptr))) {
        return  o.template lift<Value>(Any([value](T n) { return n == value; }));
    }

    template<class... AN>
    static operators::detail::any_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "contains takes (T)");
    }
};

}

#endif
