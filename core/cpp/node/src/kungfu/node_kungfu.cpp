//
// Created by Keren Dong on 2019/12/25.
//

#include <kungfu/yijinjing/io.h>

#include "commission_store.h"
#include "config_store.h"
#include "history.h"
#include "io.h"
#include "journal.h"
#include "longfist.h"
#include "watcher.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::Value FormatTime(const Napi::CallbackInfo &info) {
  int64_t timestamp = 0;
  if (info[0].IsNumber()) {
    timestamp = info[0].ToNumber().Int32Value();
  }
  if (info[0].IsBigInt()) {
    bool lossless;
    timestamp = info[0].As<Napi::BigInt>().Int64Value(&lossless);
  }
  return Napi::String::New(info.Env(), time::strftime(timestamp));
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  ensure_sqlite_initilize();

  Longfist::Init(env, exports);
  History::Init(env, exports);
  ConfigStore::Init(env, exports);
  CommissionStore::Init(env, exports);
  Frame::Init(env, exports);
  Reader::Init(env, exports);
  IODevice::Init(env, exports);
  Session::Init(env, exports);
  Watcher::Init(env, exports);

  exports.Set("formatTime", Napi::Function::New(env, FormatTime));

  return exports;
}

NODE_API_MODULE(addon, InitAll)
} // namespace kungfu::node
