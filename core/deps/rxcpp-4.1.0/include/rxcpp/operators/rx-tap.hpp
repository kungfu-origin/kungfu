// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-tap.hpp

    \brief inspect calls to on_next, on_error and on_completed.

    \tparam MakeObserverArgN...  these args are passed to make_observer.

    \param an  these args are passed to make_observer.

    \return  Observable that emits the same items as the source observable to both the subscriber and the observer.

    \note If an on_error method is not supplied the observer will ignore errors rather than call std::terminate()

    \sample
    \snippet tap.cpp tap sample
    \snippet output.txt tap sample

    If the source observable generates an error, the observer passed to tap is called:
    \snippet tap.cpp error tap sample
    \snippet output.txt error tap sample
*/

#if !defined(RXCPP_OPERATORS_RX_TAP_HPP)
#define RXCPP_OPERATORS_RX_TAP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct tap_invalid_arguments {};

template<class... AN>
struct tap_invalid : public rxo::operator_base<tap_invalid_arguments<AN...>> {
    using type = observable<tap_invalid_arguments<AN...>, tap_invalid<AN...>>;
};
template<class... AN>
using tap_invalid_t = typename tap_invalid<AN...>::type;

template<class T, class MakeObserverArgN>
struct tap_observer_factory;

template<class T, class... ArgN>
struct tap_observer_factory<T, std::tuple<ArgN...>>
{
    using source_value_type = rxu::decay_t<T>;
    using out_type = decltype(make_observer<source_value_type, rxcpp::detail::OnErrorIgnore>(*((ArgN*)nullptr)...));
    auto operator()(ArgN&&... an) -> out_type const {
        return make_observer<source_value_type, rxcpp::detail::OnErrorIgnore>(std::forward<ArgN>(an)...);
    }
};

template<class T, class MakeObserverArgN, class Factory = tap_observer_factory<T, MakeObserverArgN>>
struct tap
{
    using source_value_type = rxu::decay_t<T>;
    using args_type = rxu::decay_t<MakeObserverArgN>;
    using factory_type = Factory;
    using out_type = typename factory_type::out_type;
    out_type out;

    tap(args_type a)
        : out(rxu::apply(std::move(a), factory_type()))
    {
    }

    template<class Subscriber>
    struct tap_observer
    {
        using this_type = tap_observer<Subscriber>;
        using value_type = source_value_type;
        using dest_type = rxu::decay_t<Subscriber>;
        using factory_type = Factory;
        using out_type = typename factory_type::out_type;
        using observer_type = observer<value_type, this_type>;
        dest_type dest;
        out_type out;

        tap_observer(dest_type d, out_type o)
            : dest(std::move(d))
            , out(std::move(o))
        {
        }
        void on_next(source_value_type v) const {
            out.on_next(v);
            dest.on_next(v);
        }
        void on_error(std::exception_ptr e) const {
            out.on_error(e);
            dest.on_error(e);
        }
        void on_completed() const {
            out.on_completed();
            dest.on_completed();
        }

        static subscriber<value_type, observer<value_type, this_type>> make(dest_type d, out_type o) {
            return make_subscriber<value_type>(d, this_type(d, std::move(o)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(tap_observer<Subscriber>::make(std::move(dest), out)) {
        return      tap_observer<Subscriber>::make(std::move(dest), out);
    }
};

}

/*! @copydoc rx-tap.hpp
*/
template<class... AN>
auto tap(AN&&... an)
    ->      operator_factory<tap_tag, AN...> {
     return operator_factory<tap_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<tap_tag>
{
    template<class Observable, class... MakeObserverArgN,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Tap = rxo::detail::tap<SourceValue, std::tuple<rxu::decay_t<MakeObserverArgN>...>>>
    static auto member(Observable&& o, MakeObserverArgN&&... an)
        -> decltype(o.template lift<SourceValue>(Tap(std::make_tuple(std::forward<MakeObserverArgN>(an)...)))) {
        return      o.template lift<SourceValue>(Tap(std::make_tuple(std::forward<MakeObserverArgN>(an)...)));
    }

    template<class... AN>
    static operators::detail::tap_invalid_t<AN...> member(const AN&...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "tap takes (MakeObserverArgN...)");
    }
};

}

#endif
