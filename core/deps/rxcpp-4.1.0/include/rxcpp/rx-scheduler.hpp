// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_HPP)
#define RXCPP_RX_SCHEDULER_HPP

#include "rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

class worker_interface;
class scheduler_interface;

namespace detail {

class action_type;
typedef std::shared_ptr<action_type> action_ptr;

typedef std::shared_ptr<worker_interface> worker_interface_ptr;
typedef std::shared_ptr<const worker_interface> const_worker_interface_ptr;

typedef std::weak_ptr<worker_interface> worker_interface_weak_ptr;
typedef std::weak_ptr<const worker_interface> const_worker_interface_weak_ptr;

typedef std::shared_ptr<scheduler_interface> scheduler_interface_ptr;
typedef std::shared_ptr<const scheduler_interface> const_scheduler_interface_ptr;

inline action_ptr shared_empty() {
    static action_ptr shared_empty = std::make_shared<detail::action_type>();
    return shared_empty;
}

}

// It is essential to keep virtual function calls out of an inner loop.
// To make tail-recursion work efficiently the recursion objects create
// a space on the stack inside the virtual function call in the actor that
// allows the callback and the scheduler to share stack space that records
// the request and the allowance without any virtual calls in the loop.

/// recursed is set on a schedulable by the action to allow the called
/// function to request to be rescheduled.
class recursed
{
    bool& isrequested;
    recursed operator=(const recursed&);
public:
    explicit recursed(bool& r)
        : isrequested(r)
    {
    }
    /// request to be rescheduled
    inline void operator()() const {
        isrequested = true;
    }
};

/// recurse is passed to the action by the scheduler.
/// the action uses recurse to coordinate the scheduler and the function.
class recurse
{
    bool& isallowed;
    mutable bool isrequested;
    recursed requestor;
    recurse operator=(const recurse&);
public:
    explicit recurse(bool& a)
        : isallowed(a)
        , isrequested(true)
        , requestor(isrequested)
    {
    }
    /// does the scheduler allow tail-recursion now?
    inline bool is_allowed() const {
        return isallowed;
    }
    /// did the function request to be recursed?
    inline bool is_requested() const {
        return isrequested;
    }
    /// reset the function request. call before each call to the function.
    inline void reset() const {
        isrequested = false;
    }
    /// get the recursed to set into the schedulable for the function to use to request recursion
    inline const recursed& get_recursed() const {
        return requestor;
    }
};

/// recursion is used by the scheduler to signal to each action whether tail recursion is allowed.
class recursion
{
    mutable bool isallowed;
    recurse recursor;
    recursion operator=(const recursion&);
public:
    recursion()
        : isallowed(true)
        , recursor(isallowed)
    {
    }
    explicit recursion(bool b)
        : isallowed(b)
        , recursor(isallowed)
    {
    }
    /// set whether tail-recursion is allowed
    inline void reset(bool b = true) const {
        isallowed = b;
    }
    /// get the recurse to pass into each action being called
    inline const recurse& get_recurse() const {
        return recursor;
    }
};


struct action_base
{
    typedef tag_action action_tag;
};

class schedulable;

/// action provides type-forgetting for a potentially recursive set of calls to a function that takes a schedulable
class action : public action_base
{
    typedef action this_type;
    detail::action_ptr inner;
public:
    action()
    {
    }
    explicit action(detail::action_ptr i)
    : inner(std::move(i))
    {
    }

    /// return the empty action
    inline static action empty() {
        return action(detail::shared_empty());
    }

    /// call the function
    inline void operator()(const schedulable& s, const recurse& r) const;
};

struct scheduler_base
{
    typedef std::chrono::steady_clock clock_type;
    typedef tag_scheduler scheduler_tag;
};

struct worker_base : public subscription_base
{
    typedef tag_worker worker_tag;
};

class worker_interface
    : public std::enable_shared_from_this<worker_interface>
{
    typedef worker_interface this_type;

public:
    typedef scheduler_base::clock_type clock_type;

    virtual ~worker_interface() {}

    virtual clock_type::time_point now() const = 0;

    virtual void schedule(const schedulable& scbl) const = 0;
    virtual void schedule(clock_type::time_point when, const schedulable& scbl) const = 0;
};

namespace detail {

template<class F>
struct is_action_function
{
    struct not_void {};
    template<class CF>
    static auto check(int) -> decltype((*(CF*)nullptr)(*(schedulable*)nullptr));
    template<class CF>
    static not_void check(...);

