// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_NEW_THREAD_HPP)
#define RXCPP_RX_SCHEDULER_NEW_THREAD_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

typedef std::function<std::thread(std::function<void()>)> thread_factory;

struct new_thread : public scheduler_interface
{
private:
    typedef new_thread this_type;
    new_thread(const this_type&);

    struct new_worker : public worker_interface
    {
    private:
        typedef new_worker this_type;

        typedef detail::action_queue queue_type;

        new_worker(const this_type&);

        struct new_worker_state : public std::enable_shared_from_this<new_worker_state>
        {
            typedef detail::schedulable_queue<
                typename clock_type::time_point> queue_item_time;

            typedef queue_item_time::item_type item_type;

            virtual ~new_worker_state()
            {
            }

            explicit new_worker_state(composite_subscription cs)
                : lifetime(cs)
            {
            }

            composite_subscription lifetime;
            mutable std::mutex lock;
            mutable std::condition_variable wake;
            mutable queue_item_time q;
            std::thread worker;
            recursion r;
        };

        std::shared_ptr<new_worker_state> state;

    public:
        virtual ~new_worker()
        {
        }

        explicit new_worker(std::shared_ptr<new_worker_state> ws)
            : state(ws)
        {
        }

        new_worker(composite_subscription cs, thread_factory& tf)
            : state(std::make_shared<new_worker_state>(cs))
        {
            auto keepAlive = state;

            state->lifetime.add([keepAlive](){
                std::unique_lock<std::mutex> guard(keepAlive->lock);
                auto expired = std::move(keepAlive->q);
                if (!keepAlive->q.empty()) std::terminate();
                keepAlive->wake.notify_one();

                if (keepAlive->worker.joinable() && keepAlive->worker.get_id() != std::this_thread::get_id()) {
                    guard.unlock();
                    keepAlive->worker.join();
                }
                else {
                    keepAlive->worker.detach();
                }
            });

            state->worker = tf([keepAlive](){

                // take ownership
                queue_type::ensure(std::make_shared<new_worker>(keepAlive));
                // release ownership
                RXCPP_UNWIND_AUTO([]{
                    queue_type::destroy();
                });

                for(;;) {
                    std::unique_lock<std::mutex> guard(keepAlive->lock);
                    if (keepAlive->q.empty()) {
                        keepAlive->wake.wait(guard, [keepAlive](){
                            return !keepAlive->lifetime.is_subscribed() || !keepAlive->q.empty();
                        });
                    }
                    if (!keepAlive->lifetime.is_subscribed()) {
                        break;
                    }
                    auto& peek = keepAlive->q.top();
                    if (!peek.what.is_subscribed()) {
                        keepAlive->q.pop();
                        continue;
                    }
                    if (clock_type::now() < peek.when) {
                        keepAlive->wake.wait_until(guard, peek.when);
                        continue;
                    }
                    auto what = peek.what;
                    keepAlive->q.pop();
                    keepAlive->r.reset(keepAlive->q.empty());
                    guard.unlock();
                    what(keepAlive->r.get_recurse());
                }
            });
        }

        virtual clock_type::time_point now() const {
            return clock_type::now();
        }

        virtual void schedule(const schedulable& scbl) const {
            schedule(now(), scbl);
        }

        virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
            if (scbl.is_subscribed()) {
                std::unique_lock<std::mutex> guard(state->lock);
                state->q.push(new_worker_state::item_type(when, scbl));
                state->r.reset(false);
            }
            state->wake.notify_one();
        }
    };

    mutable thread_factory factory;

public:
    new_thread()
        : factory([](std::function<void()> start){
            return std::thread(std::move(start));
        })
    {
    }
    explicit new_thread(thread_factory tf)
        : factory(tf)
    {
    }
    virtual ~new_thread()
    {
    }

    virtual clock_type::time_point now() const {
        return clock_type::now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        return worker(cs, std::make_shared<new_worker>(cs, factory));
    }
};

inline scheduler make_new_thread() {
    static scheduler instance = make_scheduler<new_thread>();
    return instance;
}
inline scheduler make_new_thread(thread_factory tf) {
    return make_scheduler<new_thread>(tf);
}

}

}

#endif
