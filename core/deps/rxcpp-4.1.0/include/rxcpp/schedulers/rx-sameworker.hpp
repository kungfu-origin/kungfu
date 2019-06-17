// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_SAME_WORKER_HPP)
#define RXCPP_RX_SCHEDULER_SAME_WORKER_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

struct same_worker : public scheduler_interface
{
private:
    typedef same_worker this_type;
    same_worker(const this_type&);

    rxsc::worker controller;

public:
    explicit same_worker(rxsc::worker w)
        : controller(std::move(w))
    {
    }
    virtual ~same_worker()
    {
    }

    virtual clock_type::time_point now() const {
        return controller.now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        // use different lifetime
        auto inner_lifetime = controller.get_subscription();
        auto token = inner_lifetime.add(cs);
        cs.add([inner_lifetime, token](){inner_lifetime.remove(token);});
        return worker(cs, controller);
    }
};

inline scheduler make_same_worker(rxsc::worker w) {
    return make_scheduler<same_worker>(std::move(w));
}

}

}

#endif
