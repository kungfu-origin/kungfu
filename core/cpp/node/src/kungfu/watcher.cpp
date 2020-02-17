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

    inline void InitStateMap(const Napi::CallbackInfo &info, Napi::ObjectReference &ref)
    {
        boost::hana::for_each(StateDataTypes, [&](auto it)
        {
            using DataType = typename decltype(+boost::hana::second(it))::type;
            auto name = std::string(boost::hana::first(it).c_str());
            ref.Set(name, Napi::Object::New(info.Env()));
        });
    }

    Watcher::Watcher(const Napi::CallbackInfo &info) :
            ObjectWrap(info),
            apprentice(get_watcher_location(info), true),
            ledger_location_(mode::LIVE, category::SYSTEM, "service", "ledger", get_io_device()->get_home()->locator),
            state_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
            ledger_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
            update_state(state_ref_),
            update_ledger(ledger_ref_),
            publish(*this, state_ref_)
    {
        InitStateMap(info, state_ref_);
        InitStateMap(info, ledger_ref_);
        SPDLOG_INFO("watcher created at {}", get_io_device()->get_home()->uname);
    }

    Watcher::~Watcher()
    {
        ledger_ref_.Unref();
        state_ref_.Unref();
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

    Napi::Value Watcher::GetState(const Napi::CallbackInfo &info)
    {
        return state_ref_.Value();
    }

    void Watcher::SetState(const Napi::CallbackInfo &info, const Napi::Value &value)
    {}

    Napi::Value Watcher::GetLedger(const Napi::CallbackInfo &info)
    {
        return ledger_ref_.Value();
    }

    void Watcher::SetLedger(const Napi::CallbackInfo &info, const Napi::Value &value)
    {}

    Napi::Value Watcher::PublishState(const Napi::CallbackInfo &info)
    {
        Napi::Object obj = info[0].ToObject();
        longfist::cast_type_invoke(obj.Get("type").ToString().Utf8Value(), obj, publish);
        return Napi::Value();
    }

    void Watcher::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Watcher", {
                InstanceMethod("isLive", &Watcher::IsLive),
                InstanceMethod("setup", &Watcher::Setup),
                InstanceMethod("step", &Watcher::Step),
                InstanceMethod("publishState", &Watcher::PublishState),
                InstanceAccessor("state", &Watcher::GetState, &Watcher::SetState),
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
        events_ | from(get_master_commands_uid()) |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, update_state);
          });

        events_ | from(ledger_location_.uid) | to(location::PUBLIC) |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, update_ledger);
          });

        apprentice::react();
    }
}