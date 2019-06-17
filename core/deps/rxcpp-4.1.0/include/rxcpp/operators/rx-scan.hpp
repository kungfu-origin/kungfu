// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-scan.hpp

    \brief For each item from this observable use Accumulator to combine items into a value that will be emitted from the new observable that is returned.

    \tparam Seed         the type of the initial value for the accumulator.
    \tparam Accumulator  the type of the data accumulating function.

    \param seed  the initial value for the accumulator.
    \param a     an accumulator function to be invoked on each item emitted by the source observable, whose result will be emitted and used in the next accumulator call.

    \return  An observable that emits the results of each call to the accumulator function.

    \sample
    \snippet scan.cpp scan sample
    \snippet output.txt scan sample
*/

#if !defined(RXCPP_OPERATORS_RX_SCAN_HPP)
#define RXCPP_OPERATORS_RX_SCAN_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct scan_invalid_arguments {};

template<class... AN>
struct scan_invalid : public rxo::operator_base<scan_invalid_arguments<AN...>> {
    using type = observable<scan_invalid_arguments<AN...>, scan_invalid<AN...>>;
};
template<class... AN>
using scan_invalid_t = typename scan_invalid<AN...>::type;

template<class T, class Observable, class Accumulator, class Seed>
struct scan : public operator_base<rxu::decay_t<Seed>>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Accumulator> accumulator_type;
    typedef rxu::decay_t<Seed> seed_type;

    struct scan_initial_type
    {
        scan_initial_type(source_type o, accumulator_type a, seed_type s)
            : source(std::move(o))
            , accumulator(std::move(a))
            , seed(s)
        {
        }
        source_type source;
        accumulator_type accumulator;
        seed_type seed;
    };
    scan_initial_type initial;

    scan(source_type o, accumulator_type a, seed_type s)
        : initial(std::move(o), a, s)
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber o) const {
        struct scan_state_type
            : public scan_initial_type
            , public std::enable_shared_from_this<scan_state_type>
        {
            scan_state_type(scan_initial_type i, Subscriber scrbr)
                : scan_initial_type(i)
                , result(scan_initial_type::seed)
                , out(std::move(scrbr))
            {
            }
            seed_type result;
            Subscriber out;
        };
        auto state = std::make_shared<scan_state_type>(initial, std::move(o));
        state->source.subscribe(
            state->out,
        // on_next
            [state](T t) {
                state->result = state->accumulator(state->result, t);
                state->out.on_next(state->result);
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

/*! @copydoc rx-scan.hpp
*/
template<class... AN>
auto scan(AN&&... an)
    ->     operator_factory<scan_tag, AN...> {
    return operator_factory<scan_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<scan_tag>
{
    template<class Observable, class Seed, class Accumulator,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_accumulate_function_for<rxu::value_type_t<Observable>, rxu::decay_t<Seed>, rxu::decay_t<Accumulator>>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Scan = rxo::detail::scan<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Accumulator>, rxu::decay_t<Seed>>,
        class Value = rxu::value_type_t<Scan>,
        class Result = observable<Value, Scan>>
    static Result member(Observable&& o, Seed s, Accumulator&& a) {
        return Result(Scan(std::forward<Observable>(o), std::forward<Accumulator>(a), s));
    }

    template<class... AN>
    static operators::detail::scan_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "scan takes (Seed, Accumulator); Accumulator must be a function with the signature Seed(Seed, T)");
    }
};

}

#endif
