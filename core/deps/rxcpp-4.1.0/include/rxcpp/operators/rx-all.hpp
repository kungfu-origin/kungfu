// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-all.hpp

    \brief Returns an Observable that emits true if every item emitted by the source Observable satisfies a specified condition, otherwise false.
           Emits true if the source Observable terminates without emitting any item.

    \tparam Predicate  the type of the test function.

    \param p  the test function to test items emitted by the source Observable.

    \return  Observable that emits true if every item emitted by the source observable satisfies a specified condition, otherwise false.

    \sample
    \snippet all.cpp all sample
    \snippet output.txt all sample
*/

#if !defined(RXCPP_OPERATORS_RX_ALL_HPP)
#define RXCPP_OPERATORS_RX_ALL_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct all_invalid_arguments {};

template<class... AN>
struct all_invalid : public rxo::operator_base<all_invalid_arguments<AN...>> {
    using type = observable<all_invalid_arguments<AN...>, all_invalid<AN...>>;
};
template<class... AN>
using all_invalid_t = typename all_invalid<AN...>::type;

template<class T, class Predicate>
struct all
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Predicate> test_type;
    test_type test;

    typedef bool value_type;

    all(test_type t)
        : test(std::move(t))
    {
    }

    template<class Subscriber>
    struct all_observer
    {
        typedef all_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        test_type test;
        mutable bool done;

        all_observer(dest_type d, test_type t)
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
            if (filtered.get() && !done) {
                done = true;
                dest.on_next(false);
                dest.on_completed();
            }
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            if(!done) {
                done = true;
                dest.on_next(true);
                dest.on_completed();
            }
        }

        static subscriber<value_type, observer_type> make(dest_type d, test_type t) {
            return make_subscriber<value_type>(d, this_type(d, std::move(t)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(all_observer<Subscriber>::make(std::move(dest), test)) {
        return      all_observer<Subscriber>::make(std::move(dest), test);
    }
};

}

/*! @copydoc rx-all.hpp
*/
template<class... AN>
auto all(AN&&... an)
    ->     operator_factory<all_tag, AN...> {
    return operator_factory<all_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! \brief Returns an Observable that emits true if the source Observable is empty, otherwise false.

    \return An observable that emits a boolean value.

    \sample
    \snippet is_empty.cpp is_empty sample
    \snippet output.txt is_empty sample
*/
template<class... AN>
auto is_empty(AN&&... an)
->     operator_factory<is_empty_tag, AN...> {
    return operator_factory<is_empty_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<all_tag>
{
    template<class Observable, class Predicate,
        class SourceValue = rxu::value_type_t<Observable>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class All = rxo::detail::all<SourceValue, rxu::decay_t<Predicate>>,
        class Value = rxu::value_type_t<All>>
    static auto member(Observable&& o, Predicate&& p)
        -> decltype(o.template lift<Value>(All(std::forward<Predicate>(p)))) {
        return      o.template lift<Value>(All(std::forward<Predicate>(p)));
    }

    template<class... AN>
    static operators::detail::all_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "all takes (Predicate)");
    }
};

template<>
struct member_overload<is_empty_tag>
{
    template<class Observable,
        class SourceValue = rxu::value_type_t<Observable>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class Predicate = std::function<bool(SourceValue)>,
        class IsEmpty = rxo::detail::all<SourceValue, rxu::decay_t<Predicate>>,
        class Value = rxu::value_type_t<IsEmpty>>
    static auto member(Observable&& o)
    -> decltype(o.template lift<Value>(IsEmpty(nullptr))) {
        return  o.template lift<Value>(IsEmpty([](SourceValue) { return false; }));
    }

    template<class... AN>
    static operators::detail::all_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "is_empty takes no arguments");
    }
};

}

#endif
