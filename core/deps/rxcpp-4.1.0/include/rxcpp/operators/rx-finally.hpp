// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-finally.hpp

    \brief Add a new action at the end of the new observable that is returned.

    \tparam LastCall the type of the action function

    \param lc the action function

    \return Observable that emits the same items as the source observable, then invokes the given action.

    \sample
    \snippet finally.cpp finally sample
    \snippet output.txt finally sample

    If the source observable generates an error, the final action is still being called:
    \snippet finally.cpp error finally sample
    \snippet output.txt error finally sample
*/

#if !defined(RXCPP_OPERATORS_RX_FINALLY_HPP)
#define RXCPP_OPERATORS_RX_FINALLY_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct finally_invalid_arguments {};

template<class... AN>
struct finally_invalid : public rxo::operator_base<finally_invalid_arguments<AN...>> {
    using type = observable<finally_invalid_arguments<AN...>, finally_invalid<AN...>>;
};
template<class... AN>
using finally_invalid_t = typename finally_invalid<AN...>::type;

template<class T, class LastCall>
struct finally
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<LastCall> last_call_type;
    last_call_type last_call;

    finally(last_call_type lc)
        : last_call(std::move(lc))
    {
    }

    template<class Subscriber>
    struct finally_observer
    {
        typedef finally_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;

        finally_observer(dest_type d)
            : dest(std::move(d))
        {
        }
        void on_next(source_value_type v) const {
            dest.on_next(v);
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<value_type, observer_type> make(dest_type d, const last_call_type& lc) {
            auto dl = d.get_subscription();
            composite_subscription cs;
            dl.add(cs);
            cs.add([=](){
                dl.unsubscribe();
                lc();
            });
            return make_subscriber<value_type>(cs, this_type(d));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(finally_observer<Subscriber>::make(std::move(dest), last_call)) {
        return      finally_observer<Subscriber>::make(std::move(dest), last_call);
    }
};

}

/*! @copydoc rx-finally.hpp
*/
template<class... AN>
auto finally(AN&&... an)
    ->      operator_factory<finally_tag, AN...> {
     return operator_factory<finally_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<finally_tag>
{
    template<class Observable, class LastCall,
        class SourceValue = rxu::value_type_t<Observable>,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class Finally = rxo::detail::finally<SourceValue, rxu::decay_t<LastCall>>>
    static auto member(Observable&& o, LastCall&& lc)
        -> decltype(o.template lift<SourceValue>(Finally(std::forward<LastCall>(lc)))) {
        return      o.template lift<SourceValue>(Finally(std::forward<LastCall>(lc)));
    }

    template<class... AN>
    static operators::detail::finally_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "finally takes (LastCall)");
    }
};

}

#endif
