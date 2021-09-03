//
// Created by Keren Dong on 2019/12/25.
//

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/util/util.h>

#include "commission_store.h"
#include "config_store.h"
#include "data_table.h"
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
using namespace kungfu::node;

namespace kungfu::node {

Napi::Value FormatTime(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsBigInt)) {
    return {};
  }
  auto format = IsValid(info, 1, &Napi::Value::IsString) ? info[1].ToString().Utf8Value() : KUNGFU_DATETIME_FORMAT;
  return Napi::String::New(info.Env(), time::strftime(GetBigInt(info, 0), format));
}

Napi::Value FormatStringToHashHex(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid argument");
  }
  auto arg = info[0].ToString().Utf8Value();
  uint32_t hash = hash_32((const unsigned char *)(arg.c_str()), arg.length());
  return Napi::String::New(info.Env(), fmt::format("{:08x}", hash));
}

Napi::Value ParseTime(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString) and IsValid(info, 1, &Napi::Value::IsString)) {
    return Napi::BigInt::New(info.Env(), TryParseTime(info, 0));
  }
  auto time_string = info[0].ToString().Utf8Value();
  auto format = info[1].ToString().Utf8Value();
  return Napi::BigInt::New(info.Env(), time::strptime(time_string, format));
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  ensure_sqlite_initilize();

  Longfist::Init(env, exports);
  History::Init(env, exports);
  ConfigStore::Init(env, exports);
  CommissionStore::Init(env, exports);
  Frame::Init(env, exports);
  Reader::Init(env, exports);
  Assemble::Init(env, exports);
  IODevice::Init(env, exports);
  DataTable::Init(env, exports);
  Watcher::Init(env, exports);

  exports.Set("formatTime", Napi::Function::New(env, FormatTime));
  exports.Set("formatStringToHashHex", Napi::Function::New(env, FormatStringToHashHex));
  exports.Set("parseTime", Napi::Function::New(env, ParseTime));

  return exports;
}
} // namespace kungfu::node

NODE_API_MODULE(addon, InitAll)