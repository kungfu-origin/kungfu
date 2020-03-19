//
// Created by Keren Dong on 2020/1/1.
//

#include "journal.h"
#include "io.h"

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
int64_t GetTimestamp(Napi::Value arg) {
  if (arg.IsNumber()) {
    return arg.ToNumber().Int32Value();
  }
  if (arg.IsBigInt()) {
    bool lossless;
    return arg.As<Napi::BigInt>().Int64Value(&lossless);
  }
  throw yijinjing_error("timestamp argument must be bigint");
}

Napi::FunctionReference Frame::constructor;

Frame::Frame(const Napi::CallbackInfo &info) : ObjectWrap(info) {}

void Frame::SetFrame(yijinjing::journal::frame_ptr frame) { frame_ = std::move(frame); }

Napi::Value Frame::DataLength(const Napi::CallbackInfo &info) {
  return Napi::Number::New(info.Env(), frame_->data_length());
}

Napi::Value Frame::GenTime(const Napi::CallbackInfo &info) { return Napi::BigInt::New(info.Env(), frame_->gen_time()); }

Napi::Value Frame::TriggerTime(const Napi::CallbackInfo &info) {
  return Napi::BigInt::New(info.Env(), frame_->trigger_time());
}

Napi::Value Frame::MsgType(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), frame_->msg_type()); }

Napi::Value Frame::Source(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), frame_->source()); }

Napi::Value Frame::Dest(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), frame_->dest()); }

Napi::Value Frame::ToString(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), "Frame.js"); }

void Frame::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Frame",
                  {InstanceMethod("dataLength", &Frame::DataLength), InstanceMethod("genTime", &Frame::GenTime),
                   InstanceMethod("triggerTime", &Frame::TriggerTime), InstanceMethod("msgType", &Frame::MsgType),
                   InstanceMethod("source", &Frame::Source), InstanceMethod("dest", &Frame::Dest),
                   InstanceMethod("toString", &Frame::ToString)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Frame", func);
}

Napi::Value Frame::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }

Napi::FunctionReference Reader::constructor;

Reader::Reader(const Napi::CallbackInfo &info)
    : ObjectWrap(info), reader(true),
      io_device_(reinterpret_cast<IODevice *>(Napi::ObjectWrap<IODevice>::Unwrap(info[0].As<Napi::Object>()))) {}

Napi::Value Reader::ToString(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), "Reader.js"); }

Napi::Value Reader::CurrentFrame(const Napi::CallbackInfo &info) {
  auto frame = Frame::NewInstance(info.This());
  Napi::ObjectWrap<Frame>::Unwrap(frame.As<Napi::Object>())->SetFrame(current_frame());
  return frame;
}

Napi::Value Reader::SeekToTime(const Napi::CallbackInfo &info) {
  seek_to_time(GetTimestamp(info[0]));
  return Napi::Value();
}

Napi::Value Reader::DataAvailable(const Napi::CallbackInfo &info) {
  return Napi::Boolean::New(info.Env(), data_available());
}

Napi::Value Reader::Next(const Napi::CallbackInfo &info) {
  next();
  return Napi::Value();
}

Napi::Value Reader::Join(const Napi::CallbackInfo &info) {
  auto category = longfist::enums::get_category_by_name(info[0].As<Napi::String>().Utf8Value());
  auto group = info[1].As<Napi::String>().Utf8Value();
  auto name = info[2].As<Napi::String>().Utf8Value();
  auto mode = longfist::enums::get_mode_by_name(info[3].As<Napi::String>().Utf8Value());
  uint32_t dest_id = info[4].As<Napi::Number>().Int32Value();
  auto from_time = GetTimestamp(info[5]);
  join(std::make_shared<location>(mode, category, group, name, io_device_->get_home()->locator), dest_id, from_time);
  return Napi::Value();
}

Napi::Value Reader::Disjoin(const Napi::CallbackInfo &info) {
  uint32_t dest_id = info[0].As<Napi::Number>().Int32Value();
  disjoin(dest_id);
  return Napi::Value();
}

void Reader::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Reader",
                                    {
                                        InstanceMethod("toString", &Reader::ToString),
                                        InstanceMethod("currentFrame", &Reader::CurrentFrame),
                                        InstanceMethod("seekToTime", &Reader::SeekToTime),
                                        InstanceMethod("dataAvailable", &Reader::DataAvailable),
                                        InstanceMethod("next", &Reader::Next),
                                        InstanceMethod("join", &Reader::Join),
                                        InstanceMethod("disjoin", &Reader::Disjoin),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Reader", func);
}

Napi::Value Reader::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }
} // namespace kungfu::node