    static const bool value = std::is_same<decltype(check<rxu::decay_t<F>>(0)), void>::value;
};

}

class weak_worker;

/// a worker ensures that all scheduled actions on the same instance are executed in-order with no overlap
/// a worker ensures that all scheduled actions are unsubscribed when it is unsubscribed
/// some inner implementations will impose additional constraints on the execution of items.
class worker : public worker_base
{
    typedef worker this_type;
    detail::worker_interface_ptr inner;
    composite_subscription lifetime;
    friend bool operator==(const worker&, const worker&);
    friend class weak_worker;
public:
    typedef scheduler_base::clock_type clock_type;
    typedef composite_subscription::weak_subscription weak_subscription;

    worker()
    {
    }
    worker(composite_subscription cs, detail::const_worker_interface_ptr i)
        : inner(std::const_pointer_cast<worker_interface>(i))
        , lifetime(std::move(cs))
    {
    }
    worker(composite_subscription cs, worker o)
        : inner(o.inner)
        , lifetime(std::move(cs))
    {
    }

    inline const composite_subscription& get_subscription() const {
        return lifetime;
    }
    inline composite_subscription& get_subscription() {
        return lifetime;
    }

    // composite_subscription
    //
    inline bool is_subscribed() const {
        return lifetime.is_subscribed();
    }
    inline weak_subscription add(subscription s) const {
        return lifetime.add(std::move(s));
    }
    inline void remove(weak_subscription w) const {
        return lifetime.remove(std::move(w));
    }
    inline void clear() const {
        return lifetime.clear();
    }
    inline void unsubscribe() const {
        return lifetime.unsubscribe();
    }

    // worker_interface
    //
    /// return the current time for this worker
    inline clock_type::time_point now() const {
        return inner->now();
    }

    /// insert the supplied schedulable to be run as soon as possible
    inline void schedule(const schedulable& scbl) const {
        // force rebinding scbl to this worker
        schedule_rebind(scbl);
    }

    /// insert the supplied schedulable to be run at the time specified
    inline void schedule(clock_type::time_point when, const schedulable& scbl) const {
        // force rebinding scbl to this worker
        schedule_rebind(when, scbl);
    }

    // helpers
    //

    /// insert the supplied schedulable to be run at now() + the delay specified
    inline void schedule(clock_type::duration when, const schedulable& scbl) const {
        // force rebinding scbl to this worker
        schedule_rebind(now() + when, scbl);
    }

    /// insert the supplied schedulable to be run at the initial time specified and then again at initial + (N * period)
    /// this will continue until the worker or schedulable is unsubscribed.
    inline void schedule_periodically(clock_type::time_point initial, clock_type::duration period, const schedulable& scbl) const {
        // force rebinding scbl to this worker
        schedule_periodically_rebind(initial, period, scbl);
    }

    /// insert the supplied schedulable to be run at now() + the initial delay specified and then again at now() + initial + (N * period)
    /// this will continue until the worker or schedulable is unsubscribed.
    inline void schedule_periodically(clock_type::duration initial, clock_type::duration period, const schedulable& scbl) const {
        // force rebinding scbl to this worker
        schedule_periodically_rebind(now() + initial, period, scbl);
    }

    /// use the supplied arguments to make a schedulable and then insert it to be run
    template<class Arg0, class... ArgN>
    auto schedule(Arg0&& a0, ArgN&&... an) const
        -> typename std::enable_if<
            (detail::is_action_function<Arg0>::value ||
            is_subscription<Arg0>::value) &&
            !is_schedulable<Arg0>::value>::type;
    template<class... ArgN>
    /// use the supplied arguments to make a schedulable and then insert it to be run
    void schedule_rebind(const schedulable& scbl, ArgN&&... an) const;

    /// use the supplied arguments to make a schedulable and then insert it to be run
    template<class Arg0, class... ArgN>
    auto schedule(clock_type::time_point when, Arg0&& a0, ArgN&&... an) const
        -> typename std::enable_if<
            (detail::is_action_function<Arg0>::value ||
            is_subscription<Arg0>::value) &&
            !is_schedulable<Arg0>::value>::type;
    /// use the supplied arguments to make a schedulable and then insert it to be run
    template<class... ArgN>
    void schedule_rebind(clock_type::time_point when, const schedulable& scbl, ArgN&&... an) const;

