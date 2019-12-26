//
// Created by Keren Dong on 2020/1/1.
//

#include "journal.h"

namespace kungfu::node
{
    Napi::FunctionReference Reader::constructor;

    Reader::Reader(const Napi::CallbackInfo &info) : ObjectWrap(info), reader(true)
    {}

    Napi::Value Reader::ToString(const Napi::CallbackInfo &info)
    {
        return Napi::String::New(info.Env(), "Reader.js");
    }

    void Reader::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Reader", {
                InstanceMethod("toString", &Reader::ToString)
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Reader", func);
    }

    Napi::Value Reader::NewInstance()
    {
        return constructor.New({});
    }
}
