// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-take.hpp

    \brief For the first count items from this observable emit them from the new observable that is returned.

    \tparam Count the type of the items counter.

    \param t the number of items to take.

    \return An observable that emits only the first t items emitted by the source Observable, or all of the items from the source observable if that observable emits fewer than t items.

    \sample
    \snippet take.cpp take sample
    \snippet output.txt take sample
*/

#if !defined(RXCPP_OPERATORS_RX_TAKE_HPP)
#define RXCPP_OPERATORS_RX_TAKE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct take_invalid_arguments {};

template<class... AN>
struct take_invalid : public rxo::operator_base<take_invalid_arguments<AN...>> {
    using type = observable<take_invalid_arguments<AN...>, take_invalid<AN...>>;
};
template<class... AN>
using take_invalid_t = typename take_invalid<AN...>::type;

template<class T, class Observable, class Count>
struct take : public operator_base<T>
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

    take(source_type s, count_type t)
        : initial(std::move(s), std::move(t))
    {
    }

    struct mode
    {
        enum type {
            taking,    // capture messages
            triggered, // ignore messages
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
                , mode_value(mode::taking)
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
            [state, source_lifetime](T t) {
                if (state->mode_value < mode::triggered) {
                    if (--state->count > 0) {
                        state->out.on_next(t);
                    } else {
                        state->mode_value = mode::triggered;
                        state->out.on_next(t);
                        // must shutdown source before signaling completion
                        source_lifetime.unsubscribe();
                        state->out.on_completed();
                    }
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

/*! @copydoc rx-take.hpp
*/
template<class... AN>
auto take(AN&&... an)
    ->     operator_factory<take_tag, AN...> {
    return operator_factory<take_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<take_tag>
{
    template<class Observable,
        class Count,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Take = rxo::detail::take<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
        class Value = rxu::value_type_t<Take>,
        class Result = observable<Value, Take>>
    static Result member(Observable&& o, Count&& c) {
        return Result(Take(std::forward<Observable>(o), std::forward<Count>(c)));
    }

    template<class... AN>
    static operators::detail::take_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "take takes (optional Count)");
    }
};

}

#endif
