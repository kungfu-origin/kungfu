// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_TEST_HPP)
#define RXCPP_RX_SCHEDULER_TEST_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace schedulers {

namespace detail {

class test_type : public scheduler_interface
{
public:

    typedef scheduler_interface::clock_type clock_type;

    struct test_type_state : public virtual_time<long, long>
    {
        typedef virtual_time<long, long> base;

        using base::schedule_absolute;
        using base::schedule_relative;

        clock_type::time_point now() const {
            return to_time_point(clock_now);
        }

        virtual void schedule_absolute(long when, const schedulable& a) const
        {
            if (when <= base::clock_now)
                when = base::clock_now + 1;

            return base::schedule_absolute(when, a);
        }

        virtual long add(long absolute, long relative) const
        {
            return absolute + relative;
        }

        virtual clock_type::time_point to_time_point(long absolute) const
        {
            return clock_type::time_point(std::chrono::milliseconds(absolute));
        }

        virtual long to_relative(clock_type::duration d) const
        {
            return static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(d).count());
        }
    };

private:
    mutable std::shared_ptr<test_type_state> state;

public:
    struct test_type_worker : public worker_interface
    {
        mutable std::shared_ptr<test_type_state> state;

        typedef test_type_state::absolute absolute;
        typedef test_type_state::relative relative;

        test_type_worker(std::shared_ptr<test_type_state> st)
            : state(std::move(st))
        {
        }

        virtual clock_type::time_point now() const {
            return state->now();
        }

        virtual void schedule(const schedulable& scbl) const {
            state->schedule_absolute(state->clock(), scbl);
        }

        virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
            state->schedule_relative(state->to_relative(when - now()), scbl);
        }

        void schedule_absolute(absolute when, const schedulable& scbl) const {
            state->schedule_absolute(when, scbl);
        }

        void schedule_relative(relative when, const schedulable& scbl) const {
            state->schedule_relative(when, scbl);
        }

        bool is_enabled() const {return state->is_enabled();}
        absolute clock() const {return state->clock();}

        void start() const
        {
            state->start();
        }

        void stop() const
        {
            state->stop();
        }

        void advance_to(absolute time) const
        {
            state->advance_to(time);
        }

        void advance_by(relative time) const
        {
            state->advance_by(time);
        }

        void sleep(relative time) const
        {
            state->sleep(time);
        }

        template<class T>
        subscriber<T, rxt::testable_observer<T>> make_subscriber() const;
    };

public:
    test_type()
        : state(std::make_shared<test_type_state>())
    {
    }

    virtual clock_type::time_point now() const {
        return state->now();
    }

    virtual worker create_worker(composite_subscription cs) const {
        return worker(cs, std::make_shared<test_type_worker>(state));
    }

    bool is_enabled() const {return state->is_enabled();}
    long clock() {
        return state->clock();
    }

    clock_type::time_point to_time_point(long absolute) const {
        return state->to_time_point(absolute);
    }

    std::shared_ptr<test_type_worker> create_test_type_worker_interface() const {
        return std::make_shared<test_type_worker>(state);
    }

    template<class T>
    rxt::testable_observable<T> make_hot_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const;

    template<class T>
    rxt::testable_observable<T> make_cold_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const;
};

template<class T>
class mock_observer
    : public rxt::detail::test_subject_base<T>
{
    typedef typename rxn::notification<T> notification_type;
    typedef rxn::recorded<typename notification_type::type> recorded_type;

public:
    explicit mock_observer(std::shared_ptr<test_type::test_type_state> sc)
        : sc(sc)
    {
    }

    std::shared_ptr<test_type::test_type_state> sc;
    std::vector<recorded_type> m;

    virtual void on_subscribe(subscriber<T>) const {
        std::terminate();
    }
    virtual std::vector<rxn::subscription> subscriptions() const {
        std::terminate();
    }

    virtual std::vector<recorded_type> messages() const {
        return m;
    }
};

