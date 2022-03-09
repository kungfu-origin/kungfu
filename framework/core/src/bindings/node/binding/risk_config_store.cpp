//
// Created by Keren Dong on 2020/2/25.
//

#include "risk_config_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference RiskConfigStore::constructor = {};

RiskConfigStore::RiskConfigStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(IODevice::GetLocator(info)), profile_(locator_) {}

inline RiskConfig getConfigFromJs(const Napi::CallbackInfo &info, const locator_ptr &locator) {
  RiskConfig query = {};
  auto config_location = ExtractLocation(info, 0, locator);
  if (config_location) {
    query.location_uid = config_location->uid;
    query.category = config_location->category;
    query.group = config_location->group;
    query.name = config_location->name;
    query.mode = config_location->mode;
  }
  return query;
}

Napi::Value RiskConfigStore::SetRiskConfig(const Napi::CallbackInfo &info) {
  RiskConfig risk_config = getConfigFromJs(info, locator_);
  if (info[0].IsObject()) {
    risk_config.max_order_volume = info[0].ToObject().Get("max_order_volume").ToNumber().Int32Value();
    risk_config.max_daily_volume = info[0].ToObject().Get("max_daily_volume").ToNumber().Int32Value();
    risk_config.white_list = info[0].ToObject().Get("white_list").ToString().Utf8Value();
    risk_config.self_filled_check = info[0].ToObject().Get("self_filled_check").ToBoolean().Value();
    risk_config.max_cancel_ratio = info[0].ToObject().Get("max_cancel_ratio").ToNumber().DoubleValue();
  } else {
    risk_config.max_order_volume = info[4].ToNumber().Int32Value();
    risk_config.max_daily_volume = info[5].ToNumber().Int32Value();
    risk_config.white_list = info[6].ToString().Utf8Value();
    risk_config.self_filled_check = info[7].ToBoolean().Value();
    risk_config.max_cancel_ratio = info[8].ToNumber().DoubleValue();
  }
  profile_.set(risk_config);
  return {};
}

Napi::Value RiskConfigStore::GetRiskConfig(const Napi::CallbackInfo &info) {
  auto result = Napi::Object::New(info.Env());
  auto risk_config = profile_.get(getConfigFromJs(info, locator_));
  set(risk_config, result);
  return result;
}

Napi::Value RiskConfigStore::GetAllRiskConfig(const Napi::CallbackInfo &info) {
  auto table = Napi::Object::New(info.Env());
  for (const auto &risk_config : profile_.get_all(RiskConfig{})) {
    auto uid = fmt::format("{:016x}", risk_config.uid());
    auto object = Napi::Object::New(info.Env());
    set(risk_config, object);
    table.Set(uid, object);
  }
  return table;
}

Napi::Value RiskConfigStore::RemoveRiskConfig(const Napi::CallbackInfo &info) {
  profile_.remove(profile_.get(getConfigFromJs(info, locator_)));
  return {};
}

void RiskConfigStore::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "RiskConfigStore",
                                    {
                                        InstanceMethod("setRiskConfig", &RiskConfigStore::SetRiskConfig),
                                        InstanceMethod("getRiskConfig", &RiskConfigStore::GetRiskConfig),
                                        InstanceMethod("getAllRiskConfig", &RiskConfigStore::GetAllRiskConfig),
                                        InstanceMethod("removeRiskConfig", &RiskConfigStore::RemoveRiskConfig),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("RiskConfigStore", func);
}

Napi::Value RiskConfigStore::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }
} // namespace kungfu::node
