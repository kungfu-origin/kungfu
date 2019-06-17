// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SUBSCRIBER_HPP)
#define RXCPP_RX_SUBSCRIBER_HPP

#include "rx-includes.hpp"

namespace rxcpp {

template<class T>
struct subscriber_base : public observer_base<T>, public subscription_base
{
    typedef tag_subscriber subscriber_tag;
};

/*!
    \brief binds an observer that consumes values with a composite_subscription that controls lifetime.

    \ingroup group-core

*/
template<class T, class Observer = observer<T>>
class subscriber : public subscriber_base<T>
{
    static_assert(!is_subscriber<Observer>::value, "not allowed to nest subscribers");
    static_assert(is_observer<Observer>::value, "subscriber must contain an observer<T, ...>");
    typedef subscriber<T, Observer> this_type;
    typedef rxu::decay_t<Observer> observer_type;

    composite_subscription lifetime;
    observer_type destination;
    trace_id id;

    struct nextdetacher
    {
        ~nextdetacher()
        {
            trace_activity().on_next_return(*that);
            if (do_unsubscribe) {
                that->unsubscribe();
            }
        }
        nextdetacher(const this_type* that)
            : that(that)
            , do_unsubscribe(true)
        {
        }
        template<class U>
        void operator()(U u) {
            trace_activity().on_next_enter(*that, u);
            try {
                that->destination.on_next(std::move(u));
                do_unsubscribe = false;
            } catch(...) {
                auto ex = std::current_exception();
                trace_activity().on_error_enter(*that, ex);
                that->destination.on_error(std::move(ex));
                trace_activity().on_error_return(*that);
            }
        }
        const this_type* that;
        volatile bool do_unsubscribe;
    };

    struct errordetacher
    {
        ~errordetacher()
        {
            trace_activity().on_error_return(*that);
            that->unsubscribe();
        }
        errordetacher(const this_type* that)
            : that(that)
        {
        }
        inline void operator()(std::exception_ptr ex) {
            trace_activity().on_error_enter(*that, ex);
            that->destination.on_error(std::move(ex));
        }
        const this_type* that;
    };

    struct completeddetacher
    {
        ~completeddetacher()
        {
            trace_activity().on_completed_return(*that);
            that->unsubscribe();
        }
        completeddetacher(const this_type* that)
            : that(that)
        {
        }
        inline void operator()() {
            trace_activity().on_completed_enter(*that);
            that->destination.on_completed();
        }
        const this_type* that;
    };

    subscriber();
public:
    typedef typename composite_subscription::weak_subscription weak_subscription;

    subscriber(const this_type& o)
        : lifetime(o.lifetime)
        , destination(o.destination)
        , id(o.id)
    {
    }
    subscriber(this_type&& o)
        : lifetime(std::move(o.lifetime))
        , destination(std::move(o.destination))
        , id(std::move(o.id))
    {
    }

    template<class U, class O>
    friend class subscriber;

    template<class O>
    subscriber(
        const subscriber<T, O>& o,
        typename std::enable_if<
               !std::is_same<O, observer<T>>::value &&
               std::is_same<Observer, observer<T>>::value, void**>::type = nullptr)
        : lifetime(o.lifetime)
        , destination(o.destination.as_dynamic())
        , id(o.id)
    {
    }

    template<class U>
    subscriber(trace_id id, composite_subscription cs, U&& o)
        : lifetime(std::move(cs))
        , destination(std::forward<U>(o))
        , id(std::move(id))
    {
        static_assert(!is_subscriber<U>::value, "cannot nest subscribers");
        static_assert(is_observer<U>::value, "must pass observer to subscriber");
        trace_activity().create_subscriber(*this);
    }

    this_type& operator=(this_type o) {
        lifetime = std::move(o.lifetime);
        destination = std::move(o.destination);
        id = std::move(o.id);
        return *this;
    }

    const observer_type& get_observer() const {
        return destination;
    }
    observer_type& get_observer() {
        return destination;
    }
    const composite_subscription& get_subscription() const {
        return lifetime;
    }
    composite_subscription& get_subscription() {
        return lifetime;
    }
    trace_id get_id() const {
        return id;
    }

    subscriber<T> as_dynamic() const {
        return subscriber<T>(id, lifetime, destination.as_dynamic());
    }

