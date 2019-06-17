// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_OPERATORS_HPP)
#define RXCPP_RX_OPERATORS_HPP

#include "rx-includes.hpp"

namespace rxcpp {

namespace operators {

struct tag_operator {};
template<class T>
struct operator_base
{
    typedef T value_type;
    typedef tag_operator operator_tag;
};

namespace detail {

template<class T, class =rxu::types_checked>
struct is_operator : std::false_type
{
};

template<class T>
struct is_operator<T, rxu::types_checked_t<typename T::operator_tag>> 
    : std::is_convertible<typename T::operator_tag*, tag_operator*>
{
};

}

template<class T, class Decayed = rxu::decay_t<T>>
struct is_operator : detail::is_operator<Decayed>
{
};


}
namespace rxo=operators;

template<class Tag> 
struct member_overload
{
    template<class... AN>
    static auto member(AN&&...) ->
                typename Tag::template include_header<std::false_type> {
        return  typename Tag::template include_header<std::false_type>();
    }
};

template<class T, class... AN>
struct delayed_type{using value_type = T; static T value(AN**...) {return T{};}};

template<class T, class... AN>
using delayed_type_t = rxu::value_type_t<delayed_type<T, AN...>>;

template<class Tag, class... AN, class Overload = member_overload<rxu::decay_t<Tag>>>
auto observable_member(Tag, AN&&... an) -> 
    decltype(Overload::member(std::forward<AN>(an)...)) {
    return   Overload::member(std::forward<AN>(an)...);
}

template<class Tag, class... AN>
class operator_factory
{
    using this_type = operator_factory<Tag, AN...>;
    using tag_type = rxu::decay_t<Tag>;
    using tuple_type = std::tuple<rxu::decay_t<AN>...>;
    
    tuple_type an;

public:
    operator_factory(tuple_type an)
        : an(std::move(an))
    {
    }

    template<class... ZN>
    auto operator()(tag_type t, ZN&&... zn) const
        -> decltype(observable_member(t, std::forward<ZN>(zn)...)) {
        return      observable_member(t, std::forward<ZN>(zn)...);
    }

    template<class Observable>
    auto operator()(Observable source) const 
        -> decltype(rxu::apply(std::tuple_cat(std::make_tuple(tag_type{}, source), (*(tuple_type*)nullptr)), (*(this_type*)nullptr))) {
        return      rxu::apply(std::tuple_cat(std::make_tuple(tag_type{}, source),                      an),                  *this);
    }
};

}

#include "operators/rx-lift.hpp"
#include "operators/rx-subscribe.hpp"

namespace rxcpp {

struct amb_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-amb.hpp>");
    };
};

struct all_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-all.hpp>");
    };
};

struct is_empty_tag : all_tag {};

struct any_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-any.hpp>");
    };
};

struct exists_tag : any_tag {};
struct contains_tag : any_tag {};

struct buffer_count_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-buffer_count.hpp>");
    };
};

struct buffer_with_time_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-buffer_time.hpp>");
    };
};

struct buffer_with_time_or_count_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-buffer_time_count.hpp>");
    };
};

struct combine_latest_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-combine_latest.hpp>");
    };
};

struct concat_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-concat.hpp>");
    };
};

struct concat_map_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-concat_map.hpp>");
    };
};

struct connect_forever_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-connect_forever.hpp>");
    };
};

struct debounce_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-debounce.hpp>");
    };
};

struct delay_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-delay.hpp>");
    };
};

struct distinct_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-distinct.hpp>");
    };
};

struct distinct_until_changed_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-distinct_until_changed.hpp>");
    };
};

struct element_at_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-element_at.hpp>");
    };
};

struct filter_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-filter.hpp>");
    };
};

struct finally_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-finally.hpp>");
    };
};

struct flat_map_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-flat_map.hpp>");
    };
};

struct group_by_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-group_by.hpp>");
    };
};

struct ignore_elements_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-ignore_elements.hpp>");
    };
};

struct map_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-map.hpp>");
    };
};

struct merge_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-merge.hpp>");
    };
};
struct merge_delay_error_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-merge_delay_error.hpp>");
    };
};

struct multicast_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-multicast.hpp>");
    };
};

struct observe_on_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-observe_on.hpp>");
    };
};

struct on_error_resume_next_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-on_error_resume_next.hpp>");
    };
};

class empty_error: public std::runtime_error
{
    public:
        explicit empty_error(const std::string& msg):
            std::runtime_error(msg)
        {}
};
struct reduce_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-reduce.hpp>");
    };
};
struct first_tag : reduce_tag {};
struct last_tag : reduce_tag {};
struct sum_tag : reduce_tag {};
struct average_tag : reduce_tag {};
struct min_tag : reduce_tag {};
struct max_tag : reduce_tag {};

struct ref_count_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-ref_count.hpp>");
    };
};

struct pairwise_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-pairwise.hpp>");
    };
};

struct publish_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-publish.hpp>");
    };
};
struct publish_synchronized_tag : publish_tag {};
    
struct repeat_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-repeat.hpp>");
    };
};

struct replay_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-replay.hpp>");
    };
};

struct retry_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-retry.hpp>");
    };
};

struct sample_with_time_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-sample_time.hpp>");
    };
};

struct scan_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-scan.hpp>");
    };
};

struct sequence_equal_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-sequence_equal.hpp>");
    };
};

struct skip_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-skip.hpp>");
    };
};

struct skip_while_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-skip_while.hpp>");
    };
};

struct skip_last_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-skip_last.hpp>");
    };
};

struct skip_until_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-skip_until.hpp>");
    };
};

struct start_with_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-start_with.hpp>");
    };
};

struct subscribe_on_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-subscribe_on.hpp>");
    };
};

struct switch_if_empty_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-switch_if_empty.hpp>");
    };
};
struct default_if_empty_tag : switch_if_empty_tag {};

struct switch_on_next_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-switch_on_next.hpp>");
    };
};

struct take_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-take.hpp>");
    };
};

struct take_last_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-take_last.hpp>");
    };
};

struct take_while_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-take_while.hpp>");
    };
};

struct take_until_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-take_until.hpp>");
    };
};

struct tap_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-rap.hpp>");
    };
};

struct timeout_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-timeout.hpp>");
    };
};

struct time_interval_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-time_interval.hpp>");
    };
};

struct timestamp_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-timestamp.hpp>");
    };
};

struct window_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-window.hpp>");
    };
};

struct window_with_time_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-window_time.hpp>");
    };
};

struct window_with_time_or_count_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-window_time_count.hpp>");
    };
};

struct window_toggle_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-window_toggle.hpp>");
    };
};

struct with_latest_from_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-with_latest_from.hpp>");
    };
};

struct zip_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include <rxcpp/operators/rx-zip.hpp>");
    };
};

}

#include "operators/rx-multicast.hpp"
#include "operators/rx-publish.hpp"
#include "operators/rx-ref_count.hpp"

#endif
