//
// Created by Keren Dong on 2020/1/14.
//

#include "watcher.h"
#include "io.h"

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference Watcher::constructor;

    inline location_ptr get_watcher_location(const Napi::CallbackInfo &info)
    {
        return std::make_shared<location>(
                mode::LIVE, category::SYSTEM, "node", info[1].As<Napi::String>().Utf8Value(), IODevice::GetLocator(info)
        );
    }

    Watcher::Watcher(const Napi::CallbackInfo &info) :
            ObjectWrap(info),
            apprentice(get_watcher_location(info), true),
            ledger_location_(mode::LIVE, category::SYSTEM, "service", "ledger", IODevice::GetLocator(info)),
            ledger_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
            update_ledger(ledger_)
    {
        boost::hana::for_each(StateDataTypes, [&](auto it)
        {
            using DataType = typename decltype(+boost::hana::second(it))::type;
            auto name = std::string(boost::hana::first(it).c_str());
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

    void Watcher::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location)
    {
        apprentice::register_location(trigger_time, location);

        if (location->uid == ledger_location_.uid)
        {
            request_read_from_public(now(), ledger_location_.uid, get_master_start_time());
        }
    }

    void Watcher::react()
    {
        events_ | from(ledger_location_.uid) | to(location::PUBLIC) |
        $([&](const event_ptr &event)
          {
              longfist::cast_invoke(event, update_ledger);
          });

        apprentice::react();
    }
}