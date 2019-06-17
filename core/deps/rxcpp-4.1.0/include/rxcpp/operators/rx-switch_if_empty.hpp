// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-switch_if_empty.hpp

    \brief If the source Observable terminates without emitting any items, emits items from a backup Observable.

    \tparam BackupSource the type of the backup observable.

    \param t a backup observable that is used if the source observable is empty.

    \return Observable that emits items from a backup observable if the source observable is empty.

    \sample
    \snippet switch_if_empty.cpp switch_if_empty sample
    \snippet output.txt switch_if_empty sample
*/

#if !defined(RXCPP_OPERATORS_RX_SWITCH_IF_EMPTY_HPP)
#define RXCPP_OPERATORS_RX_SWITCH_IF_EMPTY_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct switch_if_empty_invalid_arguments {};

template<class... AN>
struct switch_if_empty_invalid : public rxo::operator_base<switch_if_empty_invalid_arguments<AN...>> {
    using type = observable<switch_if_empty_invalid_arguments<AN...>, switch_if_empty_invalid<AN...>>;
};
template<class... AN>
using switch_if_empty_invalid_t = typename switch_if_empty_invalid<AN...>::type;

template<class T, class BackupSource>
struct switch_if_empty
{
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<BackupSource> backup_source_type;

    backup_source_type backup;

    switch_if_empty(backup_source_type b)
        : backup(std::move(b))
    {
    }

    template<class Subscriber>
    struct switch_if_empty_observer
    {
        typedef switch_if_empty_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;

        dest_type dest;
        composite_subscription lifetime;
        backup_source_type backup;
        mutable bool is_empty;

        switch_if_empty_observer(dest_type d, composite_subscription cs, backup_source_type b)
            : dest(std::move(d))
            , lifetime(std::move(cs))
            , backup(std::move(b))
            , is_empty(true)
        {
            dest.add(lifetime);
        }
        void on_next(source_value_type v) const {
            is_empty = false;
            dest.on_next(std::move(v));
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(std::move(e));
        }
        void on_completed() const {
            if(!is_empty) {
                dest.on_completed();
            } else {
                backup.subscribe(dest);
            }
        }

        static subscriber<value_type, observer_type> make(dest_type d, backup_source_type b) {
            auto cs = composite_subscription();
            return make_subscriber<value_type>(cs, observer_type(this_type(std::move(d), cs, std::move(b))));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(switch_if_empty_observer<Subscriber>::make(std::move(dest), std::move(backup))) {
        return      switch_if_empty_observer<Subscriber>::make(std::move(dest), std::move(backup));
    }
};

}

/*! @copydoc rx-switch_if_empty.hpp
*/
template<class... AN>
auto switch_if_empty(AN&&... an)
    ->      operator_factory<switch_if_empty_tag, AN...> {
     return operator_factory<switch_if_empty_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

/*! \brief If the source Observable terminates without emitting any items, emits a default item and completes.

    \tparam Value the type of the value to emit.

    \param v the default value to emit.

    \return Observable that emits the specified default item if the source observable is empty.

    \sample
    \snippet default_if_empty.cpp default_if_empty sample
    \snippet output.txt default_if_empty sample
*/
template<class... AN>
auto default_if_empty(AN&&... an)
    ->      operator_factory<default_if_empty_tag, AN...> {
     return operator_factory<default_if_empty_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<switch_if_empty_tag>
{
    template<class Observable, class BackupSource,
        class Enabled = rxu::enable_if_all_true_type_t<
            all_observables<Observable, BackupSource>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SwitchIfEmpty = rxo::detail::switch_if_empty<SourceValue, rxu::decay_t<BackupSource>>>
    static auto member(Observable&& o, BackupSource&& b)
        -> decltype(o.template lift<SourceValue>(SwitchIfEmpty(std::forward<BackupSource>(b)))) {
        return      o.template lift<SourceValue>(SwitchIfEmpty(std::forward<BackupSource>(b)));
    }

    template<class... AN>
    static operators::detail::switch_if_empty_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "switch_if_empty takes (BackupSource)");
    }
};

template<>
struct member_overload<default_if_empty_tag>
{
    template<class Observable, class Value,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BackupSource = decltype(rxs::from(std::declval<SourceValue>())),
        class DefaultIfEmpty = rxo::detail::switch_if_empty<SourceValue, BackupSource>>
    static auto member(Observable&& o, Value v)
        -> decltype(o.template lift<SourceValue>(DefaultIfEmpty(rxs::from(std::move(v))))) {
        return      o.template lift<SourceValue>(DefaultIfEmpty(rxs::from(std::move(v))));
    }

    template<class... AN>
    static operators::detail::switch_if_empty_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "default_if_empty takes (Value)");
    }
};

}

#endif