template<class T>
subscriber<T, rxt::testable_observer<T>> test_type::test_type_worker::make_subscriber() const
{
    typedef typename rxn::notification<T> notification_type;
    typedef rxn::recorded<typename notification_type::type> recorded_type;

    auto ts = std::make_shared<mock_observer<T>>(state);

    return rxcpp::make_subscriber<T>(rxt::testable_observer<T>(ts, make_observer_dynamic<T>(
          // on_next
          [ts](T value)
          {
              ts->m.push_back(
                              recorded_type(ts->sc->clock(), notification_type::on_next(value)));
          },
          // on_error
          [ts](std::exception_ptr e)
          {
              ts->m.push_back(
                              recorded_type(ts->sc->clock(), notification_type::on_error(e)));
          },
          // on_completed
          [ts]()
          {
              ts->m.push_back(
                              recorded_type(ts->sc->clock(), notification_type::on_completed()));
          })));
}

template<class T>
class cold_observable
    : public rxt::detail::test_subject_base<T>
{
    typedef cold_observable<T> this_type;
    std::shared_ptr<test_type::test_type_state> sc;
    typedef rxn::recorded<typename rxn::notification<T>::type> recorded_type;
    mutable std::vector<recorded_type> mv;
    mutable std::vector<rxn::subscription> sv;
    mutable worker controller;

public:

    cold_observable(std::shared_ptr<test_type::test_type_state> sc, worker w, std::vector<recorded_type> mv)
        : sc(sc)
        , mv(std::move(mv))
        , controller(w)
    {
    }

    template<class Iterator>
    cold_observable(std::shared_ptr<test_type::test_type_state> sc, worker w, Iterator begin, Iterator end)
        : sc(sc)
        , mv(begin, end)
        , controller(w)
    {
    }

    virtual void on_subscribe(subscriber<T> o) const {
        sv.push_back(rxn::subscription(sc->clock()));
        auto index = sv.size() - 1;

        for (auto& message : mv) {
            auto n = message.value();
            sc->schedule_relative(message.time(), make_schedulable(
                controller,
                [n, o](const schedulable&) {
                    if (o.is_subscribed()) {
                        n->accept(o);
                    }
                }));
        }

        auto sharedThis = std::static_pointer_cast<const this_type>(this->shared_from_this());
        o.add([sharedThis, index]() {
            sharedThis->sv[index] = rxn::subscription(sharedThis->sv[index].subscribe(), sharedThis->sc->clock());
        });
    }

    virtual std::vector<rxn::subscription> subscriptions() const {
        return sv;
    }

    virtual std::vector<recorded_type> messages() const {
        return mv;
    }
};

template<class T>
rxt::testable_observable<T> test_type::make_cold_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const
{
    auto co = std::make_shared<cold_observable<T>>(state, create_worker(composite_subscription()), std::move(messages));
    return rxt::testable_observable<T>(co);
}

template<class T>
class hot_observable
    : public rxt::detail::test_subject_base<T>
{
    typedef hot_observable<T> this_type;
    std::shared_ptr<test_type::test_type_state> sc;
    typedef rxn::recorded<typename rxn::notification<T>::type> recorded_type;
    typedef subscriber<T> observer_type;
    mutable std::vector<recorded_type> mv;
    mutable std::vector<rxn::subscription> sv;
    mutable std::list<observer_type> observers;
    mutable worker controller;

public:

    hot_observable(std::shared_ptr<test_type::test_type_state> sc, worker w, std::vector<recorded_type> mv)
        : sc(sc)
        , mv(mv)
        , controller(w)
    {
        for (auto& message : mv) {
            auto n = message.value();
            sc->schedule_absolute(message.time(), make_schedulable(
                controller,
                [this, n](const schedulable&) {
                    auto local = this->observers;
                    for (auto& o : local) {
                        if (o.is_subscribed()) {
                            n->accept(o);
                        }
                    }
                }));
        }
    }

    virtual ~hot_observable() {}

    virtual void on_subscribe(observer_type o) const {
        auto olocation = observers.insert(observers.end(), o);

        sv.push_back(rxn::subscription(sc->clock()));
        auto index = sv.size() - 1;

        auto sharedThis = std::static_pointer_cast<const this_type>(this->shared_from_this());
        o.add([sharedThis, index, olocation]() {
            sharedThis->sv[index] = rxn::subscription(sharedThis->sv[index].subscribe(), sharedThis->sc->clock());
            sharedThis->observers.erase(olocation);
        });
    }

    virtual std::vector<rxn::subscription> subscriptions() const {
        return sv;
    }

    virtual std::vector<recorded_type> messages() const {
        return mv;
    }
};

