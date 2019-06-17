// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_RUN_LOOP_HPP)
#define RXCPP_RX_SCHEDULER_RUN_LOOP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

namespace detail {

struct run_loop_state : public std::enable_shared_from_this<run_loop_state>
{
    typedef scheduler::clock_type clock_type;

    typedef detail::schedulable_queue<
        clock_type::time_point> queue_item_time;

    typedef queue_item_time::item_type item_type;
    typedef queue_item_time::const_reference const_reference_item_type;

    virtual ~run_loop_state()
    {
    }

    run_loop_state()
    {
    }

    composite_subscription lifetime;
    mutable std::mutex lock;
    mutable queue_item_time q;
    recursion r;
    std::function<void(clock_type::time_point)> notify_earlier_wakeup;
};

}


struct run_loop_scheduler : public scheduler_interface
{
private:
    typedef run_loop_scheduler this_type;
    run_loop_scheduler(const this_type&);

    struct run_loop_worker : public worker_interface
    {
    private:
        typedef run_loop_worker this_type;

        run_loop_worker(const this_type&);

    public:
        std::weak_ptr<detail::run_loop_state> state;

        virtual ~run_loop_worker()
        {
        }

        explicit run_loop_worker(std::weak_ptr<detail::run_loop_state> ws)
            : state(ws)
        {
        }

        virtual clock_type::time_point now() const {
            return clock_type::now();
        }

        virtual void schedule(const schedulable& scbl) const {
            schedule(now(), scbl);
        }

        virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
            if (scbl.is_subscribed()) {
                auto st = state.lock();
                std::unique_lock<std::mutex> guard(st->lock);
                const bool need_earlier_wakeup_notification = st->notify_earlier_wakeup &&
                                                              (st->q.empty() || when < st->q.top().when);
                st->q.push(detail::run_loop_state::item_type(when, scbl));
                st->r.reset(false);
                if (need_earlier_wakeup_notification) st->notify_earlier_wakeup(when);
                guard.unlock(); // So we can't get attempt to recursively lock the state
            }
        }
    };

    std::weak_ptr<detail::run_loop_state> state;

public:
    explicit run_loop_scheduler(std::weak_ptr<detail::run_loop_state> ws)
        : state(ws)
    {
    }
    virtual ~run_loop_scheduler()
    {
    }

    virtual clock_type::time_point now() const {
        return clock_type::now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        auto lifetime = state.lock()->lifetime;
        auto token = lifetime.add(cs);
        cs.add([=](){lifetime.remove(token);});
        return worker(cs, create_worker_interface());
    }

    std::shared_ptr<worker_interface> create_worker_interface() const {
        return std::make_shared<run_loop_worker>(state);
    }
};

class run_loop
{
private:
    typedef run_loop this_type;
    // don't allow this instance to copy/move since it owns current_thread queue
    // for the thread it is constructed on.
    run_loop(const this_type&);
    run_loop(this_type&&);

    typedef detail::action_queue queue_type;

    typedef detail::run_loop_state::item_type item_type;
    typedef detail::run_loop_state::const_reference_item_type const_reference_item_type;

    std::shared_ptr<detail::run_loop_state> state;
    std::shared_ptr<run_loop_scheduler> sc;

public:
    typedef scheduler::clock_type clock_type;
    run_loop()
        : state(std::make_shared<detail::run_loop_state>())
        , sc(std::make_shared<run_loop_scheduler>(state))
    {
        // take ownership so that the current_thread scheduler
        // uses the same queue on this thread
        queue_type::ensure(sc->create_worker_interface());
    }
    ~run_loop()
    {
        state->lifetime.unsubscribe();

        std::unique_lock<std::mutex> guard(state->lock);

        // release ownership
        queue_type::destroy();

        auto expired = std::move(state->q);
        if (!state->q.empty()) std::terminate();
    }

    clock_type::time_point now() const {
        return clock_type::now();
    }
    
    composite_subscription get_subscription() const {
        return state->lifetime;
    }
    
    bool empty() const {
        return state->q.empty();
    }

    const_reference_item_type peek() const {
        return state->q.top();
    }

    void dispatch() const {
        std::unique_lock<std::mutex> guard(state->lock);
        if (state->q.empty()) {
            return;
        }
        auto& peek = state->q.top();
        if (!peek.what.is_subscribed()) {
            state->q.pop();
            return;
        }
        if (clock_type::now() < peek.when) {
            return;
        }
        auto what = peek.what;
        state->q.pop();
        state->r.reset(state->q.empty());
        guard.unlock();
        what(state->r.get_recurse());
    }

    scheduler get_scheduler() const {
        return make_scheduler(sc);
    }

    void set_notify_earlier_wakeup(std::function<void(clock_type::time_point)> const& f) {
        std::unique_lock<std::mutex> guard(state->lock);
        state->notify_earlier_wakeup = f;
    }
};

inline scheduler make_run_loop(const run_loop& r) {
    return r.get_scheduler();
}

}

}

#endif
