// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-pairwise.hpp

    \brief Take values pairwise from this observable.

    \return Observable that emits tuples of two the most recent items emitted by the source observable.

    \sample
    \snippet pairwise.cpp pairwise sample
    \snippet output.txt pairwise sample

    If the source observable emits less than two items, no pairs are emitted  by the source observable:
    \snippet pairwise.cpp pairwise short sample
    \snippet output.txt pairwise short sample
*/

#if !defined(RXCPP_OPERATORS_RX_PAIRWISE_HPP)
#define RXCPP_OPERATORS_RX_PAIRWISE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct pairwise_invalid_arguments {};

template<class... AN>
struct pairwise_invalid : public rxo::operator_base<pairwise_invalid_arguments<AN...>> {
    using type = observable<pairwise_invalid_arguments<AN...>, pairwise_invalid<AN...>>;
};
template<class... AN>
using pairwise_invalid_t = typename pairwise_invalid<AN...>::type;

template<class T>
struct pairwise
{
    typedef rxu::decay_t<T> source_value_type;
    typedef std::tuple<source_value_type, source_value_type> value_type;

    template<class Subscriber>
    struct pairwise_observer
    {
        typedef pairwise_observer<Subscriber> this_type;
        typedef std::tuple<source_value_type, source_value_type> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<T, this_type> observer_type;
        dest_type dest;
        mutable rxu::detail::maybe<source_value_type> remembered;

        pairwise_observer(dest_type d)
            : dest(std::move(d))
        {
        }
        void on_next(source_value_type v) const {
            if (remembered.empty()) {
                remembered.reset(v);
                return;
            }

            dest.on_next(std::make_tuple(remembered.get(), v));
            remembered.reset(v);
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<T, observer_type> make(dest_type d) {
            auto cs = d.get_subscription();
            return make_subscriber<T>(std::move(cs), observer_type(this_type(std::move(d))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(pairwise_observer<Subscriber>::make(std::move(dest))) {
        return      pairwise_observer<Subscriber>::make(std::move(dest));
    }
};

}

/*! @copydoc rx-pairwise.hpp
*/
template<class... AN>
auto pairwise(AN&&... an)
    ->     operator_factory<pairwise_tag, AN...> {
    return operator_factory<pairwise_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<pairwise_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class Pairwise = rxo::detail::pairwise<SourceValue>,
        class Value = rxu::value_type_t<Pairwise>>
    static auto member(Observable&& o)
    -> decltype(o.template lift<Value>(Pairwise())) {
        return  o.template lift<Value>(Pairwise());
    }

    template<class... AN>
    static operators::detail::pairwise_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "pairwise takes no arguments");
    }
};

}

#endif
