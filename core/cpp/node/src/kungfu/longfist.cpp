//
// Created by Keren Dong on 2020/2/15.
//

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
            auto name = boost::hana::first(it).c_str();
            using DataType = typename decltype(+boost::hana::second(it))::type;
            longfist.Set(Napi::String::New(env, name), Napi::Function::New(env, Longfist::data<DataType>));
        });

        constructor = Napi::Persistent(longfist);
        constructor.SuppressDestruct();

        exports.Set("longfist", longfist);
    }
}

