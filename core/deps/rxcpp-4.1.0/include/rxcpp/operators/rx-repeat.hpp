// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-repeat.hpp

    \brief Repeat this observable for the given number of times or infinitely.

    \tparam Count  the type of the counter (optional).

    \param t The number of times the source observable items are repeated (optional). If not specified, infinitely repeats the source observable. Specifying 0 returns an empty sequence immediately

    \return  An observable that repeats the sequence of items emitted by the source observable for t times.

    \sample
    \snippet repeat.cpp repeat count sample
    \snippet output.txt repeat count sample

    If the source observable calls on_error, repeat stops:
    \snippet repeat.cpp repeat error sample
    \snippet output.txt repeat error sample
*/

#if !defined(RXCPP_OPERATORS_RX_REPEAT_HPP)
#define RXCPP_OPERATORS_RX_REPEAT_HPP

#include "../rx-includes.hpp"
#include "rx-retry-repeat-common.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct repeat_invalid_arguments {};

template<class... AN>
struct repeat_invalid : public rxo::operator_base<repeat_invalid_arguments<AN...>> {
    using type = observable<repeat_invalid_arguments<AN...>, repeat_invalid<AN...>>;
};
template<class... AN>
using repeat_invalid_t = typename repeat_invalid<AN...>::type;

// Contain repeat variations in a namespace
namespace repeat {
  struct event_handlers {
    template <typename State>
    static inline void on_error(State& state, std::exception_ptr& e) {
      state->out.on_error(e);
    }
          
    template <typename State>
    static inline void on_completed(State& state) {
      // Functions update() and completed_predicate() vary between finite and infinte versions
      state->update();
      if (state->completed_predicate()) {
        state->out.on_completed();
      } else {
        state->do_subscribe();
      }
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

/*! @copydoc rx-repeat.hpp
*/
template<class... AN>
auto repeat(AN&&... an)
->     operator_factory<repeat_tag, AN...> {
    return operator_factory<repeat_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<repeat_tag> {
  template<class Observable,
           class Enabled = rxu::enable_if_all_true_type_t<is_observable<Observable>>,
           class SourceValue = rxu::value_type_t<Observable>,
           class Repeat = rxo::detail::repeat::infinite<SourceValue, rxu::decay_t<Observable>>,
           class Value = rxu::value_type_t<Repeat>,
           class Result = observable<Value, Repeat>>
  static Result member(Observable&& o) {
    return Result(Repeat(std::forward<Observable>(o)));
  }

  template<class Observable,
           class Count,
           class Enabled = rxu::enable_if_all_true_type_t<is_observable<Observable>>,
           class SourceValue = rxu::value_type_t<Observable>,
           class Repeat = rxo::detail::repeat::finite<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Count>>,
           class Value = rxu::value_type_t<Repeat>,
           class Result = observable<Value, Repeat>>
  static Result member(Observable&& o, Count&& c) {
    return Result(Repeat(std::forward<Observable>(o), std::forward<Count>(c)));
  }

  template<class... AN>
  static operators::detail::repeat_invalid_t<AN...> member(AN...) {
    std::terminate();
    return {};
    static_assert(sizeof...(AN) == 10000, "repeat takes (optional Count)");
  }
};

}

#endif
