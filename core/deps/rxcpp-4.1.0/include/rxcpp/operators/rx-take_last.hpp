// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-take_last.hpp

    \brief Emit only the final t items emitted by the source Observable.

    \tparam Count  the type of the items counter.

    \param t  the number of last items to take.

    \return  An observable that emits only the last t items emitted by the source Observable, or all of the items from the source observable if that observable emits fewer than t items.

    \sample
    \snippet take_last.cpp take_last sample
    \snippet output.txt take_last sample
*/

#if !defined(RXCPP_OPERATORS_RX_TAKE_LAST_HPP)
#define RXCPP_OPERATORS_RX_TAKE_LAST_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct take_last_invalid_arguments {};

template<class... AN>
struct take_last_invalid : public rxo::operator_base<take_last_invalid_arguments<AN...>> {
    using type = observable<take_last_invalid_arguments<AN...>, take_last_invalid<AN...>>;
};
template<class... AN>
using take_last_invalid_t = typename take_last_invalid<AN...>::type;

template<class T, class Observable, class Count>
struct take_last : public operator_base<T>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Count> count_type;

    typedef std::queue<T> queue_type;
    typedef typename queue_type::size_type queue_size_type;

    struct values
    {
        values(source_type s, count_type t)
            : source(std::move(s))
            , count(static_cast<queue_size_type>(t))
        {
        }
        source_type source;
        queue_size_type count;
    };
    values initial;

    take_last(source_type s, count_type t)
        : initial(std::move(s), std::move(t))
    {
    }

    template<class Subscriber>
    void on_subscribe(const Subscriber& s) const {

        typedef Subscriber output_type;
        struct state_type
            : public std::enable_shared_from_this<state_type>
            , public values
        {
            state_type(const values& i, const output_type& oarg)
                : values(i)
                , out(oarg)
            {
            }
            queue_type items;
            output_type out;
        };
        // take a copy of the values for each subscription
        auto state = std::make_shared<state_type>(initial, s);

        composite_subscription source_lifetime;

        s.add(source_lifetime);

        state->source.subscribe(
        // split subscription lifetime
            source_lifetime,
        // on_next
            [state, source_lifetime](T t) {
                if(state->count > 0) {
                    if (state->items.size() == state->count) {
                        state->items.pop();
                    }
                    state->items.push(t);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                while(!state->items.empty()) {
                    state->out.on_next(std::move(state->items.front()));
                    state->items.pop();
                }
                state->out.on_completed();
            }
        );
    }
};

}

/*! @copydoc rx-take_last.hpp
*/
template<class... AN>
auto take_last(AN&&... an)
    ->     operator_factory<take_last_tag, AN...> {
    return operator_factory<take_last_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<take_last_tag>
{
    template<class Observable,
        class Count,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class TakeLast = rxo::detail::take_last<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
        class Value = rxu::value_type_t<TakeLast>,
        class Result = observable<Value, TakeLast>>
    static Result member(Observable&& o, Count&& c) {
        return Result(TakeLast(std::forward<Observable>(o), std::forward<Count>(c)));
    }

    template<class... AN>
    static operators::detail::take_last_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "take_last takes (Count)");
    }
};

}

#endif
