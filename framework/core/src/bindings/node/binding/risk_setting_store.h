//
// Created by Keren Dong on 2020/2/25.
//

#ifndef KUNGFU_NODE_RISKSETTING_H
#define KUNGFU_NODE_RISKSETTING_H

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/profile.h>

#include "common.h"
#include "operators.h"

namespace kungfu::node {
class RiskSettingStore : public Napi::ObjectWrap<RiskSettingStore> {
public:
  explicit RiskSettingStore(const Napi::CallbackInfo &info);

  ~RiskSettingStore() override = default;

  Napi::Value SetRiskSetting(const Napi::CallbackInfo &info);

  Napi::Value GetRiskSetting(const Napi::CallbackInfo &info);

  Napi::Value GetAllRiskSetting(const Napi::CallbackInfo &info);

  Napi::Value RemoveRiskSetting(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

  static Napi::Value NewInstance(Napi::Value arg);

private:
  serialize::JsSet set;
  yijinjing::data::locator_ptr locator_;
  yijinjing::practice::profile profile_;

  static Napi::FunctionReference constructor;

  friend class Watcher;
};
} // namespace kungfu::node

#endif // KUNGFU_NODE_CONFIGURATION_H
