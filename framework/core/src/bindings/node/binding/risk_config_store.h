//
// Created by Keren Dong on 2020/2/25.
//

#ifndef KUNGFU_NODE_RISKCONFIGURATION_H
#define KUNGFU_NODE_RISKCONFIGURATION_H

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/profile.h>

#include "common.h"
#include "operators.h"

namespace kungfu::node {
class RiskConfigStore : public Napi::ObjectWrap<RiskConfigStore> {
public:
  explicit RiskConfigStore(const Napi::CallbackInfo &info);

  ~RiskConfigStore() override = default;

  Napi::Value SetRiskConfig(const Napi::CallbackInfo &info);

  Napi::Value GetRiskConfig(const Napi::CallbackInfo &info);

  Napi::Value GetAllRiskConfig(const Napi::CallbackInfo &info);

  Napi::Value RemoveRiskConfig(const Napi::CallbackInfo &info);

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