    /// use the supplied arguments to make a schedulable and then insert it to be run
    template<class Arg0, class... ArgN>
    auto schedule_periodically(clock_type::time_point initial, clock_type::duration period, Arg0&& a0, ArgN&&... an) const
        -> typename std::enable_if<
            (detail::is_action_function<Arg0>::value ||
            is_subscription<Arg0>::value) &&
            !is_schedulable<Arg0>::value>::type;
    /// use the supplied arguments to make a schedulable and then insert it to be run
    template<class... ArgN>
    void schedule_periodically_rebind(clock_type::time_point initial, clock_type::duration period, const schedulable& scbl, ArgN&&... an) const;
};

inline bool operator==(const worker& lhs, const worker& rhs) {
    return lhs.inner == rhs.inner && lhs.lifetime == rhs.lifetime;
}
inline bool operator!=(const worker& lhs, const worker& rhs) {
    return !(lhs == rhs);
}

class weak_worker
{
    detail::worker_interface_weak_ptr inner;
    composite_subscription lifetime;

public:
    weak_worker()
    {
    }
    explicit weak_worker(worker& owner)
        : inner(owner.inner)
        , lifetime(owner.lifetime)
    {
    }

    worker lock() const {
        return worker(lifetime, inner.lock());
    }
};

class scheduler_interface
    : public std::enable_shared_from_this<scheduler_interface>
{
    typedef scheduler_interface this_type;

public:
    typedef scheduler_base::clock_type clock_type;

    virtual ~scheduler_interface() {}

    virtual clock_type::time_point now() const = 0;

    virtual worker create_worker(composite_subscription cs) const = 0;
};


struct schedulable_base :
    // public subscription_base, <- already in worker base
    public worker_base,
    public action_base
{
    typedef tag_schedulable schedulable_tag;
};

/*!
    \brief allows functions to be called at specified times and possibly in other contexts.

    \ingroup group-core

*/
class scheduler : public scheduler_base
{
    typedef scheduler this_type;
    detail::scheduler_interface_ptr inner;
    friend bool operator==(const scheduler&, const scheduler&);
public:
    typedef scheduler_base::clock_type clock_type;

    scheduler()
    {
    }
    explicit scheduler(detail::scheduler_interface_ptr i)
        : inner(std::move(i))
    {
    }
    explicit scheduler(detail::const_scheduler_interface_ptr i)
        : inner(std::const_pointer_cast<scheduler_interface>(i))
    {
    }

    /// return the current time for this scheduler
    inline clock_type::time_point now() const {
        return inner->now();
    }
    /// create a worker with a lifetime.
    /// when the worker is unsubscribed all scheduled items will be unsubscribed.
    /// items scheduled to a worker will be run one at a time.
    /// scheduling order is preserved: when more than one item is scheduled for
    /// time T then at time T they will be run in the order that they were scheduled.
    inline worker create_worker(composite_subscription cs = composite_subscription()) const {
        return inner->create_worker(cs);
    }
};

template<class Scheduler, class... ArgN>
inline scheduler make_scheduler(ArgN&&... an) {
    return scheduler(std::static_pointer_cast<scheduler_interface>(std::make_shared<Scheduler>(std::forward<ArgN>(an)...)));
}

inline scheduler make_scheduler(std::shared_ptr<scheduler_interface> si) {
    return scheduler(si);
}

class schedulable : public schedulable_base
{
    typedef schedulable this_type;

    composite_subscription lifetime;
    weak_worker controller;
    action activity;
    bool scoped;
    composite_subscription::weak_subscription action_scope;

    struct detacher
    {
        ~detacher()
        {
            if (that) {
                that->unsubscribe();
            }
        }
        detacher(const this_type* that)
            : that(that)
        {
        }
        const this_type* that;
    };

    class recursed_scope_type
    {
        mutable const recursed* requestor;

        class exit_recursed_scope_type
        {
            const recursed_scope_type* that;
        public:
            ~exit_recursed_scope_type()
            {
                if (that != nullptr) {
                    that->requestor = nullptr;
                }
            }
            exit_recursed_scope_type(const recursed_scope_type* that)
                : that(that)
            {
            }
            exit_recursed_scope_type(exit_recursed_scope_type && other) RXCPP_NOEXCEPT
                : that(other.that)
            {
                other.that = nullptr;
            }
        };
    public:
        recursed_scope_type()
            : requestor(nullptr)
        {
        }
        recursed_scope_type(const recursed_scope_type&)
            : requestor(nullptr)
        {
            // does not aquire recursion scope
        }
        recursed_scope_type& operator=(const recursed_scope_type& )
        {
            // no change in recursion scope
            return *this;
        }
        exit_recursed_scope_type reset(const recurse& r) const {
            requestor = std::addressof(r.get_recursed());
            return exit_recursed_scope_type(this);
        }
        bool is_recursed() const {
            return !!requestor;
        }
        void operator()() const {
            (*requestor)();
        }
    };
    recursed_scope_type recursed_scope;

public:
    typedef composite_subscription::weak_subscription weak_subscription;
    typedef scheduler_base::clock_type clock_type;

