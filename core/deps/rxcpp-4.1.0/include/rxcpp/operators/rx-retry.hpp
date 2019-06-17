// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-retry.hpp

    \brief Retry this observable for the given number of times.

    \tparam Count the type of the counter (optional)

    \param t  the total number of tries (optional), i.e. retry(2) means one normal try, before an error occurs, and one retry. If not specified, infinitely retries the source observable. Specifying 0 returns immediately without subscribing

    \return  An observable that mirrors the source observable, resubscribing to it if it calls on_error up to a specified number of retries.

    \sample
    \snippet retry.cpp retry count sample
    \snippet output.txt retry count sample
*/

#if !defined(RXCPP_OPERATORS_RX_RETRY_HPP)
#define RXCPP_OPERATORS_RX_RETRY_HPP

#include "../rx-includes.hpp"
#include "rx-retry-repeat-common.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct retry_invalid_arguments {};

template<class... AN>
struct retry_invalid : public rxo::operator_base<retry_invalid_arguments<AN...>> {
  using type = observable<retry_invalid_arguments<AN...>, retry_invalid<AN...>>;
};
template<class... AN>
using retry_invalid_t = typename retry_invalid<AN...>::type;

// Contain retry variations in a namespace
namespace retry {
  struct event_handlers {
    template <typename State>
    static inline void on_error(State& state, std::exception_ptr& e) {
      state->update();
      // Use specialized predicate for finite/infinte case
      if (state->completed_predicate()) {
        state->out.on_error(e);                                  
      } else {
        state->do_subscribe();
      }
    }
          
    template <typename State>
    static inline void on_completed(State& state) {
      state->out.on_completed();
    }
  };

  // Finite repeat case (explicitely limited with the number of times)
  template <class T, class Observable, class Count>
  using finite = ::rxcpp::operators::detail::retry_repeat_common::finite
    <event_handlers, T, Observable, Count>;
  
  // Infinite repeat case
  template <class T, class Observable>
  using infinite = ::rxcpp::operators::detail::retry_repeat_common::infinite
    <event_handlers, T, Observable>;
  
}
} // detail

/*! @copydoc rx-retry.hpp
*/
template<class... AN>
auto retry(AN&&... an)
->     operator_factory<retry_tag, AN...> {
    return operator_factory<retry_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}    

}

template<> 
struct member_overload<retry_tag>
{
  template<class Observable,
           class Enabled = rxu::enable_if_all_true_type_t<is_observable<Observable>>,
           class SourceValue = rxu::value_type_t<Observable>,
           class Retry = rxo::detail::retry::infinite<SourceValue, rxu::decay_t<Observable>>,
           class Value = rxu::value_type_t<Retry>,
           class Result = observable<Value, Retry>
           >
  static Result member(Observable&& o) {
    return Result(Retry(std::forward<Observable>(o)));
  }

  template<class Observable,
           class Count,
           class Enabled = rxu::enable_if_all_true_type_t<is_observable<Observable>>,
           class SourceValue = rxu::value_type_t<Observable>,
           class Retry = rxo::detail::retry::finite<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
           class Value = rxu::value_type_t<Retry>,
           class Result = observable<Value, Retry>
           >
  static Result member(Observable&& o, Count&& c) {
    return Result(Retry(std::forward<Observable>(o), std::forward<Count>(c)));
  }

  template<class... AN>
  static operators::detail::retry_invalid_t<AN...> member(const AN&...) {
    std::terminate();
    return {};
    static_assert(sizeof...(AN) == 10000, "retry takes (optional Count)");
  } 
};
    
}

#endif