template<class T>
rxt::testable_observable<T> test_type::make_hot_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const
{
    auto worker = create_worker(composite_subscription());
    auto shared = std::make_shared<hot_observable<T>>(state, worker, std::move(messages));
    return rxt::testable_observable<T>(shared);
}

template<class F>
struct is_create_source_function
{
    struct not_void {};
    template<class CF>
    static auto check(int) -> decltype((*(CF*)nullptr)());
    template<class CF>
    static not_void check(...);

    static const bool value = is_observable<decltype(check<rxu::decay_t<F>>(0))>::value;
};

}

class test : public scheduler
{
    std::shared_ptr<detail::test_type> tester;
public:

    explicit test(std::shared_ptr<detail::test_type> t)
        : scheduler(std::static_pointer_cast<scheduler_interface>(t))
        , tester(t)
    {
    }

    typedef detail::test_type::clock_type clock_type;

    static const long created_time = 100;
    static const long subscribed_time = 200;
    static const long unsubscribed_time = 1000;

    template<class T>
    struct messages
    {
        typedef typename rxn::notification<T> notification_type;
        typedef rxn::recorded<typename notification_type::type> recorded_type;
        typedef rxn::subscription subscription_type;

        messages() {}

        template<typename U>
        static recorded_type next(long ticks, U value) {
            return recorded_type(ticks, notification_type::on_next(std::move(value)));
        }

        static recorded_type completed(long ticks) {
            return recorded_type(ticks, notification_type::on_completed());
        }

        template<typename Exception>
        static recorded_type error(long ticks, Exception&& e) {
            return recorded_type(ticks, notification_type::on_error(std::forward<Exception>(e)));
        }

        static rxn::subscription subscribe(long subscribe, long unsubscribe) {
            return rxn::subscription(subscribe, unsubscribe);
        }
    };

    class test_worker : public worker
    {
        std::shared_ptr<detail::test_type::test_type_worker> tester;
    public:
        
        ~test_worker() {
        }

        explicit test_worker(composite_subscription cs, std::shared_ptr<detail::test_type::test_type_worker> t)
            : worker(cs, std::static_pointer_cast<worker_interface>(t))
            , tester(t)
        {
        }

        bool is_enabled() const {return tester->is_enabled();}
        long clock() const {return tester->clock();}

        void schedule_absolute(long when, const schedulable& a) const {
            tester->schedule_absolute(when, a);
        }

        void schedule_relative(long when, const schedulable& a) const {
            tester->schedule_relative(when, a);
        }

        template<class Arg0, class... ArgN>
        auto schedule_absolute(long when, Arg0&& a0, ArgN&&... an) const
            -> typename std::enable_if<
                (detail::is_action_function<Arg0>::value ||
                is_subscription<Arg0>::value) &&
                !is_schedulable<Arg0>::value>::type {
            tester->schedule_absolute(when, make_schedulable(*this, std::forward<Arg0>(a0), std::forward<ArgN>(an)...));
        }

        template<class Arg0, class... ArgN>
        auto schedule_relative(long when, Arg0&& a0, ArgN&&... an) const
            -> typename std::enable_if<
                (detail::is_action_function<Arg0>::value ||
                is_subscription<Arg0>::value) &&
                !is_schedulable<Arg0>::value>::type {
            tester->schedule_relative(when, make_schedulable(*this, std::forward<Arg0>(a0), std::forward<ArgN>(an)...));
        }

        void advance_to(long time) const
        {
            tester->advance_to(time);
        }

        void advance_by(long time) const
        {
            tester->advance_by(time);
        }

        void sleep(long time) const
        {
            tester->sleep(time);
        }

        template<class T, class F>
        auto start(F createSource, long created, long subscribed, long unsubscribed) const
            -> subscriber<T, rxt::testable_observer<T>>
        {
            struct state_type
            : public std::enable_shared_from_this<state_type>
            {
                typedef decltype(createSource()) source_type;

                std::unique_ptr<source_type> source;
                subscriber<T, rxt::testable_observer<T>> o;

                explicit state_type(subscriber<T, rxt::testable_observer<T>> o)
                : source()
                , o(o)
                {
                }
            };
            auto state = std::make_shared<state_type>(this->make_subscriber<T>());

            schedule_absolute(created, [createSource, state](const schedulable&) {
                state->source.reset(new typename state_type::source_type(createSource()));
            });
            schedule_absolute(subscribed, [state](const schedulable&) {
                state->source->subscribe(state->o);
            });
            schedule_absolute(unsubscribed, [state](const schedulable&) {
                state->o.unsubscribe();
            });

            tester->start();

            return state->o;
        }