    ~schedulable()
    {
        if (scoped) {
            controller.lock().remove(action_scope);
        }
    }
    schedulable()
        : scoped(false)
    {
    }

    /// action and worker share lifetime
    schedulable(worker q, action a)
        : lifetime(q.get_subscription())
        , controller(q)
        , activity(std::move(a))
        , scoped(false)
    {
    }
    /// action and worker have independent lifetimes
    schedulable(composite_subscription cs, worker q, action a)
        : lifetime(std::move(cs))
        , controller(q)
        , activity(std::move(a))
        , scoped(true)
        , action_scope(controller.lock().add(lifetime))
    {
    }
    /// inherit lifetimes
    schedulable(schedulable scbl, worker q, action a)
        : lifetime(scbl.get_subscription())
        , controller(q)
        , activity(std::move(a))
        , scoped(scbl.scoped)
        , action_scope(scbl.scoped ? controller.lock().add(lifetime) : weak_subscription())
    {
    }

    inline const composite_subscription& get_subscription() const {
        return lifetime;
    }
    inline composite_subscription& get_subscription() {
        return lifetime;
    }
    inline const worker get_worker() const {
        return controller.lock();
    }
    inline worker get_worker() {
        return controller.lock();
    }
    inline const action& get_action() const {
        return activity;
    }
    inline action& get_action() {
        return activity;
    }

    inline static schedulable empty(worker sc) {
        return schedulable(composite_subscription::empty(), sc, action::empty());
    }

    inline auto set_recursed(const recurse& r) const
        -> decltype(recursed_scope.reset(r)) {
        return      recursed_scope.reset(r);
    }

    // recursed
    //
    bool is_recursed() const {
        return recursed_scope.is_recursed();
    }
    /// requests tail-recursion of the same action
    /// this will exit the process if called when
    /// is_recursed() is false.
    /// Note: to improve perf it is not required
    /// to call is_recursed() before calling this
    /// operator. Context is sufficient. The schedulable
    /// passed to the action by the scheduler will return
    /// true from is_recursed()
    inline void operator()() const {
        recursed_scope();
    }

    // composite_subscription
    //
    inline bool is_subscribed() const {
        return lifetime.is_subscribed();
    }
    inline weak_subscription add(subscription s) const {
        return lifetime.add(std::move(s));
    }
    template<class F>
    auto add(F f) const
    -> typename std::enable_if<rxcpp::detail::is_unsubscribe_function<F>::value, weak_subscription>::type {
        return lifetime.add(make_subscription(std::move(f)));
    }
    inline void remove(weak_subscription w) const {
        return lifetime.remove(std::move(w));
    }
    inline void clear() const {
        return lifetime.clear();
    }
    inline void unsubscribe() const {
        return lifetime.unsubscribe();
    }

    // scheduler
    //
    inline clock_type::time_point now() const {
        return controller.lock().now();
    }
    /// put this on the queue of the stored scheduler to run asap
    inline void schedule() const {
        if (is_subscribed()) {
            get_worker().schedule(*this);
        }
    }
    /// put this on the queue of the stored scheduler to run at the specified time
    inline void schedule(clock_type::time_point when) const {
        if (is_subscribed()) {
            get_worker().schedule(when, *this);
        }
    }
    /// put this on the queue of the stored scheduler to run after a delay from now
    inline void schedule(clock_type::duration when) const {
        if (is_subscribed()) {
            get_worker().schedule(when, *this);
        }
    }

    // action
    //
    /// invokes the action
    inline void operator()(const recurse& r) const {
        if (!is_subscribed()) {
            return;
        }
        detacher protect(this);
        activity(*this, r);
        protect.that = nullptr;
    }
};

struct current_thread;

