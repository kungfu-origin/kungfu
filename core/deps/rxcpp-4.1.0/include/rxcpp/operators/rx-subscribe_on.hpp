// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

/*! \file rx-subscribe_on.hpp

    \brief Subscription and unsubscription are queued and delivered using the scheduler from the supplied coordination.

    \tparam Coordination  the type of the scheduler.

    \param  cn  the scheduler to perform subscription actions on.

    \return  The source observable modified so that its subscriptions happen on the specified scheduler.

    \sample
    \snippet subscribe_on.cpp subscribe_on sample
    \snippet output.txt subscribe_on sample

    Invoking rxcpp::observable::observe_on operator, instead of subscribe_on, gives following results:
    \snippet output.txt observe_on sample
*/

#if !defined(RXCPP_OPERATORS_RX_SUBSCRIBE_ON_HPP)
#define RXCPP_OPERATORS_RX_SUBSCRIBE_ON_HPP

#include "../rx-includes.hpp"

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct subscribe_on_invalid_arguments {};

template<class... AN>
struct subscribe_on_invalid : public rxo::operator_base<subscribe_on_invalid_arguments<AN...>> {
    using type = observable<subscribe_on_invalid_arguments<AN...>, subscribe_on_invalid<AN...>>;
};
template<class... AN>
using subscribe_on_invalid_t = typename subscribe_on_invalid<AN...>::type;

template<class T, class Observable, class Coordination>
struct subscribe_on : public operator_base<T>
{
    typedef rxu::decay_t<Observable> source_type;
    typedef rxu::decay_t<Coordination> coordination_type;
    typedef typename coordination_type::coordinator_type coordinator_type;
    struct subscribe_on_values
    {
        ~subscribe_on_values()
        {
        }
        subscribe_on_values(source_type s, coordination_type sf)
            : source(std::move(s))
            , coordination(std::move(sf))
        {
        }
        source_type source;
        coordination_type coordination;
    private:
        subscribe_on_values& operator=(subscribe_on_values o) RXCPP_DELETE;
    };
    const subscribe_on_values initial;

    ~subscribe_on()
    {
    }
    subscribe_on(source_type s, coordination_type sf)
        : initial(std::move(s), std::move(sf))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber s) const {

        typedef Subscriber output_type;
        struct subscribe_on_state_type
            : public std::enable_shared_from_this<subscribe_on_state_type>
            , public subscribe_on_values
        {
            subscribe_on_state_type(const subscribe_on_values& i, const output_type& oarg)
                : subscribe_on_values(i)
                , out(oarg)
            {
            }
            composite_subscription source_lifetime;
            output_type out;
        private:
            subscribe_on_state_type& operator=(subscribe_on_state_type o) RXCPP_DELETE;
        };

        composite_subscription coordinator_lifetime;

        auto coordinator = initial.coordination.create_coordinator(coordinator_lifetime);

        auto controller = coordinator.get_worker();

        // take a copy of the values for each subscription
        auto state = std::make_shared<subscribe_on_state_type>(initial, std::move(s));

        auto sl = state->source_lifetime;
        auto ol = state->out.get_subscription();

        auto disposer = [=](const rxsc::schedulable&){
            sl.unsubscribe();
            ol.unsubscribe();
            coordinator_lifetime.unsubscribe();
        };
        auto selectedDisposer = on_exception(
            [&](){return coordinator.act(disposer);},
            state->out);
        if (selectedDisposer.empty()) {
            return;
        }
        
        state->source_lifetime.add([=](){
            controller.schedule(selectedDisposer.get());
        });

        state->out.add([=](){
            sl.unsubscribe();
            ol.unsubscribe();
            coordinator_lifetime.unsubscribe();
        });

        auto producer = [=](const rxsc::schedulable&){
            state->source.subscribe(state->source_lifetime, state->out);
        };

        auto selectedProducer = on_exception(
            [&](){return coordinator.act(producer);},
            state->out);
        if (selectedProducer.empty()) {
            return;
        }

        controller.schedule(selectedProducer.get());
    }
private:
    subscribe_on& operator=(subscribe_on o) RXCPP_DELETE;
};

}

/*! @copydoc rx-subscribe_on.hpp
*/
template<class... AN>
auto subscribe_on(AN&&... an)
    ->      operator_factory<subscribe_on_tag, AN...> {
     return operator_factory<subscribe_on_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<subscribe_on_tag>
{
    template<class Observable, class Coordination,
        class Enabled = rxu::enable_if_all_true_type_t<
            is_observable<Observable>,
            is_coordination<Coordination>>,
        class SourceValue = rxu::value_type_t<Observable>,
        class SubscribeOn = rxo::detail::subscribe_on<SourceValue, rxu::decay_t<Observable>, rxu::decay_t<Coordination>>,
        class Value = rxu::value_type_t<SubscribeOn>,
        class Result = observable<Value, SubscribeOn>>
    static Result member(Observable&& o, Coordination&& cn) {
        return Result(SubscribeOn(std::forward<Observable>(o), std::forward<Coordination>(cn)));
    }

    template<class... AN>
    static operators::detail::subscribe_on_invalid_t<AN...> member(AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "subscribe_on takes (Coordination)");
    }
};

}

#endif
