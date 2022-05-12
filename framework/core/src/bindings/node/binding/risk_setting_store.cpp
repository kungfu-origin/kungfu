//
// Created by Keren Dong on 2020/2/25.
//

#include "risk_setting_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference RiskSettingStore::constructor = {};

RiskSettingStore::RiskSettingStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(ExtractRuntimeLocatorByInfo0(info)), profile_(locator_) {}

inline RiskSetting getConfigFromJs(const Napi::CallbackInfo &info, const locator_ptr &locator) {
  RiskSetting query = {};
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

Napi::Value RiskSettingStore::SetRiskSetting(const Napi::CallbackInfo &info) {
  RiskSetting risk_setting = getConfigFromJs(info, locator_);
  int valueIndex = info[0].IsObject() ? 1 : 4;
  if (info[0].IsObject()) {
    risk_setting.value = info[0].ToObject().Get("value").ToString().Utf8Value();
  } else {
    risk_setting.value = info[valueIndex].ToString().Utf8Value();
  }
  profile_.set(risk_setting);
  return {};
}

Napi::Value RiskSettingStore::GetRiskSetting(const Napi::CallbackInfo &info) {
  auto result = Napi::Object::New(info.Env());
  auto risk_setting = profile_.get(getConfigFromJs(info, locator_));
  set(risk_setting, result);
  return result;
}

Napi::Value RiskSettingStore::SetAllRiskSetting(const Napi::CallbackInfo &info) {
  try {
    if (info[0].IsArray()) {
      auto args = info[0].As<Napi::Array>();
      std::vector<RiskSetting> risk_settings;
      for (int i = 0; i < args.Length(); i++) {
        RiskSetting risk_setting = {};
        get(args.Get(i).ToObject(), risk_setting);
        risk_settings.push_back(risk_setting);
      }
      profile_.remove_all<RiskSetting>();
      for (auto risk_setting : risk_settings) {
        profile_.set(risk_setting);
      }
      return Napi::Boolean::New(info.Env(), true);
    }
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to set risk_settings {}", ex.what());
  }
  return Napi::Boolean::New(info.Env(), false);
}

Napi::Value RiskSettingStore::GetAllRiskSetting(const Napi::CallbackInfo &info) {
  auto table = Napi::Object::New(info.Env());
  for (const auto &risk_setting : profile_.get_all(RiskSetting{})) {
    auto uid = fmt::format("{:016x}", risk_setting.uid());
    auto object = Napi::Object::New(info.Env());
    set(risk_setting, object);
    table.Set(uid, object);
  }
  return table;
}

Napi::Value RiskSettingStore::RemoveRiskSetting(const Napi::CallbackInfo &info) {
  profile_.remove(profile_.get(getConfigFromJs(info, locator_)));
  return {};
}

void RiskSettingStore::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "RiskSettingStore",
                                    {
                                        InstanceMethod("setRiskSetting", &RiskSettingStore::SetRiskSetting),
                                        InstanceMethod("getRiskSetting", &RiskSettingStore::GetRiskSetting),
                                        InstanceMethod("getAllRiskSetting", &RiskSettingStore::GetAllRiskSetting),
                                        InstanceMethod("removeRiskSetting", &RiskSettingStore::RemoveRiskSetting),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("RiskSettingStore", func);
}

Napi::Value RiskSettingStore::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }
} // namespace kungfu::node
