// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-lift.hpp

    \brief  takes any function that will take a subscriber for this observable and produce a subscriber.
            this is intended to allow externally defined operators, that use make_subscriber, to be connected into the expression.

    \tparam ResultType  the type of the emitted results.
    \tparam Operator    the type of the operator.

    \return An observable that emitting the items from its source.
 */

#if !defined(RXCPP_OPERATORS_RX_LIFT_HPP)
#define RXCPP_OPERATORS_RX_LIFT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace detail {

template<class V, class S, class F>
struct is_lift_function_for {

    struct tag_not_valid {};
    template<class CS, class CF>
    static auto check(int) -> decltype((*(CF*)nullptr)(*(CS*)nullptr));
    template<class CS, class CF>
    static tag_not_valid check(...);

    using for_type = rxu::decay_t<S>;
    using func_type = rxu::decay_t<F>;
    using detail_result = decltype(check<for_type, func_type>(0));

    static const bool value = rxu::all_true_type<
        is_subscriber<detail_result>,
        is_subscriber<for_type>,
        std::is_convertible<V, typename rxu::value_type_from<detail_result>::type>>::value;
};

}

namespace operators {

namespace detail {

template<class ResultType, class SourceOperator, class Operator>
struct lift_traits
{
    typedef rxu::decay_t<ResultType> result_value_type;
    typedef rxu::decay_t<SourceOperator> source_operator_type;
    typedef rxu::decay_t<Operator> operator_type;

    typedef typename source_operator_type::value_type source_value_type;
};

template<class ResultType, class SourceOperator, class Operator>
struct lift_operator : public operator_base<typename lift_traits<ResultType, SourceOperator, Operator>::result_value_type>
{
    typedef lift_traits<ResultType, SourceOperator, Operator> traits;
    typedef typename traits::source_operator_type source_operator_type;
    typedef typename traits::operator_type operator_type;
    source_operator_type source;
    operator_type chain;

    lift_operator(source_operator_type s, operator_type op)
        : source(std::move(s))
        , chain(std::move(op))
    {
    }
    template<class Subscriber>
    void on_subscribe(Subscriber o) const {
        auto lifted = chain(std::move(o));
        trace_activity().lift_enter(source, chain, o, lifted);
        source.on_subscribe(std::move(lifted));
        trace_activity().lift_return(source, chain);
    }
};

template<class ResultType, class Operator>
class lift_factory
{
    typedef rxu::decay_t<Operator> operator_type;
    operator_type chain;
public:
    lift_factory(operator_type op) : chain(std::move(op)) {}
    template<class Observable>
    auto operator()(const Observable& source)
        -> decltype(source.template lift<ResultType>(chain)) {
        return      source.template lift<ResultType>(chain);
        static_assert(rxcpp::detail::is_lift_function_for<rxu::value_type_t<Observable>, subscriber<ResultType>, Operator>::value, "Function passed for lift() must have the signature subscriber<...>(subscriber<T, ...>)");
    }
};

}

template<class ResultType, class Operator>
auto lift(Operator&& op)
    ->      detail::lift_factory<ResultType, Operator> {
    return  detail::lift_factory<ResultType, Operator>(std::forward<Operator>(op));
}

}

}

#endif
