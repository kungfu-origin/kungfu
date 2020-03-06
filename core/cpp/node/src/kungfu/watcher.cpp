//
// Created by Keren Dong on 2020/1/14.
//

#include "io.h"
#include "watcher.h"

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::sqlite;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference Watcher::constructor;

    inline location_ptr GetWatcherLocation(const Napi::CallbackInfo &info)
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
            apprentice(GetWatcherLocation(info), true),
            ledger_location_(mode::LIVE, category::SYSTEM, "service", "ledger", get_locator()),
            config_ref_(Napi::ObjectReference::New(ConfigStore::NewInstance({info[0]}).ToObject(), 1)),
            state_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
            ledger_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
            update_state(state_ref_),
            update_ledger(ledger_ref_),
            publish(*this, state_ref_)
    {
        InitStateMap(info, state_ref_);
        InitStateMap(info, ledger_ref_);
        SPDLOG_INFO("watcher created at {}", get_io_device()->get_home()->uname);

        auto locator = get_locator();
        for (const auto &pair : ConfigStore::Unwrap(config_ref_.Value())->cs_.get_all(Config{}))
        {
            RestoreState(location::make_shared(pair.second, locator));
        }
        RestoreState(location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", locator));
        SPDLOG_INFO("watcher ledger restored");
    }

    Watcher::~Watcher()
    {
        ledger_ref_.Unref();
        state_ref_.Unref();
        config_ref_.Unref();
    }

    void Watcher::NoSet(const Napi::CallbackInfo &info, const Napi::Value &value)
    {
        SPDLOG_WARN("do not manipulate watcher internals");
    }

    Napi::Value Watcher::GetLocator(const Napi::CallbackInfo &info)
    {
        return std::dynamic_pointer_cast<Locator>(get_locator())->get_js_locator();
    }

    Napi::Value Watcher::GetConfig(const Napi::CallbackInfo &info)
    {
        return config_ref_.Value();
    }

    Napi::Value Watcher::GetState(const Napi::CallbackInfo &info)
    {
        return state_ref_.Value();
    }

    Napi::Value Watcher::GetLedger(const Napi::CallbackInfo &info)
    {
        return ledger_ref_.Value();
    }

    Napi::Value Watcher::IsUsable(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(info.Env(), is_usable());
    }

    Napi::Value Watcher::IsLive(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(info.Env(), is_live());
    }

    Napi::Value Watcher::IsStarted(const Napi::CallbackInfo &info)
    {
        return Napi::Boolean::New(info.Env(), is_started());
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

    Napi::Value Watcher::GetLocation(const Napi::CallbackInfo &info)
    {
        auto locationObj = Napi::Object::New(info.Env());
        auto uid = info.Length() > 0 ? info[0].ToNumber().Uint32Value() : 0;
        if (uid == 0)
        {
            auto location = get_io_device()->get_home();
            locationObj.Set("category", Napi::String::New(info.Env(), get_category_name(location->category)));
            locationObj.Set("group", Napi::String::New(info.Env(), location->group));
            locationObj.Set("name", Napi::String::New(info.Env(), location->name));
            locationObj.Set("mode", Napi::String::New(info.Env(), get_mode_name(location->mode)));
            locationObj.Set("locator", std::dynamic_pointer_cast<Locator>(location->locator)->get_js_locator());
        }
        if (has_location(uid))
        {
            auto location = get_location(uid);
            locationObj.Set("category", Napi::String::New(info.Env(), get_category_name(location->category)));
            locationObj.Set("group", Napi::String::New(info.Env(), location->group));
            locationObj.Set("name", Napi::String::New(info.Env(), location->name));
            locationObj.Set("mode", Napi::String::New(info.Env(), get_mode_name(location->mode)));
            locationObj.Set("locator", std::dynamic_pointer_cast<Locator>(location->locator)->get_js_locator());
        }
        return locationObj;
    }

    Napi::Value Watcher::PublishState(const Napi::CallbackInfo &info)
    {
        Napi::Object obj = info[0].ToObject();
        longfist::cast_type_invoke(obj.Get("type").ToString().Utf8Value(), obj, publish);
        return Napi::Value();
    }

    Napi::Value Watcher::isReadyToInteract(const Napi::CallbackInfo &info)
    {
        auto account_location = ExtractLocation(info, 0, get_locator());
        return Napi::Boolean::New(info.Env(), account_location and has_writer(account_location->uid));
    }

    Napi::Value Watcher::IssueOrder(const Napi::CallbackInfo &info)
    {
        return InteractWithTD<OrderInput>(info, &OrderInput::order_id);
    }

    Napi::Value Watcher::CancelOrder(const Napi::CallbackInfo &info)
    {
        return InteractWithTD<OrderAction>(info, &OrderAction::order_action_id);
    }

    void Watcher::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Watcher", {
                InstanceMethod("isUsable", &Watcher::IsUsable),
                InstanceMethod("isLive", &Watcher::IsLive),
                InstanceMethod("isStarted", &Watcher::IsStarted),
                InstanceMethod("setup", &Watcher::Setup),
                InstanceMethod("step", &Watcher::Step),
                InstanceMethod("getLocation", &Watcher::GetLocation),
                InstanceMethod("publishState", &Watcher::PublishState),
                InstanceMethod("isReadyToInteract", &Watcher::isReadyToInteract),
                InstanceMethod("issueOrder", &Watcher::IssueOrder),
                InstanceMethod("cancelOrder", &Watcher::CancelOrder),
                InstanceAccessor("locator", &Watcher::GetLocator, &Watcher::NoSet),
                InstanceAccessor("config", &Watcher::GetConfig, &Watcher::NoSet),
                InstanceAccessor("state", &Watcher::GetState, &Watcher::NoSet),
                InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::NoSet),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Watcher", func);
    }

    void Watcher::react()
    {
        events_ |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, update_state);
          });

        apprentice::react();
    }

    void Watcher::on_start()
    {
        events_ |
        $([&](const event_ptr &event)
          {
              longfist::cast_event_invoke(event, update_ledger);
          });

        events_ | is(Channel::tag) |
        $([&](const event_ptr &event)
          {
              auto home_uid = get_home_uid();
              const Channel &channel = event->data<Channel>();
              if (channel.source_id != home_uid and channel.dest_id != home_uid)
              {
                  reader_->join(get_location(channel.source_id), channel.dest_id, event->gen_time());
              }
          });

        events_ | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              auto register_data = event->data<Register>();
              auto app_location = location::make_shared(register_data, get_locator());
              request_write_to(event->gen_time(), app_location->uid);
              request_read_from(event->gen_time(), app_location->uid, register_data.checkin_time);
              request_read_from_public(event->gen_time(), app_location->uid, register_data.checkin_time);
          });
    }

    void Watcher::RestoreState(const location_ptr &state_location)
    {
        add_location(0, state_location);
        serialize::JsRestoreState(*this, ledger_ref_, state_location)();
    }
}