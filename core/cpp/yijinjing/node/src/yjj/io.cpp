//
// Created by Keren Dong on 2020/1/1.
//

#include "io.h"

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference Locator::constructor;

    Locator::Locator(const Napi::CallbackInfo &info) : ObjectWrap(info)
    {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);

        int length = info.Length();

        if (length <= 0 || !info[0].IsObject())
        {
            Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
            return;
        }
        locator_ = Napi::Reference<Napi::Object>::New(info[0].As<Napi::Object>());
    }

    bool Locator::has_env(const std::string &name) const
    {
        auto js_delegate = locator_.Get("has_env").As<Napi::Function>();
        auto v = js_delegate.Call({Napi::String::New(locator_.Env(), name)});
        return v.As<Napi::Boolean>();
    }

    std::string Locator::get_env(const std::string &name) const
    {
        auto js_delegate = locator_.Get("get_env").As<Napi::Function>();
        auto v = js_delegate.Call({Napi::String::New(locator_.Env(), name)});
        return v.As<Napi::String>().Utf8Value();
    }

    std::string Locator::layout_dir(location_ptr location, layout l) const
    {
        auto js_delegate = locator_.Get("layout_dir").As<Napi::Function>();
        auto v = js_delegate.Call({
                                             Napi::String::New(locator_.Env(), get_category_name(location->category)),
                                             Napi::String::New(locator_.Env(), location->group),
                                             Napi::String::New(locator_.Env(), location->name),
                                             Napi::String::New(locator_.Env(), get_mode_name(location->mode)),
                                             Napi::String::New(locator_.Env(), get_layout_name(l))
                                     });
        return v.As<Napi::String>().Utf8Value();
    }

    std::string Locator::layout_file(location_ptr location, layout l, const std::string &name) const
    {
        auto js_delegate = locator_.Get("layout_file").As<Napi::Function>();
        auto v = js_delegate.Call({
                                             Napi::String::New(locator_.Env(), get_category_name(location->category)),
                                             Napi::String::New(locator_.Env(), location->group),
                                             Napi::String::New(locator_.Env(), location->name),
                                             Napi::String::New(locator_.Env(), get_mode_name(location->mode)),
                                             Napi::String::New(locator_.Env(), get_layout_name(l)),
                                             Napi::String::New(locator_.Env(), name)
                                     });
        return v.As<Napi::String>().Utf8Value();
    }

    std::string Locator::default_to_system_db(location_ptr location, const std::string &name) const
    {
        throw yijinjing_error("not supported");
    }

    std::vector<int> Locator::list_page_id(location_ptr location, uint32_t dest_id) const
    {
        auto js_delegate = locator_.Get("list_page_id").As<Napi::Function>();
        auto v = js_delegate.Call({
                                          Napi::String::New(locator_.Env(), get_category_name(location->category)),
                                          Napi::String::New(locator_.Env(), location->group),
                                          Napi::String::New(locator_.Env(), location->name),
                                          Napi::String::New(locator_.Env(), get_mode_name(location->mode)),
                                          Napi::Number::New(locator_.Env(), dest_id)
                                  });
        Napi::Array r = v.As<Napi::Array>();
        std::vector<int> result;
        for (int i = 0; i < r.Length(); i++)
        {
            Napi::Value e = r[i];
            result.push_back(e.As<Napi::Number>());
        }
        return result;
    }

    std::vector<location_ptr>
    Locator::list_locations(const std::string &category, const std::string &group, const std::string &name, const std::string &mode) const
    {
        std::vector<location_ptr> result;
        return result;
    }

    std::vector<uint32_t> Locator::list_location_dest(location_ptr location) const
    {
        std::vector<uint32_t> result;
        return result;
    }

    Napi::Value Locator::ToString(const Napi::CallbackInfo &info)
    {
        return Napi::String::New(info.Env(), "Locator.js");
    }

    void Locator::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Locator", {
                InstanceMethod("toString", &Locator::ToString)
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Locator", func);
    }

    Napi::FunctionReference IODevice::constructor;

    IODevice::IODevice(const Napi::CallbackInfo &info) : ObjectWrap(info), io_device_client(GetLocation(info), false)
    {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);
    }

    Napi::Value IODevice::ToString(const Napi::CallbackInfo &info)
    {
        return Napi::String::New(info.Env(), "IODevice.js@" + home_->uname);
    }

    Napi::Value IODevice::FindSessions(const Napi::CallbackInfo &info)
    {
        auto sessions = find_sessions();
        Napi::Array result = Napi::Array::New(info.Env(), sessions.size());
        for (uint32_t i = 0; i < sessions.size(); i++)
        {
            result[i] = Napi::String::New(info.Env(), sessions[i]);
        }
        return result;
    }

    Napi::Value IODevice::OpenReader(const Napi::CallbackInfo &info)
    {
        return Reader::NewInstance(info.This());
    }

    location_ptr IODevice::GetLocation(const Napi::CallbackInfo &info)
    {
        mode m = get_mode_by_name(info[0].As<Napi::String>().Utf8Value());
        category c = get_category_by_name(info[1].As<Napi::String>().Utf8Value());
        auto group = info[2].As<Napi::String>().Utf8Value();
        auto name = info[3].As<Napi::String>().Utf8Value();
        locator_ptr lp(reinterpret_cast<Locator *>(Napi::ObjectWrap<Locator>::Unwrap(info[4].As<Napi::Object>())));
        return std::make_shared<location>(m, c, group, name, lp);
    }

    void IODevice::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "IODevice", {
                InstanceMethod("toString", &IODevice::ToString),
                InstanceMethod("findSessions", &IODevice::FindSessions),
                InstanceMethod("openReader", &IODevice::OpenReader),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("IODevice", func);
    }

    Napi::FunctionReference Session::constructor;

    Session::Session(const Napi::CallbackInfo &info) : ObjectWrap(info)
    {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);
    }

    Napi::Value Session::ToString(const Napi::CallbackInfo &info)
    {
        return Napi::String::New(info.Env(), "Session.js");
    }

    void Session::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "Session", {
                InstanceMethod("toString", &Session::ToString)
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("Session", func);
    }
}