// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-buffer_count.hpp

    \brief Return an observable that emits connected, non-overlapping buffer, each containing at most count items from the source observable.
           If the skip parameter is set, return an observable that emits buffers every skip items containing at most count items from the source observable.

    \param count  the maximum size of each buffers before it should be emitted.
    \param skip   how many items need to be skipped before starting a new buffers (optional).

    \return  Observable that emits connected, non-overlapping buffers, each containing at most count items from the source observable.
             If the skip parameter is set, return an Observable that emits buffers every skip items containing at most count items from the source observable.

    \sample
    \snippet buffer.cpp buffer count sample
    \snippet output.txt buffer count sample

    \sample
    \snippet buffer.cpp buffer count+skip sample
    \snippet output.txt buffer count+skip sample
*/

#if !defined(RXCPP_OPERATORS_RX_BUFFER_COUNT_HPP)
#define RXCPP_OPERATORS_RX_BUFFER_COUNT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct buffer_count_invalid_arguments {};

template<class... AN>
struct buffer_count_invalid : public rxo::operator_base<buffer_count_invalid_arguments<AN...>> {
    using type = observable<buffer_count_invalid_arguments<AN...>, buffer_count_invalid<AN...>>;
};
template<class... AN>
using buffer_count_invalid_t = typename buffer_count_invalid<AN...>::type;

template<class T>
struct buffer_count
{
    typedef rxu::decay_t<T> source_value_type;
    typedef std::vector<source_value_type> value_type;

    struct buffer_count_values
    {
        buffer_count_values(int c, int s)
            : count(c)
            , skip(s)
        {
        }
        int count;
        int skip;
    };

    buffer_count_values initial;

    buffer_count(int count, int skip)
        : initial(count, skip)
    {
    }

    template<class Subscriber>
    struct buffer_count_observer : public buffer_count_values
    {
        typedef buffer_count_observer<Subscriber> this_type;
        typedef std::vector<T> value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        typedef observer<value_type, this_type> observer_type;
        dest_type dest;
        mutable int cursor;
        mutable std::deque<value_type> chunks;

        buffer_count_observer(dest_type d, buffer_count_values v)
            : buffer_count_values(v)
            , dest(std::move(d))
            , cursor(0)
        {
        }
        void on_next(T v) const {
            if (cursor++ % this->skip == 0) {
                chunks.emplace_back();
            }
            for(auto& chunk : chunks) {
                chunk.push_back(v);
            }
            while (!chunks.empty() && int(chunks.front().size()) == this->count) {
                dest.on_next(std::move(chunks.front()));
                chunks.pop_front();
            }
        }
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }
        void on_completed() const {
            auto done = on_exception(
                [&](){
                    while (!chunks.empty()) {
                        dest.on_next(std::move(chunks.front()));
                        chunks.pop_front();
                    }
                    return true;
                },
                dest);
            if (done.empty()) {
                return;
            }
            dest.on_completed();
        }

        static subscriber<T, observer<T, this_type>> make(dest_type d, buffer_count_values v) {
            auto cs = d.get_subscription();
            return make_subscriber<T>(std::move(cs), this_type(std::move(d), std::move(v)));
        }
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const
        -> decltype(buffer_count_observer<Subscriber>::make(std::move(dest), initial)) {
        return      buffer_count_observer<Subscriber>::make(std::move(dest), initial);
    }
};

}

/*! @copydoc rx-buffer_count.hpp
*/
template<class... AN>
auto buffer(AN&&... an)
    ->      operator_factory<buffer_count_tag, AN...> {
     return operator_factory<buffer_count_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<buffer_count_tag>
{
    template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferCount = rxo::detail::buffer_count<SourceValue>,
        class Value = rxu::value_type_t<BufferCount>>
    static auto member(Observable&& o, int count, int skip)
        -> decltype(o.template lift<Value>(BufferCount(count, skip))) {
        return      o.template lift<Value>(BufferCount(count, skip));
    }

     template<class Observable,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class BufferCount = rxo::detail::buffer_count<SourceValue>,
        class Value = rxu::value_type_t<BufferCount>>
    static auto member(Observable&& o, int count)
        -> decltype(o.template lift<Value>(BufferCount(count, count))) {
        return      o.template lift<Value>(BufferCount(count, count));
    }

    template<class... AN>
    static operators::detail::buffer_count_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "buffer takes (Count, optional Skip)");
    }
};

}

#endif
