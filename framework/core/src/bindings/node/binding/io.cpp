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

Napi::FunctionReference IODevice::constructor = {};

IODevice::IODevice(const Napi::CallbackInfo &info) : ObjectWrap(info), io_device(GetLocation(info), true, true) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
}

Napi::Value IODevice::OpenReader(const Napi::CallbackInfo &info) { return Reader::NewInstance(info.This()); }

locator_ptr IODevice::GetLocator(const Napi::CallbackInfo &info, int index) {
  if (not IsValid(info, index, &Napi::Value::IsObject)) {
    throw Napi::Error::New(info.Env(), "Invalid locator argument");
  }

  auto dirname = info[index].As<Napi::String>().Utf8Value();
  return GetRuntimeLocator(dirname);
}

location_ptr IODevice::GetLocation(const Napi::CallbackInfo &info) {
  longfist::enums::category c = longfist::enums::get_category_by_name(info[0].As<Napi::String>().Utf8Value());
  auto group = info[1].As<Napi::String>().Utf8Value();
  auto name = info[2].As<Napi::String>().Utf8Value();
  longfist::enums::mode m = get_mode_by_name(info[3].As<Napi::String>().Utf8Value());
  return std::make_shared<location>(m, c, group, name, GetLocator(info, 4));
}

void IODevice::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "IODevice",
                                    {
                                        InstanceMethod("openReader", &IODevice::OpenReader),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("IODevice", func);
}
} // namespace kungfu::node