    // observer
    //
    template<class V>
    void on_next(V&& v) const {
        if (!is_subscribed()) {
            return;
        }
        nextdetacher protect(this);
        protect(std::forward<V>(v));
    }
    void on_error(std::exception_ptr e) const {
        if (!is_subscribed()) {
            return;
        }
        errordetacher protect(this);
        protect(std::move(e));
    }
    void on_completed() const {
        if (!is_subscribed()) {
            return;
        }
        completeddetacher protect(this);
        protect();
    }

    // composite_subscription
    //
    bool is_subscribed() const {
        return lifetime.is_subscribed();
    }
    weak_subscription add(subscription s) const {
        return lifetime.add(std::move(s));
    }
    template<class F>
    auto add(F f) const
    -> typename std::enable_if<detail::is_unsubscribe_function<F>::value, weak_subscription>::type {
        return lifetime.add(make_subscription(std::move(f)));
    }
    void remove(weak_subscription w) const {
        return lifetime.remove(std::move(w));
    }
    void clear() const {
        return lifetime.clear();
    }
    void unsubscribe() const {
        return lifetime.unsubscribe();
    }

};

template<class T, class Observer>
auto make_subscriber(
            subscriber<T,   Observer> o)
    ->      subscriber<T,   Observer> {
    return  subscriber<T,   Observer>(std::move(o));
}

// observer
//

template<class T>
auto make_subscriber()
    -> typename std::enable_if<
        detail::is_on_next_of<T, detail::OnNextEmpty<T>>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>>(trace_id::make_next_id_subscriber(), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>(detail::OnNextEmpty<T>()));
}

