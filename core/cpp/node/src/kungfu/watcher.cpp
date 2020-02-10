//
// Created by Keren Dong on 2020/1/14.
//

#include "watcher.h"
#include "io.h"

using namespace boost::hana;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference Watcher::constructor;

    Watcher::Watcher(const Napi::CallbackInfo &info) :
            ObjectWrap(info),
            apprentice(std::make_shared<location>(mode::LIVE, category::SYSTEM, "node", "watcher", IODevice::GetLocator(info)), true),
            ledger_(Napi::ObjectReference::New(Napi::Object::New(info.Env())))
    {
        for_each(StateDataTypes, [&](auto it)
        {
            using DataType = typename decltype(+second(it))::type;
            auto name = std::string(first(it).c_str());
            auto states = Napi::Object::New(info.Env());
            ledger_.Set(name, states);
        });
    }

    Napi::Value Watcher::IsLive(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(info.Env(), is_live());
    }

    Napi::Value Watcher::Setup(const Napi::CallbackInfo &info)
    {
        setup();
        return Napi::Value();
    }

    Napi::Value Watcher::Step(const Napi::CallbackInfo &info)
    {
        step();
        return Napi::Value();
    }

    Napi::Value Watcher::GetLedger(const Napi::CallbackInfo &info)
    {
        return ledger_.Value();
    }

    void Watcher::SetLedger(const Napi::CallbackInfo &info, const Napi::Value &value)
    {}

    void Watcher::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Watcher", {
                InstanceMethod("isLive", &Watcher::IsLive),
                InstanceMethod("setup", &Watcher::Setup),
                InstanceMethod("step", &Watcher::Step),
                InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::SetLedger),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Watcher", func);
    }
}