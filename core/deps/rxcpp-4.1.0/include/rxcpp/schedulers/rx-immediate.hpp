// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_IMMEDIATE_HPP)
#define RXCPP_RX_SCHEDULER_IMMEDIATE_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

struct immediate : public scheduler_interface
{
private:
    typedef immediate this_type;
    immediate(const this_type&);

    struct immediate_worker : public worker_interface
    {
    private:
        typedef immediate_worker this_type;
        immediate_worker(const this_type&);
    public:
        virtual ~immediate_worker()
        {
        }
        immediate_worker()
        {
        }

        virtual clock_type::time_point now() const {
            return clock_type::now();
        }

        virtual void schedule(const schedulable& scbl) const {
            if (scbl.is_subscribed()) {
                // allow recursion
                recursion r(true);
                scbl(r.get_recurse());
            }
        }

        virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
            std::this_thread::sleep_until(when);
            if (scbl.is_subscribed()) {
                // allow recursion
                recursion r(true);
                scbl(r.get_recurse());
            }
        }
    };

    std::shared_ptr<immediate_worker> wi;

public:
    immediate()
        : wi(std::make_shared<immediate_worker>())
    {
    }
    virtual ~immediate()
    {
    }

    virtual clock_type::time_point now() const {
        return clock_type::now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        return worker(std::move(cs), wi);
    }
};

inline const scheduler& make_immediate() {
    static scheduler instance = make_scheduler<immediate>();
    return instance;
}

}

}

#endif