template<class T, class I>
auto make_subscriber(
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(trace_id::make_next_id_subscriber(), composite_subscription(), o);
}
template<class T, class Observer>
auto make_subscriber(const Observer& o)
    -> typename std::enable_if<
    is_observer<Observer>::value &&
    !is_subscriber<Observer>::value,
            subscriber<T,   Observer>>::type {
    return  subscriber<T,   Observer>(trace_id::make_next_id_subscriber(), composite_subscription(), o);
}
template<class T, class Observer>
auto make_subscriber(const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
            subscriber<T,   observer<T, Observer>>>::type {
    return  subscriber<T,   observer<T, Observer>>(trace_id::make_next_id_subscriber(), composite_subscription(), o);
}
template<class T, class OnNext>
auto make_subscriber(const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(trace_id::make_next_id_subscriber(), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext>(on));
}
template<class T, class OnNext, class OnError>
auto make_subscriber(const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(trace_id::make_next_id_subscriber(), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
}
template<class T, class OnNext, class OnCompleted>
auto make_subscriber(const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(trace_id::make_next_id_subscriber(), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
}
template<class T, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(trace_id::make_next_id_subscriber(), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
}

// explicit lifetime
//

template<class T>
auto make_subscriber(const composite_subscription& cs)
    ->      subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>> {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>>(trace_id::make_next_id_subscriber(), cs,
                            observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>(detail::OnNextEmpty<T>()));
}

template<class T, class I>
auto make_subscriber(const composite_subscription& cs,
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(trace_id::make_next_id_subscriber(), cs, o);
}
template<class T, class I>
auto make_subscriber(const composite_subscription& cs,
    const                   subscriber<T, I>& s)
    ->      subscriber<T,   I> {
    return  subscriber<T,   I>(trace_id::make_next_id_subscriber(), cs, s.get_observer());
}
template<class T, class Observer>
auto make_subscriber(const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        !is_subscriber<Observer>::value &&
        is_observer<Observer>::value,
            subscriber<T,   Observer>>::type {
    return  subscriber<T,   Observer>(trace_id::make_next_id_subscriber(), cs, o);
}
template<class T, class Observer>
auto make_subscriber(const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
            subscriber<T,   observer<T, Observer>>>::type {
    return  subscriber<T,   observer<T, Observer>>(trace_id::make_next_id_subscriber(), cs, make_observer<T>(o));
}
template<class T, class OnNext>
auto make_subscriber(const composite_subscription& cs, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(trace_id::make_next_id_subscriber(), cs,
                            observer<T, detail::stateless_observer_tag, OnNext>(on));
}
template<class T, class OnNext, class OnError>
auto make_subscriber(const composite_subscription& cs, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(trace_id::make_next_id_subscriber(), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
}
template<class T, class OnNext, class OnCompleted>
auto make_subscriber(const composite_subscription& cs, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(trace_id::make_next_id_subscriber(), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
}
template<class T, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const composite_subscription& cs, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(trace_id::make_next_id_subscriber(), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
}

// explicit id
//

template<class T>
auto make_subscriber(trace_id id)
    ->      subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>> {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>>(std::move(id), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>(detail::OnNextEmpty<T>()));
}

template<class T>
auto make_subscriber(trace_id id, const composite_subscription& cs)
    ->      subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>> {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>>(std::move(id), cs,
                            observer<T, detail::stateless_observer_tag, detail::OnNextEmpty<T>>(detail::OnNextEmpty<T>()));
}

template<class T, class I>
auto make_subscriber(trace_id id,
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(std::move(id), composite_subscription(), o);
}
template<class T, class I>
auto make_subscriber(trace_id id, const composite_subscription& cs,
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(std::move(id), cs, o);
}
template<class T, class Observer>
auto make_subscriber(trace_id id, const Observer& o)
    -> typename std::enable_if<
        is_observer<Observer>::value,
            subscriber<T,   Observer>>::type {
    return  subscriber<T,   Observer>(std::move(id), composite_subscription(), o);
}
template<class T, class Observer>
auto make_subscriber(trace_id id, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        is_observer<Observer>::value,
            subscriber<T,   Observer>>::type {
    return  subscriber<T,   Observer>(std::move(id), cs, o);
}
template<class T, class Observer>
auto make_subscriber(trace_id id, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
            subscriber<T,   observer<T, Observer>>>::type {
    return  subscriber<T,   observer<T, Observer>>(std::move(id), composite_subscription(), o);
}
template<class T, class Observer>
auto make_subscriber(trace_id id, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
            subscriber<T,   observer<T, Observer>>>::type {
    return  subscriber<T,   observer<T, Observer>>(std::move(id), cs, o);
}
template<class T, class OnNext>
auto make_subscriber(trace_id id, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(std::move(id), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext>(on));
}
template<class T, class OnNext>
auto make_subscriber(trace_id id, const composite_subscription& cs, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(std::move(id), cs,
                            observer<T, detail::stateless_observer_tag, OnNext>(on));
}
template<class T, class OnNext, class OnError>
auto make_subscriber(trace_id id, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(std::move(id), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
}
template<class T, class OnNext, class OnError>
auto make_subscriber(trace_id id, const composite_subscription& cs, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(std::move(id), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
}
template<class T, class OnNext, class OnCompleted>
auto make_subscriber(trace_id id, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(std::move(id), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
}
template<class T, class OnNext, class OnCompleted>
auto make_subscriber(trace_id id, const composite_subscription& cs, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(std::move(id), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
}
template<class T, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(trace_id id, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(std::move(id), composite_subscription(),
                            observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
}
template<class T, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(trace_id id, const composite_subscription& cs, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
            subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    return  subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(std::move(id), cs,
                            observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
}

// chain defaults from subscriber
//

template<class T, class OtherT, class OtherObserver, class I>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr,
    const                   observer<T, I>& o)
    ->       subscriber<T,   observer<T, I>> {
    auto r = subscriber<T,   observer<T, I>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(), o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class I>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id,
    const                   observer<T, I>& o)
    ->       subscriber<T,   observer<T, I>> {
    auto r = subscriber<T,   observer<T, I>>(std::move(id), scbr.get_subscription(), o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const Observer& o)
    -> typename std::enable_if<
        is_observer<Observer>::value,
             subscriber<T,   Observer>>::type {
    auto r = subscriber<T,   Observer>(std::move(id), scbr.get_subscription(), o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const Observer& o)
    -> typename std::enable_if<
        !is_subscription<Observer>::value &&
        is_observer<Observer>::value,
             subscriber<T,   Observer>>::type {
    auto r = subscriber<T,   Observer>(trace_id::make_next_id_subscriber(), scbr.get_subscription(), o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
             subscriber<T,   observer<T, Observer>>>::type {
    auto r = subscriber<T,   observer<T, Observer>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(), make_observer<T>(o));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
             subscriber<T,   observer<T, Observer>>>::type {
    auto r = subscriber<T,   observer<T, Observer>>(std::move(id), scbr.get_subscription(), o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext>(on));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(std::move(id), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext>(on));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(std::move(id), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(std::move(id), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(trace_id::make_next_id_subscriber(), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(std::move(id), scbr.get_subscription(),
                             observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
    trace_activity().connect(r, scbr);
    return r;
}

template<class T, class OtherT, class OtherObserver, class I>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& , const composite_subscription& cs,
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(trace_id::make_next_id_subscriber(), cs, o);
}
template<class T, class OtherT, class OtherObserver, class I>
auto make_subscriber(const subscriber<OtherT, OtherObserver>&, trace_id id, const composite_subscription& cs,
    const                   observer<T, I>& o)
    ->      subscriber<T,   observer<T, I>> {
    return  subscriber<T,   observer<T, I>>(std::move(id), cs, o);
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        is_observer<Observer>::value,
             subscriber<T,   Observer>>::type {
    auto r = subscriber<T,   Observer>(trace_id::make_next_id_subscriber(), cs, o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        is_observer<Observer>::value,
             subscriber<T,   Observer>>::type {
    auto r = subscriber<T,   Observer>(std::move(id), cs, o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
             subscriber<T,   observer<T, Observer>>>::type {
    auto r = subscriber<T,   observer<T, Observer>>(trace_id::make_next_id_subscriber(), cs, o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class Observer>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const Observer& o)
    -> typename std::enable_if<
        !detail::is_on_next_of<T, Observer>::value &&
        !is_subscriber<Observer>::value &&
        !is_subscription<Observer>::value &&
        !is_observer<Observer>::value,
             subscriber<T,   observer<T, Observer>>>::type {
    auto r = subscriber<T,   observer<T, Observer>>(std::move(id), cs, o);
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(trace_id::make_next_id_subscriber(), cs,
                             observer<T, detail::stateless_observer_tag, OnNext>(on));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const OnNext& on)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext>>(std::move(id), cs,
                             observer<T, detail::stateless_observer_tag, OnNext>(on));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(trace_id::make_next_id_subscriber(), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const OnNext& on, const OnError& oe)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError>>(std::move(id), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, OnError>(on, oe));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(trace_id::make_next_id_subscriber(), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const OnNext& on, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>>(std::move(id), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, detail::OnErrorEmpty, OnCompleted>(on, detail::OnErrorEmpty(), oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, const composite_subscription& cs, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(trace_id::make_next_id_subscriber(), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class OtherT, class OtherObserver, class OnNext, class OnError, class OnCompleted>
auto make_subscriber(const subscriber<OtherT, OtherObserver>& scbr, trace_id id, const composite_subscription& cs, const OnNext& on, const OnError& oe, const OnCompleted& oc)
    -> typename std::enable_if<
        detail::is_on_next_of<T, OnNext>::value &&
        detail::is_on_error<OnError>::value &&
        detail::is_on_completed<OnCompleted>::value,
             subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>>::type {
    auto r = subscriber<T,   observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>>(std::move(id), cs,
                             observer<T, detail::stateless_observer_tag, OnNext, OnError, OnCompleted>(on, oe, oc));
    trace_activity().connect(r, scbr);
    return r;
}

template<class T, class Observer>
auto make_subscriber(const subscriber<T, Observer>& scbr, const composite_subscription& cs)
    ->      subscriber<T,   Observer> {
    auto r = subscriber<T,   Observer>(scbr.get_id(), cs, scbr.get_observer());
    trace_activity().connect(r, scbr);
    return r;
}
template<class T, class Observer>
auto make_subscriber(const subscriber<T, Observer>& scbr, trace_id id, const composite_subscription& cs)
    ->      subscriber<T,   Observer> {
    auto r = subscriber<T,   Observer>(std::move(id), cs, scbr.get_observer());
    trace_activity().connect(r, scbr);
    return r;
}

template<class T, class Observer>
auto make_subscriber(const subscriber<T, Observer>& scbr, trace_id id)
    ->      subscriber<T,   Observer> {
    auto r = subscriber<T,   Observer>(std::move(id), scbr.get_subscription(), scbr.get_observer());
    trace_activity().connect(r, scbr);
    return r;
}

}

#endif
