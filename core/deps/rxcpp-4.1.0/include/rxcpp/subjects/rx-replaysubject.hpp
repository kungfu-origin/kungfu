// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_REPLAYSUBJECT_HPP)
#define RXCPP_RX_REPLAYSUBJECT_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace subjects {

namespace detail {

template<class Coordination>
struct replay_traits
{
    typedef rxu::maybe<std::size_t> count_type;
    typedef rxu::maybe<rxsc::scheduler::clock_type::duration> period_type;
    typedef rxsc::scheduler::clock_type::time_point time_point_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
};

template<class T, class Coordination>
class replay_observer : public detail::multicast_observer<T>
{
    typedef replay_observer<T, Coordination> this_type;
    typedef detail::multicast_observer<T> base_type;

    typedef replay_traits<Coordination> traits;
    typedef typename traits::count_type count_type;
    typedef typename traits::period_type period_type;
    typedef typename traits::time_point_type time_point_type;
    typedef typename traits::coordination_type coordination_type;
    typedef typename traits::coordinator_type coordinator_type;

    class replay_observer_state : public std::enable_shared_from_this<replay_observer_state>
    {
        mutable std::mutex lock;
        mutable std::list<T> values;
        mutable std::list<time_point_type> time_points;
        mutable count_type count;
        mutable period_type period;
    public:
        mutable coordination_type coordination;
        mutable coordinator_type coordinator;

    private:
        void remove_oldest() const {
            values.pop_front();
            if (!period.empty()) {
                time_points.pop_front();
            }
        }

    public:
        explicit replay_observer_state(count_type _count, period_type _period, coordination_type _coordination, coordinator_type _coordinator)
            : count(_count)
            , period(_period)
            , coordination(std::move(_coordination))
            , coordinator(std::move(_coordinator))
        {
        }

        void add(T v) const {
            std::unique_lock<std::mutex> guard(lock);
            if (!count.empty()) {
                if (values.size() == count.get())
                    remove_oldest();
            }

            if (!period.empty()) {
                auto now = coordination.now();
                while (!time_points.empty() && (now - time_points.front() > period.get()))
                    remove_oldest();
                time_points.push_back(now);
            }

            values.push_back(std::move(v));
        }
        std::list<T> get() const {
            std::unique_lock<std::mutex> guard(lock);
            return values;
        }
    };

    std::shared_ptr<replay_observer_state> state;

public:
    replay_observer(count_type count, period_type period, coordination_type coordination, composite_subscription cs)
        : base_type(cs)
    {
        auto coordinator = coordination.create_coordinator(cs);
        state = std::make_shared<replay_observer_state>(std::move(count), std::move(period), std::move(coordination), std::move(coordinator));
    }

    subscriber<T> get_subscriber() const {
        return make_subscriber<T>(this->get_id(), this->get_subscription(), observer<T, detail::replay_observer<T, Coordination>>(*this)).as_dynamic();
    }

    std::list<T> get_values() const {
        return state->get();
    }

    coordinator_type& get_coordinator() const {
        return state->coordinator;
    }

    template<class V>
    void on_next(V v) const {
        state->add(v);
        base_type::on_next(std::move(v));
    }
};

}

template<class T, class Coordination>
class replay
{
    typedef detail::replay_traits<Coordination> traits;
    typedef typename traits::count_type count_type;
    typedef typename traits::period_type period_type;
    typedef typename traits::time_point_type time_point_type;

    detail::replay_observer<T, Coordination> s;

public:
    explicit replay(Coordination cn, composite_subscription cs = composite_subscription())
        : s(count_type(), period_type(), cn, cs)
    {
    }

    replay(std::size_t count, Coordination cn, composite_subscription cs = composite_subscription())
        : s(count_type(std::move(count)), period_type(), cn, cs)
    {
    }

    replay(rxsc::scheduler::clock_type::duration period, Coordination cn, composite_subscription cs = composite_subscription())
        : s(count_type(), period_type(period), cn, cs)
    {
    }

    replay(std::size_t count, rxsc::scheduler::clock_type::duration period, Coordination cn, composite_subscription cs = composite_subscription())
        : s(count_type(count), period_type(period), cn, cs)
    {
    }

    bool has_observers() const {
        return s.has_observers();
    }

    std::list<T> get_values() const {
        return s.get_values();
    }

    subscriber<T> get_subscriber() const {
        return s.get_subscriber();
    }

    observable<T> get_observable() const {
        auto keepAlive = s;
        auto observable = make_observable_dynamic<T>([=](subscriber<T> o){
            if (keepAlive.get_subscription().is_subscribed()) {
                for (auto&& value: get_values())
                    o.on_next(value);
            }
            keepAlive.add(keepAlive.get_subscriber(), std::move(o));
        });
        return s.get_coordinator().in(observable);
    }
};

}

}

#endif
