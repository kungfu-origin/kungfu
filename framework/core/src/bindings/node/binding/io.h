//
// Created by Keren Dong on 2020/1/1.
//

#ifndef KUNGFU_NODE_IO_H
#define KUNGFU_NODE_IO_H

#include "common.h"
#include "journal.h"

#include <kungfu/yijinjing/io.h>

namespace kungfu::node {
yijinjing::data::location_ptr ExtractLocation(const Napi::CallbackInfo &info, int index,
                                              const yijinjing::data::locator_ptr &locator);

inline yijinjing::data::locator_ptr GetRuntimeLocator(const std::string &dirname) {
  return std::make_shared<yijinjing::data::locator>(dirname);
}

inline yijinjing::data::locator_ptr ExtractRuntimeLocatorByInfo0(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid Info[0] type, not string");
  }

  auto runtime_dir = info[0].As<Napi::String>().Utf8Value();
  return GetRuntimeLocator(runtime_dir);
}

class IODevice : public Napi::ObjectWrap<IODevice>, public yijinjing::io_device {
public:
  explicit IODevice(const Napi::CallbackInfo &info);

  Napi::Value OpenReader(const Napi::CallbackInfo &info);

  static yijinjing::data::locator_ptr GetLocator(const Napi::CallbackInfo &info, int index = 0);

  static yijinjing::data::location_ptr GetLocation(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

private:
  static Napi::FunctionReference constructor;
};
} // namespace kungfu::node

#endif // KUNGFU_NODE_IO_H
