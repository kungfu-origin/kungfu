//
// Created by Keren Dong on 2020/1/1.
//

#ifndef KUNGFU_NODE_JOURNAL_H
#define KUNGFU_NODE_JOURNAL_H

#include <napi.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::node {
class Reader;

class Frame : public Napi::ObjectWrap<Frame> {
public:
  explicit Frame(const Napi::CallbackInfo &info);

  Napi::Value DataLength(const Napi::CallbackInfo &info);

  Napi::Value GenTime(const Napi::CallbackInfo &info);

  Napi::Value TriggerTime(const Napi::CallbackInfo &info);

  Napi::Value MsgType(const Napi::CallbackInfo &info);

  Napi::Value Source(const Napi::CallbackInfo &info);

  Napi::Value Dest(const Napi::CallbackInfo &info);

  Napi::Value ToString(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

  static Napi::Value NewInstance(Napi::Value arg);

private:
  yijinjing::journal::frame_ptr frame_;
  static Napi::FunctionReference constructor;

  void SetFrame(yijinjing::journal::frame_ptr frame);

  friend class Reader;
};

class Reader : public Napi::ObjectWrap<Reader>, public yijinjing::journal::reader {
public:
  explicit Reader(const Napi::CallbackInfo &info);

  Napi::Value ToString(const Napi::CallbackInfo &info);

  Napi::Value CurrentFrame(const Napi::CallbackInfo &info);

  Napi::Value SeekToTime(const Napi::CallbackInfo &info);

  Napi::Value DataAvailable(const Napi::CallbackInfo &info);

  Napi::Value Next(const Napi::CallbackInfo &info);

  Napi::Value Join(const Napi::CallbackInfo &info);

  Napi::Value Disjoin(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

  static Napi::Value NewInstance(Napi::Value arg);

private:
  yijinjing::io_device_ptr io_device_;
  static Napi::FunctionReference constructor;
};
} // namespace kungfu::node
#endif // KUNGFU_NODE_JOURNAL_H