        template<class T, class F>
        auto start(F&& createSource, long unsubscribed) const
            -> subscriber<T, rxt::testable_observer<T>>
        {
            return start<T>(std::forward<F>(createSource), created_time, subscribed_time, unsubscribed);
        }

        template<class T, class F>
        auto start(F&& createSource) const
            -> subscriber<T, rxt::testable_observer<T>>
        {
            return start<T>(std::forward<F>(createSource), created_time, subscribed_time, unsubscribed_time);
        }

        template<class F>
        struct start_traits
        {
            typedef decltype((*(F*)nullptr)()) source_type;
            typedef typename source_type::value_type value_type;
            typedef subscriber<value_type, rxt::testable_observer<value_type>> subscriber_type;
        };

        template<class F>
        auto start(F createSource, long created, long subscribed, long unsubscribed) const
            -> typename std::enable_if<detail::is_create_source_function<F>::value, start_traits<F>>::type::subscriber_type
        {
            return start<rxu::value_type_t<start_traits<F>>>(std::move(createSource), created, subscribed, unsubscribed);
        }

        template<class F>
        auto start(F createSource, long unsubscribed) const
            -> typename std::enable_if<detail::is_create_source_function<F>::value, start_traits<F>>::type::subscriber_type
        {
            return start<rxu::value_type_t<start_traits<F>>>(std::move(createSource), created_time, subscribed_time, unsubscribed);
        }

        template<class F>
        auto start(F createSource) const
            -> typename std::enable_if<detail::is_create_source_function<F>::value, start_traits<F>>::type::subscriber_type
        {
            return start<rxu::value_type_t<start_traits<F>>>(std::move(createSource), created_time, subscribed_time, unsubscribed_time);
        }

        void start() const {
            tester->start();
        }

        template<class T>
        subscriber<T, rxt::testable_observer<T>> make_subscriber() const {
            return tester->make_subscriber<T>();
        }
    };

    clock_type::time_point now() const {
        return tester->now();
    }

    test_worker create_worker(composite_subscription cs = composite_subscription()) const {
        return test_worker(cs, tester->create_test_type_worker_interface());
    }

    bool is_enabled() const {return tester->is_enabled();}
    long clock() const {return tester->clock();}

    clock_type::time_point to_time_point(long absolute) const {
        return tester->to_time_point(absolute);
    }

    template<class T>
    rxt::testable_observable<T> make_hot_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const{
        return tester->make_hot_observable(std::move(messages));
    }

    template<class T, std::size_t size>
    auto make_hot_observable(const T (&arr) [size]) const
        -> decltype(tester->make_hot_observable(std::vector<T>())) {
        return      tester->make_hot_observable(rxu::to_vector(arr));
    }

    template<class T>
    auto make_hot_observable(std::initializer_list<T> il) const
        -> decltype(tester->make_hot_observable(std::vector<T>())) {
        return      tester->make_hot_observable(std::vector<T>(il));
    }

    template<class T>
    rxt::testable_observable<T> make_cold_observable(std::vector<rxn::recorded<std::shared_ptr<rxn::detail::notification_base<T>>>> messages) const {
        return tester->make_cold_observable(std::move(messages));
    }

    template<class T, std::size_t size>
    auto make_cold_observable(const T (&arr) [size]) const
        -> decltype(tester->make_cold_observable(std::vector<T>())) {
        return      tester->make_cold_observable(rxu::to_vector(arr));
    }

    template<class T>
    auto make_cold_observable(std::initializer_list<T> il) const
        -> decltype(tester->make_cold_observable(std::vector<T>())) {
        return      tester->make_cold_observable(std::vector<T>(il));
    }
};


inline test make_test() {
    return test(std::make_shared<detail::test_type>());
}

}

inline identity_one_worker identity_test() {
    static identity_one_worker r(rxsc::make_test());
    return r;
}

}

#endif
