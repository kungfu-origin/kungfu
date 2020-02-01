//
// Created by Keren Dong on 2020/1/14.
//

#include "app.h"
#include "io.h"

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference Apprentice::constructor;

    Apprentice::Apprentice(const Napi::CallbackInfo &info) : ObjectWrap(info), apprentice(IODevice::GetLocation(info), true)
    {}

    Napi::Value Apprentice::IsLive(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(info.Env(), is_live());
    }

    Napi::Value Apprentice::Setup(const Napi::CallbackInfo &info)
    {
        setup();
        return Napi::Value();
    }

    Napi::Value Apprentice::Step(const Napi::CallbackInfo &info)
    {
        step();
        return Napi::Value();
    }

    void Apprentice::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Apprentice", {
                InstanceMethod("isLive", &Apprentice::IsLive),
                InstanceMethod("setup", &Apprentice::Setup),
                InstanceMethod("step", &Apprentice::Step),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Apprentice", func);
    }
}