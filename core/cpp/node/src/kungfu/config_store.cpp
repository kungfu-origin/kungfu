//
// Created by Keren Dong on 2020/2/25.
//

#include "io.h"
#include "config_store.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference ConfigStore::constructor;

    ConfigStore::ConfigStore(const Napi::CallbackInfo &info) :
            ObjectWrap(info),
            locator_(IODevice::GetLocator(info)),
            cs_(IODevice::GetLocator(info))
    {}

    ConfigStore::~ConfigStore()
    {}

    inline Config getConfigFromJs(const Napi::CallbackInfo &info, const locator_ptr &locator)
    {
        location config_location(
                get_mode_by_name(info[3].As<Napi::String>().Utf8Value()),
                get_category_by_name(info[0].As<Napi::String>().Utf8Value()),
                info[1].As<Napi::String>().Utf8Value(),
                info[2].As<Napi::String>().Utf8Value(),
                locator);
        Config query{};
        query.location_uid = config_location.uid;
        query.category = config_location.category;
        query.group = config_location.group;
        query.name = config_location.name;
        query.mode = config_location.mode;
        return query;
    }

    Napi::Value ConfigStore::SetConfig(const Napi::CallbackInfo &info)
    {
        Config config = getConfigFromJs(info, locator_);
        config.value = info[4].ToString().Utf8Value();
        cs_.set(config);
        return Napi::Value();
    }

    Napi::Value ConfigStore::GetConfig(const Napi::CallbackInfo &info)
    {
        auto result = Napi::Object::New(info.Env());
        set(cs_.get(getConfigFromJs(info, locator_)), result);
        return result;
    }

    Napi::Value ConfigStore::GetAllConfig(const Napi::CallbackInfo &info)
    {
        auto table = Napi::Object::New(info.Env());
        for (const auto &pair : cs_.get_all(Config{}))
        {
            auto data = pair.second;
            std::string uid = fmt::format("{:016x}", data.uid());
            Napi::Value value = Napi::Object::New(info.Env());
            table.Set(uid, value);
            set(data, value);
        }
        return table;
    }

    Napi::Value ConfigStore::RemoveConfig(const Napi::CallbackInfo &info)
    {
        cs_.remove(cs_.get(getConfigFromJs(info, locator_)));
        return Napi::Value();
    }

    void ConfigStore::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "ConfigStore", {
                InstanceMethod("setConfig", &ConfigStore::SetConfig),
                InstanceMethod("getConfig", &ConfigStore::GetConfig),
                InstanceMethod("getAllConfig", &ConfigStore::GetAllConfig),
                InstanceMethod("removeConfig", &ConfigStore::RemoveConfig),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("ConfigStore", func);
    }

    Napi::Value ConfigStore::NewInstance(const Napi::Value arg)
    {
        return constructor.New({arg});
    }
}

