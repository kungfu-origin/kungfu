#pragma once

/*! \file rx-retry-repeat-common.hpp

    \brief Implementation commonalities between retry and repeat operators abstracted away from rx-retry.hpp and rx-repeat.hpp files. Should be used only from rx-retry.hpp and rx-repeat.hpp
    
*/

#include "../rx-includes.hpp"

namespace rxcpp {
  namespace operators {
    namespace detail {

      namespace retry_repeat_common {
        // Structure to perform general retry/repeat operations on state
        template <class Values, class Subscriber, class EventHandlers, class T>
        struct state_type : public std::enable_shared_from_this<state_type<Values, Subscriber, EventHandlers, T>>,
                            public Values {

          typedef Subscriber output_type;
          state_type(const Values& i, const output_type& oarg)
            : Values(i),
              source_lifetime(composite_subscription::empty()),
              out(oarg) {
          }
          
          void do_subscribe() {
            auto state = this->shared_from_this();
                
            state->out.remove(state->lifetime_token);
            state->source_lifetime.unsubscribe();

            state->source_lifetime = composite_subscription();
            state->lifetime_token = state->out.add(state->source_lifetime);

            state->source.subscribe(
                                    state->out,
                                    state->source_lifetime,
                                    // on_next
                                    [state](T t) {
                                      state->out.on_next(t);
                                    },
                                    // on_error
                                    [state](std::exception_ptr e) {
                                      EventHandlers::on_error(state, e);
                                    },
                                    // on_completed
                                    [state]() {
                                      EventHandlers::on_completed(state);
                                    }
                                    );
          }
          
          composite_subscription source_lifetime;
          output_type out;
          composite_subscription::weak_subscription lifetime_token;
        };

        // Finite case (explicitely limited with the number of times)
        template <class EventHandlers, class T, class Observable, class Count>
        struct finite : public operator_base<T> {
          typedef rxu::decay_t<Observable> source_type;
          typedef rxu::decay_t<Count> count_type;

          struct values {
            values(source_type s, count_type t)
              : source(std::move(s)),
                remaining_(std::move(t)) {
            }

            inline bool completed_predicate() const {
              // Return true if we are completed
              return remaining_ <= 0;
            }
      
            inline void update() {
              // Decrement counter
              --remaining_;
            }

            source_type source;

          private:
            // Counter to hold number of times remaining to complete
            count_type remaining_;
          };
    
          finite(source_type s, count_type t)
            : initial_(std::move(s), std::move(t)) {
          }

          template<class Subscriber>
          void on_subscribe(const Subscriber& s) const {
            typedef state_type<values, Subscriber, EventHandlers, T> state_t;
            // take a copy of the values for each subscription
            auto state = std::make_shared<state_t>(initial_, s);      
            if (initial_.completed_predicate()) {
              // return completed
              state->out.on_completed();
            } else {
              // start the first iteration
              state->do_subscribe();
            }
          }

        private:
          values initial_;
        };

        // Infinite case
        template <class EventHandlers, class T, class Observable>
        struct infinite : public operator_base<T> {
          typedef rxu::decay_t<Observable> source_type;
    
          struct values {
            values(source_type s)
              : source(std::move(s)) {
            }

            static inline bool completed_predicate() {
              // Infinite never completes
              return false;
            }

            static inline void update() {
              // Infinite does not need to update state
            }

            source_type source;
          };
    
          infinite(source_type s) : initial_(std::move(s)) {
          }

          template<class Subscriber>
          void on_subscribe(const Subscriber& s) const {
            typedef state_type<values, Subscriber, EventHandlers, T> state_t;
            // take a copy of the values for each subscription
            auto state = std::make_shared<state_t>(initial_, s);
            // start the first iteration
            state->do_subscribe();
          }

        private:
          values initial_;
        };
        
        
      }
    }
  }
}
