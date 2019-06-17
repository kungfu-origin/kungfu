// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_EVENT_LOOP_HPP)
#define RXCPP_RX_SCHEDULER_EVENT_LOOP_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

struct event_loop : public scheduler_interface
{
private:
    typedef event_loop this_type;
    event_loop(const this_type&);

    struct loop_worker : public worker_interface
    {
    private:
        typedef loop_worker this_type;
        loop_worker(const this_type&);

        typedef detail::schedulable_queue<
            typename clock_type::time_point> queue_item_time;

        typedef queue_item_time::item_type item_type;

        composite_subscription lifetime;
        worker controller;
        std::shared_ptr<const scheduler_interface> alive;

    public:
        virtual ~loop_worker()
        {
        }
        loop_worker(composite_subscription cs, worker w, std::shared_ptr<const scheduler_interface> alive)
            : lifetime(cs)
            , controller(w)
            , alive(alive)
        {
            auto token = controller.add(cs);
            cs.add([token, w](){
                w.remove(token);
            });
        }

        virtual clock_type::time_point now() const {
            return clock_type::now();
        }

        virtual void schedule(const schedulable& scbl) const {
            controller.schedule(lifetime, scbl.get_action());
        }

        virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
            controller.schedule(when, lifetime, scbl.get_action());
        }
    };

    mutable thread_factory factory;
    scheduler newthread;
    mutable std::atomic<std::size_t> count;
    composite_subscription loops_lifetime;
    std::vector<worker> loops;

public:
    event_loop()
        : factory([](std::function<void()> start){
            return std::thread(std::move(start));
        })
        , newthread(make_new_thread())
        , count(0)
    {
        auto remaining = std::max(std::thread::hardware_concurrency(), unsigned(4));
        while (remaining--) {
            loops.push_back(newthread.create_worker(loops_lifetime));
        }
    }
    explicit event_loop(thread_factory tf)
        : factory(tf)
        , newthread(make_new_thread(tf))
        , count(0)
    {
        auto remaining = std::max(std::thread::hardware_concurrency(), unsigned(4));
        while (remaining--) {
            loops.push_back(newthread.create_worker(loops_lifetime));
        }
    }
    virtual ~event_loop()
    {
        loops_lifetime.unsubscribe();
    }

    virtual clock_type::time_point now() const {
        return clock_type::now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        return worker(cs, std::make_shared<loop_worker>(cs, loops[++count % loops.size()], this->shared_from_this()));
    }
};

inline scheduler make_event_loop() {
    static scheduler instance = make_scheduler<event_loop>();
    return instance;
}
inline scheduler make_event_loop(thread_factory tf) {
    return make_scheduler<event_loop>(tf);
}

}

}

#endif
