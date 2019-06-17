// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-coroutine.hpp

    \brief The proposal to add couroutines to the standard adds `co_await`, `for co_await`, `co_yield` and `co_return`. This file adds `begin(observable<>)` & `end(observable<>)` which enables `for co_await` to work with the `observable<>` type.

    for co_await (auto c : interval(seconds(1), observe_on_event_loop()) | take(3)) {
        printf("%d\n", c);
    }

*/

#if !defined(RXCPP_RX_COROUTINE_HPP)
#define RXCPP_RX_COROUTINE_HPP

#include "rx-includes.hpp"

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED

#include <rxcpp/operators/rx-finally.hpp>

#include <experimental/resumable>

namespace rxcpp {
namespace coroutine {

using namespace std;
using namespace std::chrono;
using namespace std::experimental;

template<typename Source>
struct co_observable_iterator;

template<typename Source>
struct co_observable_iterator_state : std::enable_shared_from_this<co_observable_iterator_state<Source>>
{
    using value_type = typename Source::value_type;

    ~co_observable_iterator_state() {
        lifetime.unsubscribe();
    }
    explicit co_observable_iterator_state(const Source& o) : o(o) {}

    coroutine_handle<> caller{};
    composite_subscription lifetime{};
    const value_type* value{nullptr};
    exception_ptr error{nullptr};
    Source o;
};

template<typename Source>
struct co_observable_inc_awaiter
{
    bool await_ready() {
        return false;
    }

    bool await_suspend(coroutine_handle<> handle) {
        if (!state->lifetime.is_subscribed()) {return false;}
        state->caller = handle;
        return true;
    }

    co_observable_iterator<Source> await_resume();

    shared_ptr<co_observable_iterator_state<Source>> state;
};

template<typename Source>
struct co_observable_iterator : public iterator<input_iterator_tag, typename Source::value_type>
{
    using value_type = typename Source::value_type;

    co_observable_iterator() {}

    explicit co_observable_iterator(const Source& o) : state(make_shared<co_observable_iterator_state<Source>>(o)) {}
    explicit co_observable_iterator(const shared_ptr<co_observable_iterator_state<Source>>& o) : state(o) {}

    co_observable_iterator(co_observable_iterator&&)=default;
    co_observable_iterator& operator=(co_observable_iterator&&)=default;

    co_observable_inc_awaiter<Source> operator++()
    {
        return co_observable_inc_awaiter<Source>{state};
    }

    co_observable_iterator& operator++(int) = delete;
    // not implementing postincrement

    bool operator==(co_observable_iterator const &rhs) const
    {
        return !!state && !rhs.state && !state->lifetime.is_subscribed();
    }

    bool operator!=(co_observable_iterator const &rhs) const
    {
        return !(*this == rhs);
    }

    value_type const &operator*() const
    {
        return *(state->value);
    }

    value_type const *operator->() const
    {
        return std::addressof(operator*());
    }

    shared_ptr<co_observable_iterator_state<Source>> state;
};

template<typename Source>
co_observable_iterator<Source> co_observable_inc_awaiter<Source>::await_resume() {
    if (!!state->error) {rethrow_exception(state->error);}
    return co_observable_iterator<Source>{state};
}

template<typename Source>
struct co_observable_iterator_awaiter
{
    using iterator=co_observable_iterator<Source>;
    using value_type=typename iterator::value_type;

    explicit co_observable_iterator_awaiter(const Source& o) : it(o) {
    }

    bool await_ready() {
        return false;
    }

    void await_suspend(coroutine_handle<> handle) {
        weak_ptr<co_observable_iterator_state<Source>> wst=it.state;
        it.state->caller = handle;
        it.state->o |
            rxo::finally([wst](){
                auto st = wst.lock();
                if (st && !!st->caller) {
                    auto caller = st->caller;
                    st->caller = nullptr;
                    caller();
                }
            }) |
            rxo::subscribe<value_type>(
                it.state->lifetime,
                // next
                [wst](const value_type& v){
                    auto st = wst.lock();
                    if (!st || !st->caller) {terminate();}
                    st->value = addressof(v);
                    auto caller = st->caller;
                    st->caller = nullptr;
                    caller();
                },
                // error
                [wst](exception_ptr e){
                    auto st = wst.lock();
                    if (!st || !st->caller) {terminate();}
                    st->error = e;
                    auto caller = st->caller;
                    st->caller = nullptr;
                    caller();
                });
    }

    iterator await_resume() {
        if (!!it.state->error) {rethrow_exception(it.state->error);}
        return std::move(it);
    }

    iterator it;
};

}
}

namespace std
{

template<typename T, typename SourceOperator>
auto begin(const rxcpp::observable<T, SourceOperator>& o)
    ->      rxcpp::coroutine::co_observable_iterator_awaiter<rxcpp::observable<T, SourceOperator>> {
    return  rxcpp::coroutine::co_observable_iterator_awaiter<rxcpp::observable<T, SourceOperator>>{o};
}

template<typename T, typename SourceOperator>
auto end(const rxcpp::observable<T, SourceOperator>&)
    ->      rxcpp::coroutine::co_observable_iterator<rxcpp::observable<T, SourceOperator>> {
    return  rxcpp::coroutine::co_observable_iterator<rxcpp::observable<T, SourceOperator>>{};
}

}

#endif

#endif
