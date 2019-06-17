// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-skip.hpp

    \brief Make new observable with skipped first count items from this observable.

    \tparam Count the type of the items counter

    \param t the number of items to skip

    \return An observable that is identical to the source observable except that it does not emit the first t items that the source observable emits.

    \sample
    \snippet skip.cpp skip sample
    \snippet output.txt skip sample
*/

#if !defined(RXCPP_OPERATORS_RX_SKIP_HPP)
#define RXCPP_OPERATORS_RX_SKIP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct skip_invalid_arguments {};

template<class... AN>
struct skip_invalid : public rxo::operator_base<skip_invalid_arguments<AN...>> {
    using type = observable<skip_invalid_arguments<AN...>, skip_invalid<AN...>>;
};

template<class... AN>
using skip_invalid_t = typename skip_invalid<AN...>::type;

template<class T, class Observable, class Count>
struct skip : public operator_base<T>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Count> count_type;
    struct values
    {
        values(source_type s, count_type t)
            : source(std::move(s))
            , count(std::move(t))
        {
        }
        source_type source;
        count_type count;
    };
    values initial;

    skip(source_type s, count_type t)
        : initial(std::move(s), std::move(t))
    {
    }

    struct mode
    {
        enum type {
            skipping,  // ignore messages
            triggered, // capture messages
            errored,   // error occured
            stopped    // observable completed
        };
    };

    template<class Subscriber>
    void on_subscribe(const Subscriber& s) const {

        typedef Subscriber output_type;
        struct state_type
            : public std::enable_shared_from_this<state_type>
            , public values
        {
            state_type(const values& i, const output_type& oarg)
                : values(i)
                , mode_value(i.count > 0 ? mode::skipping : mode::triggered)
                , out(oarg)
            {
            }
            typename mode::type mode_value;
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
                if (state->mode_value == mode::skipping) {
                    if (--state->count == 0) {
                        state->mode_value = mode::triggered;
                    }
                } else {
                    state->out.on_next(t);
                }
            },
        // on_error
            [state](std::exception_ptr e) {
                state->mode_value = mode::errored;
                state->out.on_error(e);
            },
        // on_completed
            [state]() {
                state->mode_value = mode::stopped;
                state->out.on_completed();
            }
        );
    }
};

}

/*! @copydoc rx-skip.hpp
*/
template<class... AN>
auto skip(AN&&... an)
->     operator_factory<skip_tag, AN...> {
    return operator_factory<skip_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<skip_tag>
{
    template<class Observable,
            class Count,
            class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
            class SourceValue = rxu::value_type_t<Observable>,
            class Skip = rxo::detail::skip<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
            class Value = rxu::value_type_t<Skip>,
            class Result = observable<Value, Skip>>
    static Result member(Observable&& o, Count&& c) {
        return Result(Skip(std::forward<Observable>(o), std::forward<Count>(c)));
    }

    template<class... AN>
    static operators::detail::skip_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "skip takes (optional Count)");
    }
};

}

#endif
