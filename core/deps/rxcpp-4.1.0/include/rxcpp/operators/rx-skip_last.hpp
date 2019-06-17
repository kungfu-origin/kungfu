// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-skip_last.hpp

    \brief Make new observable with skipped last count items from this observable.

    \tparam Count the type of the items counter.

    \param  t the number of last items to skip.

    \return  An observable that is identical to the source observable except that it does not emit the last t items that the source observable emits.

    \sample
    \snippet skip_last.cpp skip_last sample
    \snippet output.txt skip_last sample
*/

#if !defined(RXCPP_OPERATORS_RX_SKIP_LAST_HPP)
#define RXCPP_OPERATORS_RX_SKIP_LAST_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct skip_last_invalid_arguments {};

template<class... AN>
struct skip_last_invalid : public rxo::operator_base<skip_last_invalid_arguments<AN...>> {
    using type = observable<skip_last_invalid_arguments<AN...>, skip_last_invalid<AN...>>;
};
template<class... AN>
using skip_last_invalid_t = typename skip_last_invalid<AN...>::type;

template<class T, class Observable, class Count>
struct skip_last : public operator_base<T>
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

    skip_last(source_type s, count_type t)
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
            [state](T t) {
                if(state->count > 0) {
                    if (state->items.size() == state->count) {
                        state->out.on_next(std::move(state->items.front()));
                        state->items.pop();
                    }
                    state->items.push(t);
                } else {
                    state->out.on_next(t);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                state->out.on_completed();
            }
        );
    }
};

}

/*! @copydoc rx-skip_last.hpp
*/
template<class... AN>
auto skip_last(AN&&... an)
    ->     operator_factory<skip_last_tag, AN...> {
    return operator_factory<skip_last_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<skip_last_tag>
{
    template<class Observable, class Count,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SkipLast = rxo::detail::skip_last<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
        class Value = rxu::value_type_t<SkipLast>,
        class Result = observable<Value, SkipLast>>
    static Result member(Observable&& o, Count&& c) {
        return Result(SkipLast(std::forward<Observable>(o), std::forward<Count>(c)));
    }

    template<class... AN>
    static operators::detail::skip_last_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "skip_last takes (Count)");
    }
};

}

#endif
