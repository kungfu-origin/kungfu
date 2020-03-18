//
// Created by Keren Dong on 2020/2/15.
//

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>

#include "longfist.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;

namespace kungfu::node
{
    Napi::ObjectReference Longfist::constructor;

    Longfist::Longfist(const Napi::CallbackInfo &info) : ObjectWrap(info)
    {}

    void Longfist::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Object longfist = Napi::Object::New(env);

        boost::hana::for_each(StateDataTypes, [&](auto it)
        {
            auto name = boost::hana::first(it);
            using DataType = typename decltype(+boost::hana::second(it))::type;
            static const auto make = serialize::JsMake<DataType>(name.c_str());
            longfist.Set(Napi::String::New(env, name.c_str()), Napi::Function::New(env, make));
        });

        constructor = Napi::Persistent(longfist);
        constructor.SuppressDestruct();

        exports.Set("longfist", longfist);
    }
}