namespace detail {

class action_type
    : public std::enable_shared_from_this<action_type>
{
    typedef action_type this_type;

public:
    typedef std::function<void(const schedulable&, const recurse&)> function_type;

private:
    function_type f;

public:
    action_type()
    {
    }

    action_type(function_type f)
        : f(std::move(f))
    {
    }

    inline void operator()(const schedulable& s, const recurse& r) {
        if (!f) {
            std::terminate();
        }
        f(s, r);
    }
};

class action_tailrecurser
    : public std::enable_shared_from_this<action_type>
{
    typedef action_type this_type;

public:
    typedef std::function<void(const schedulable&)> function_type;

private:
    function_type f;

public:
    action_tailrecurser()
    {
    }

    action_tailrecurser(function_type f)
        : f(std::move(f))
    {
    }

    inline void operator()(const schedulable& s, const recurse& r) {
        if (!f) {
            std::terminate();
        }
        trace_activity().action_enter(s);
        auto scope = s.set_recursed(r);
        while (s.is_subscribed()) {
            r.reset();
            f(s);
            if (!r.is_allowed() || !r.is_requested()) {
                if (r.is_requested()) {
                    s.schedule();
                }
                break;
            }
            trace_activity().action_recurse(s);
        }
        trace_activity().action_return(s);
    }
};
}

inline void action::operator()(const schedulable& s, const recurse& r) const {
    (*inner)(s, r);
}

inline action make_action_empty() {
    return action::empty();
}

template<class F>
inline action make_action(F&& f) {
    static_assert(detail::is_action_function<F>::value, "action function must be void(schedulable)");
    auto fn = std::forward<F>(f);
    return action(std::make_shared<detail::action_type>(detail::action_tailrecurser(fn)));
}

// copy
inline auto make_schedulable(
    const   schedulable& scbl)
    ->      schedulable {
    return  schedulable(scbl);
}
// move
inline auto make_schedulable(
            schedulable&& scbl)
    ->      schedulable {
    return  schedulable(std::move(scbl));
}

inline schedulable make_schedulable(worker sc, action a) {
    return schedulable(sc, a);
}
inline schedulable make_schedulable(worker sc, composite_subscription cs, action a) {
    return schedulable(cs, sc, a);
}

template<class F>
auto make_schedulable(worker sc, F&& f)
    -> typename std::enable_if<detail::is_action_function<F>::value, schedulable>::type {
    return schedulable(sc, make_action(std::forward<F>(f)));
}
template<class F>
auto make_schedulable(worker sc, composite_subscription cs, F&& f)
    -> typename std::enable_if<detail::is_action_function<F>::value, schedulable>::type {
    return schedulable(cs, sc, make_action(std::forward<F>(f)));
}
template<class F>
auto make_schedulable(schedulable scbl, composite_subscription cs, F&& f)
    -> typename std::enable_if<detail::is_action_function<F>::value, schedulable>::type {
    return schedulable(cs, scbl.get_worker(), make_action(std::forward<F>(f)));
}
template<class F>
auto make_schedulable(schedulable scbl, worker sc, F&& f)
    -> typename std::enable_if<detail::is_action_function<F>::value, schedulable>::type {
    return schedulable(scbl, sc, make_action(std::forward<F>(f)));
}
template<class F>
auto make_schedulable(schedulable scbl, F&& f)
    -> typename std::enable_if<detail::is_action_function<F>::value, schedulable>::type {
    return schedulable(scbl, scbl.get_worker(), make_action(std::forward<F>(f)));
}

inline auto make_schedulable(schedulable scbl, composite_subscription cs)
    -> schedulable {
    return schedulable(cs, scbl.get_worker(), scbl.get_action());
}
inline auto make_schedulable(schedulable scbl, worker sc, composite_subscription cs)
    -> schedulable {
    return schedulable(cs, sc, scbl.get_action());
}
inline auto make_schedulable(schedulable scbl, worker sc)
    -> schedulable {
    return schedulable(scbl, sc, scbl.get_action());
}

template<class Arg0, class... ArgN>
auto worker::schedule(Arg0&& a0, ArgN&&... an) const
    -> typename std::enable_if<
        (detail::is_action_function<Arg0>::value ||
        is_subscription<Arg0>::value) &&
        !is_schedulable<Arg0>::value>::type {
    auto scbl = make_schedulable(*this, std::forward<Arg0>(a0), std::forward<ArgN>(an)...);
    trace_activity().schedule_enter(*inner.get(), scbl);
    inner->schedule(std::move(scbl));
    trace_activity().schedule_return(*inner.get());
}
template<class... ArgN>
void worker::schedule_rebind(const schedulable& scbl, ArgN&&... an) const {
    auto rescbl = make_schedulable(scbl, *this, std::forward<ArgN>(an)...);
    trace_activity().schedule_enter(*inner.get(), rescbl);
    inner->schedule(std::move(rescbl));
    trace_activity().schedule_return(*inner.get());
}

