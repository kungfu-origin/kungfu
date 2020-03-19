//
// Created by Keren Dong on 2020/1/1.
//

#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
location_ptr ExtractLocation(const Napi::CallbackInfo &info, int index, const locator_ptr &locator) {
  try {
    if (info[index].IsObject()) {
      auto obj = info[index].ToObject();
      return location::make_shared(get_mode_by_name(obj.Get("mode").ToString().Utf8Value()),
                                   get_category_by_name(obj.Get("category").ToString().Utf8Value()),
                                   obj.Get("group").ToString().Utf8Value(), obj.Get("name").ToString().Utf8Value(),
                                   locator);
    } else {
      return location::make_shared(get_mode_by_name(info[index + 3].As<Napi::String>().Utf8Value()),
                                   get_category_by_name(info[index].As<Napi::String>().Utf8Value()),
                                   info[index + 1].As<Napi::String>().Utf8Value(),
                                   info[index + 2].As<Napi::String>().Utf8Value(), locator);
    }
  } catch (const std::exception &ex) {
    throw Napi::Error::New(info.Env(), fmt::format("invalid location arguments: {}", ex.what()));
  } catch (...) {
    throw Napi::Error::New(info.Env(), "invalid location arguments");
  }
}

Locator::Locator(const Napi::Object &locator_obj) { locator_ref_ = Napi::ObjectReference::New(locator_obj, 1); }

Locator::~Locator() { locator_ref_.Unref(); }

bool Locator::has_env(const std::string &name) const {
  auto js_delegate = locator_ref_.Get("has_env").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), name)});
  return v.As<Napi::Boolean>();
}

std::string Locator::get_env(const std::string &name) const {
  auto js_delegate = locator_ref_.Get("get_env").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), name)});
  return v.As<Napi::String>().Utf8Value();
}

std::string Locator::layout_dir(location_ptr location, longfist::enums::layout l) const {
  auto js_delegate = locator_ref_.Get("layout_dir").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), get_category_name(location->category)),
                             Napi::String::New(locator_ref_.Env(), location->group),
                             Napi::String::New(locator_ref_.Env(), location->name),
                             Napi::String::New(locator_ref_.Env(), longfist::enums::get_mode_name(location->mode)),
                             Napi::String::New(locator_ref_.Env(), longfist::enums::get_layout_name(l))});
  return v.As<Napi::String>().Utf8Value();
}

std::string Locator::layout_file(location_ptr location, longfist::enums::layout l, const std::string &name) const {
  auto js_delegate = locator_ref_.Get("layout_file").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), get_category_name(location->category)),
                             Napi::String::New(locator_ref_.Env(), location->group),
                             Napi::String::New(locator_ref_.Env(), location->name),
                             Napi::String::New(locator_ref_.Env(), get_mode_name(location->mode)),
                             Napi::String::New(locator_ref_.Env(), longfist::enums::get_layout_name(l)),
                             Napi::String::New(locator_ref_.Env(), name)});
  return v.As<Napi::String>().Utf8Value();
}

std::string Locator::default_to_system_db(location_ptr location, const std::string &name) const {
  throw yijinjing_error("not supported");
}

std::vector<int> Locator::list_page_id(location_ptr location, uint32_t dest_id) const {
  auto js_delegate = locator_ref_.Get("list_page_id").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), get_category_name(location->category)),
                             Napi::String::New(locator_ref_.Env(), location->group),
                             Napi::String::New(locator_ref_.Env(), location->name),
                             Napi::String::New(locator_ref_.Env(), get_mode_name(location->mode)),
                             Napi::Number::New(locator_ref_.Env(), dest_id)});
  Napi::Array r = v.As<Napi::Array>();
  std::vector<int> result;
  for (int i = 0; i < r.Length(); i++) {
    Napi::Value e = r[i];
    result.push_back(e.As<Napi::Number>());
  }
  return result;
}

std::vector<location_ptr> Locator::list_locations(const std::string &category, const std::string &group,
                                                  const std::string &name, const std::string &mode) const {
  std::vector<location_ptr> result;
  return result;
}

std::vector<uint32_t> Locator::list_location_dest(location_ptr location) const {
  std::vector<uint32_t> result;
  auto js_delegate = locator_ref_.Get("list_location_dest").As<Napi::Function>();
  auto v = js_delegate.Call({Napi::String::New(locator_ref_.Env(), get_category_name(location->category)),
                             Napi::String::New(locator_ref_.Env(), location->group),
                             Napi::String::New(locator_ref_.Env(), location->name),
                             Napi::String::New(locator_ref_.Env(), get_mode_name(location->mode))});
  Napi::Array r = v.As<Napi::Array>();
  for (int i = 0; i < r.Length(); i++) {
    Napi::Value e = r[i];
    result.push_back(e.As<Napi::Number>().Uint32Value());
  }
  return result;
}

Napi::Object Locator::get_js_locator() { return locator_ref_.Value(); }

Napi::FunctionReference IODevice::constructor;

IODevice::IODevice(const Napi::CallbackInfo &info) : ObjectWrap(info), io_device(GetLocation(info), true, true) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
}

Napi::Value IODevice::ToString(const Napi::CallbackInfo &info) {
  return Napi::String::New(info.Env(), "IODevice.js@" + home_->uname);
}

Napi::Value IODevice::FindSessions(const Napi::CallbackInfo &info) {
  auto sessions = find_sessions();
  Napi::Array result = Napi::Array::New(info.Env(), sessions.size());
  for (uint32_t i = 0; i < sessions.size(); i++) {
    result[i] = Napi::String::New(info.Env(), sessions[i]);
  }
  return result;
}

Napi::Value IODevice::OpenReader(const Napi::CallbackInfo &info) { return Reader::NewInstance(info.This()); }

locator_ptr IODevice::GetLocator(const Napi::CallbackInfo &info, int index) {
  return std::dynamic_pointer_cast<locator>(std::make_shared<Locator>(info[index].As<Napi::Object>()));
}

location_ptr IODevice::GetLocation(const Napi::CallbackInfo &info) {
  longfist::enums::category c = longfist::enums::get_category_by_name(info[0].As<Napi::String>().Utf8Value());
  auto group = info[1].As<Napi::String>().Utf8Value();
  auto name = info[2].As<Napi::String>().Utf8Value();
  longfist::enums::mode m = longfist::enums::get_mode_by_name(info[3].As<Napi::String>().Utf8Value());
  return std::make_shared<location>(m, c, group, name, GetLocator(info, 4));
}

void IODevice::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IODevice",
                                    {
                                        InstanceMethod("toString", &IODevice::ToString),
                                        InstanceMethod("findSessions", &IODevice::FindSessions),
                                        InstanceMethod("openReader", &IODevice::OpenReader),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("IODevice", func);
}

Napi::FunctionReference Session::constructor;

Session::Session(const Napi::CallbackInfo &info) : ObjectWrap(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
}

Napi::Value Session::ToString(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), "Session.js"); }

void Session::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Session", {InstanceMethod("toString", &Session::ToString)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Session", func);
}
} // namespace kungfu::node