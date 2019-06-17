// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_SOURCES_RX_SCOPE_HPP)
#define RXCPP_SOURCES_RX_SCOPE_HPP

#include "../rx-includes.hpp"

/*! \file rx-scope.hpp

    \brief Returns an observable that makes an observable by the specified observable factory using the resource provided by the specified resource factory for each new observer that subscribes.

    \tparam ResourceFactory    the type of the resource factory
    \tparam ObservableFactory  the type of the observable factory

    \param  rf  the resource factory function that resturn the rxcpp::resource that is used as a resource by the observable factory
    \param  of  the observable factory function to invoke for each observer that subscribes to the resulting observable

    \return  observable that makes an observable by the specified observable factory using the resource provided by the specified resource factory for each new observer that subscribes.

    \sample
    \snippet scope.cpp scope sample
    \snippet output.txt scope sample
*/

namespace rxcpp {

namespace sources {

namespace detail {

template<class ResourceFactory, class ObservableFactory>
struct scope_traits
{
    typedef rxu::decay_t<ResourceFactory> resource_factory_type;
    typedef rxu::decay_t<ObservableFactory> observable_factory_type;
    typedef decltype((*(resource_factory_type*)nullptr)()) resource_type;
    typedef decltype((*(observable_factory_type*)nullptr)(resource_type())) collection_type;
    typedef typename collection_type::value_type value_type;

    static_assert(is_subscription<resource_type>::value, "ResourceFactory must return a subscription");
};

template<class ResourceFactory, class ObservableFactory>
struct scope : public source_base<rxu::value_type_t<scope_traits<ResourceFactory, ObservableFactory>>>
{
    typedef scope_traits<ResourceFactory, ObservableFactory> traits;
    typedef typename traits::resource_factory_type resource_factory_type;
    typedef typename traits::observable_factory_type observable_factory_type;
    typedef typename traits::resource_type resource_type;
    typedef typename traits::value_type value_type;

    struct values
    {
        values(resource_factory_type rf, observable_factory_type of)
            : resource_factory(std::move(rf))
            , observable_factory(std::move(of))
        {
        }
        resource_factory_type resource_factory;
        observable_factory_type observable_factory;
    };
    values initial;


    scope(resource_factory_type rf, observable_factory_type of)
        : initial(std::move(rf), std::move(of))
    {
    }

    template<class Subscriber>
    void on_subscribe(Subscriber o) const {

        struct state_type
            : public std::enable_shared_from_this<state_type>
            , public values
        {
            state_type(values i, Subscriber o)
                : values(i)
                , out(std::move(o))
            {
            }
            Subscriber out;
            rxu::detail::maybe<resource_type> resource;
        };

        auto state = std::make_shared<state_type>(state_type(initial, std::move(o)));

        state->resource = on_exception(
            [&](){return state->resource_factory(); },
            state->out);
        if (state->resource.empty()) {
            return;
        }
        state->out.add(state->resource->get_subscription());

        auto selectedCollection = on_exception(
            [state](){return state->observable_factory(state->resource.get()); },
            state->out);
        if (selectedCollection.empty()) {
            return;
        }

        selectedCollection->subscribe(state->out);
    }
};

}

/*! @copydoc rx-scope.hpp
 */
template<class ResourceFactory, class ObservableFactory>
auto scope(ResourceFactory rf, ObservableFactory of)
    ->      observable<rxu::value_type_t<detail::scope_traits<ResourceFactory, ObservableFactory>>, detail::scope<ResourceFactory, ObservableFactory>> {
    return  observable<rxu::value_type_t<detail::scope_traits<ResourceFactory, ObservableFactory>>, detail::scope<ResourceFactory, ObservableFactory>>(
                                                                                                    detail::scope<ResourceFactory, ObservableFactory>(std::move(rf), std::move(of)));
}

}

}

#endif