template<class Arg0, class... ArgN>
auto worker::schedule(clock_type::time_point when, Arg0&& a0, ArgN&&... an) const
    -> typename std::enable_if<
        (detail::is_action_function<Arg0>::value ||
        is_subscription<Arg0>::value) &&
        !is_schedulable<Arg0>::value>::type {
    auto scbl = make_schedulable(*this, std::forward<Arg0>(a0), std::forward<ArgN>(an)...);
    trace_activity().schedule_when_enter(*inner.get(), when, scbl);
    inner->schedule(when, std::move(scbl));
    trace_activity().schedule_when_return(*inner.get());
}
template<class... ArgN>
void worker::schedule_rebind(clock_type::time_point when, const schedulable& scbl, ArgN&&... an) const {
    auto rescbl = make_schedulable(scbl, *this, std::forward<ArgN>(an)...);
    trace_activity().schedule_when_enter(*inner.get(), when, rescbl);
    inner->schedule(when, std::move(rescbl));
    trace_activity().schedule_when_return(*inner.get());
}

template<class Arg0, class... ArgN>
auto worker::schedule_periodically(clock_type::time_point initial, clock_type::duration period, Arg0&& a0, ArgN&&... an) const
    -> typename std::enable_if<
        (detail::is_action_function<Arg0>::value ||
        is_subscription<Arg0>::value) &&
        !is_schedulable<Arg0>::value>::type {
    schedule_periodically_rebind(initial, period, make_schedulable(*this, std::forward<Arg0>(a0), std::forward<ArgN>(an)...));
}
template<class... ArgN>
void worker::schedule_periodically_rebind(clock_type::time_point initial, clock_type::duration period, const schedulable& scbl, ArgN&&... an) const {
    auto keepAlive = *this;
    auto target = std::make_shared<clock_type::time_point>(initial);
    auto activity = make_schedulable(scbl, keepAlive, std::forward<ArgN>(an)...);
    auto periodic = make_schedulable(
        activity,
        [keepAlive, target, period, activity](schedulable self) {
            // any recursion requests will be pushed to the scheduler queue
            recursion r(false);
            // call action
            activity(r.get_recurse());

            // schedule next occurance (if the action took longer than 'period' target will be in the past)
            *target += period;
            self.schedule(*target);
        });
    trace_activity().schedule_when_enter(*inner.get(), *target, periodic);
    inner->schedule(*target, periodic);
    trace_activity().schedule_when_return(*inner.get());
}

namespace detail {

template<class TimePoint>
struct time_schedulable
{
    typedef TimePoint time_point_type;

    time_schedulable(TimePoint when, schedulable a)
        : when(when)
        , what(std::move(a))
    {
    }
    TimePoint when;
    schedulable what;
};


// Sorts time_schedulable items in priority order sorted
// on value of time_schedulable.when. Items with equal
// values for when are sorted in fifo order.
template<class TimePoint>
class schedulable_queue {
public:
    typedef time_schedulable<TimePoint> item_type;
    typedef std::pair<item_type, int64_t> elem_type;
    typedef std::vector<elem_type> container_type;
    typedef const item_type& const_reference;

private:
    struct compare_elem
    {
        bool operator()(const elem_type& lhs, const elem_type& rhs) const {
            if (lhs.first.when == rhs.first.when) {
                return lhs.second > rhs.second;
            }
            else {
                return lhs.first.when > rhs.first.when;
            }
        }
    };

    typedef std::priority_queue<
        elem_type,
        container_type,
        compare_elem
    > queue_type;

    queue_type q;

    int64_t ordinal;
public:

    schedulable_queue() 
        : ordinal(0) 
    {
    }

    const_reference top() const {
        return q.top().first;
    }

    void pop() {
        q.pop();
    }

    bool empty() const {
        return q.empty();
    }

    void push(const item_type& value) {
        q.push(elem_type(value, ordinal++));
    }

    void push(item_type&& value) {
        q.push(elem_type(std::move(value), ordinal++));
    }
};

}

}
namespace rxsc=schedulers;

}

#include "schedulers/rx-currentthread.hpp"
#include "schedulers/rx-runloop.hpp"
#include "schedulers/rx-newthread.hpp"
#include "schedulers/rx-eventloop.hpp"
#include "schedulers/rx-immediate.hpp"
#include "schedulers/rx-virtualtime.hpp"
#include "schedulers/rx-sameworker.hpp"

#endif
