// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-window.hpp

    \brief Return an observable that emits connected, non-overlapping windows, each containing at most count items from the source observable.
           If the skip parameter is set, return an observable that emits windows every skip items containing at most count items from the source observable.

    \param count  the maximum size of each window before it should be completed
    \param skip   how many items need to be skipped before starting a new window

    \return  Observable that emits connected, non-overlapping windows, each containing at most count items from the source observable.
             If the skip parameter is set, return an Observable that emits windows every skip items containing at most count items from the source observable.

    \sample
    \snippet window.cpp window count+skip sample
    \snippet output.txt window count+skip sample

    \sample
    \snippet window.cpp window count sample
    \snippet output.txt window count sample
*/

#if !defined(RXCPP_OPERATORS_RX_WINDOW_HPP)
#define RXCPP_OPERATORS_RX_WINDOW_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct window_invalid_arguments {};

template<class... AN>
struct window_invalid : public rxo::operator_base<window_invalid_arguments<AN...>> {
    using type = observable<window_invalid_arguments<AN...>, window_invalid<AN...>>;
};
template<class... AN>
using window_invalid_t = typename window_invalid<AN...>::type;

template<class T>
struct window
{
    typedef rxu::decay_t<T> source_value_type;
    typedef observable<source_value_type> value_type;

    struct window_values
    {
        window_values(int c, int s)
            : count(c)
            , skip(s)
        {
        }
        int count;
        int skip;
    };

    window_values initial;

    window(int count, int skip)
        : initial(count, skip)
    {
    }

    template<class Subscriber>
    struct window_observer : public window_values
    {
        typedef window_observer<Subscriber> this_type;
        typedef rxu::decay_t<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;
        dest_type dest;
        mutable int cursor;
        mutable std::deque<rxcpp::subjects::subject<T>> subj;

        window_observer(dest_type d, window_values v)
            : window_values(v)
            , dest(std::move(d))
            , cursor(0)
        {
            subj.push_back(rxcpp::subjects::subject<T>());
            dest.on_next(subj[0].get_observable().as_dynamic());
        }
        void on_next(T v) const {
            for (auto s : subj) {
                s.get_subscriber().on_next(v);
            }

            int c = cursor - this->count + 1;
            if (c >= 0 && c % this->skip == 0) {
                subj[0].get_subscriber().on_completed();
                subj.pop_front();
            }

            if (++cursor % this->skip == 0) {
                subj.push_back(rxcpp::subjects::subject<T>());
                dest.on_next(subj[subj.size() - 1].get_observable().as_dynamic());
            }
        }

        void on_error(std::exception_ptr e) const {
            for (auto s : subj) {
                s.get_subscriber().on_error(e);
            }
            dest.on_error(e);
        }

        void on_completed() const {
            for (auto s : subj) {
                s.get_subscriber().on_completed();
            }
            dest.on_completed();
        }

        static subscriber<T, observer_type> make(dest_type d, window_values v) {
            auto cs = d.get_subscription();
            return make_subscriber<T>(std::move(cs), observer_type(this_type(std::move(d), std::move(v))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(window_observer<Subscriber>::make(std::move(dest), initial)) {
        return      window_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-window.hpp
*/
template<class... AN>
auto window(AN&&... an)
    ->      operator_factory<window_tag, AN...> {
     return operator_factory<window_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<window_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Window = rxo::detail::window<SourceValue>,
        class Value = rxu::value_type_t<Window>>
    static auto member(Observable&& o, int count, int skip)
        -> decltype(o.template lift<Value>(Window(count, skip))) {
        return      o.template lift<Value>(Window(count, skip));
    }

     template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Window = rxo::detail::window<SourceValue>,
        class Value = rxu::value_type_t<Window>>
    static auto member(Observable&& o, int count)
        -> decltype(o.template lift<Value>(Window(count, count))) {
        return      o.template lift<Value>(Window(count, count));
    }

    template<class... AN>
    static operators::detail::window_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "window takes (Count, optional Skip)");
    }
};

}

#endif
