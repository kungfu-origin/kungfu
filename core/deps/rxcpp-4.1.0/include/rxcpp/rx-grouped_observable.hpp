// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_GROUPED_OBSERVABLE_HPP)
#define RXCPP_RX_GROUPED_OBSERVABLE_HPP

#include "rx-includes.hpp"

namespace rxcpp {

namespace detail {

template<class K, class Source>
struct has_on_get_key_for
{
    struct not_void {};
    template<class CS>
    static auto check(int) -> decltype((*(CS*)nullptr).on_get_key());
    template<class CS>
    static not_void check(...);

    typedef decltype(check<Source>(0)) detail_result;
    static const bool value = std::is_same<detail_result, rxu::decay_t<K>>::value;
};

}

template<class K, class T>
class dynamic_grouped_observable
    : public dynamic_observable<T>
{
public:
    typedef rxu::decay_t<K> key_type;
    typedef tag_dynamic_grouped_observable dynamic_observable_tag;

private:
    struct state_type
        : public std::enable_shared_from_this<state_type>
    {
        typedef std::function<key_type()> ongetkey_type;

        ongetkey_type on_get_key;
    };
    std::shared_ptr<state_type> state;

    template<class U, class V>
    friend bool operator==(const dynamic_grouped_observable<U, V>&, const dynamic_grouped_observable<U, V>&);

    template<class U, class V>
    void construct(const dynamic_grouped_observable<U, V>& o, const tag_dynamic_grouped_observable&) {
        state = o.state;
    }

    template<class U, class V>
    void construct(dynamic_grouped_observable<U, V>&& o, const tag_dynamic_grouped_observable&) {
        state = std::move(o.state);
    }

    template<class SO>
    void construct(SO&& source, const rxs::tag_source&) {
        auto so = std::make_shared<rxu::decay_t<SO>>(std::forward<SO>(source));
        state->on_get_key = [so]() mutable {
            return so->on_get_key();
        };
    }

public:

    dynamic_grouped_observable()
    {
    }

    template<class SOF>
    explicit dynamic_grouped_observable(SOF sof)
        : dynamic_observable<T>(sof)
        , state(std::make_shared<state_type>())
    {
        construct(std::move(sof),
                  typename std::conditional<is_dynamic_grouped_observable<SOF>::value, tag_dynamic_grouped_observable, rxs::tag_source>::type());
    }

    template<class SF, class CF>
    dynamic_grouped_observable(SF&& sf, CF&& cf)
        : dynamic_observable<T>(std::forward<SF>(sf))
        , state(std::make_shared<state_type>())
    {
        state->on_connect = std::forward<CF>(cf);
    }

    using dynamic_observable<T>::on_subscribe;

    key_type on_get_key() const {
        return state->on_get_key();
    }
};

template<class K, class T>
inline bool operator==(const dynamic_grouped_observable<K, T>& lhs, const dynamic_grouped_observable<K, T>& rhs) {
    return lhs.state == rhs.state;
}
template<class K, class T>
inline bool operator!=(const dynamic_grouped_observable<K, T>& lhs, const dynamic_grouped_observable<K, T>& rhs) {
    return !(lhs == rhs);
}

template<class K, class T, class Source>
grouped_observable<K, T> make_dynamic_grouped_observable(Source&& s) {
    return grouped_observable<K, T>(dynamic_grouped_observable<K, T>(std::forward<Source>(s)));
}



/*!
    \brief a source of observables which each emit values from one category specified by the key selector.

    \ingroup group-observable

*/
template<class K, class T, class SourceOperator>
class grouped_observable
    : public observable<T, SourceOperator>
{
    typedef grouped_observable<K, T, SourceOperator> this_type;
    typedef observable<T, SourceOperator> base_type;
    typedef rxu::decay_t<SourceOperator> source_operator_type;

    static_assert(detail::has_on_get_key_for<K, source_operator_type>::value, "inner must have on_get_key method key_type()");

public:
    typedef rxu::decay_t<K> key_type;
    typedef tag_grouped_observable observable_tag;

    grouped_observable()
    {
    }

    explicit grouped_observable(const SourceOperator& o)
        : base_type(o)
    {
    }
    explicit grouped_observable(SourceOperator&& o)
        : base_type(std::move(o))
    {
    }

    // implicit conversion between observables of the same value_type
    template<class SO>
    grouped_observable(const grouped_observable<K, T, SO>& o)
        : base_type(o)
    {}
    // implicit conversion between observables of the same value_type
    template<class SO>
    grouped_observable(grouped_observable<K, T, SO>&& o)
        : base_type(std::move(o))
    {}

    ///
    /// performs type-forgetting conversion to a new grouped_observable
    ///
    grouped_observable<K, T> as_dynamic() const {
        return *this;
    }

    key_type get_key() const {
        return base_type::source_operator.on_get_key();
    }
};


}

//
// support range() >> filter() >> subscribe() syntax
// '>>' is spelled 'stream'
//
template<class K, class T, class SourceOperator, class OperatorFactory>
auto operator >> (const rxcpp::grouped_observable<K, T, SourceOperator>& source, OperatorFactory&& of)
    -> decltype(source.op(std::forward<OperatorFactory>(of))) {
    return      source.op(std::forward<OperatorFactory>(of));
}

//
// support range() | filter() | subscribe() syntax
// '|' is spelled 'pipe'
//
template<class K, class T, class SourceOperator, class OperatorFactory>
auto operator | (const rxcpp::grouped_observable<K, T, SourceOperator>& source, OperatorFactory&& of)
    -> decltype(source.op(std::forward<OperatorFactory>(of))) {
    return      source.op(std::forward<OperatorFactory>(of));
}

#endif
