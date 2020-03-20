//
// Created by Keren Dong on 2020/3/16.
//

#include "commission_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference CommissionStore::constructor;

CommissionStore::CommissionStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(IODevice::GetLocator(info)), profile_(locator_) {}

inline Commission getCommissionFromJs(const Napi::CallbackInfo &info, const locator_ptr &locator) {
  Commission commission = {};
  try {
    if (info[0].IsString()) {
      strcpy(commission.instrument_id, info[0].ToString().Utf8Value().c_str());
      strcpy(commission.exchange_id, info[1].ToString().Utf8Value().c_str());
    }
    if (info[0].IsObject()) {
      serialize::JsGet{}(info[0], commission);
    }
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("invalid argument commission: {}", ex.what());
  }
  return commission;
}

Napi::Value CommissionStore::SetCommission(const Napi::CallbackInfo &info) {
  profile_.set(getCommissionFromJs(info, locator_));
  return Napi::Value();
}

Napi::Value CommissionStore::GetCommission(const Napi::CallbackInfo &info) {
  auto result = Napi::Object::New(info.Env());
  auto commission = profile_.get(getCommissionFromJs(info, locator_));
  set(commission, result);
  return result;
}

Napi::Value CommissionStore::SetAllCommission(const Napi::CallbackInfo &info) {
  try {
    if (info[0].IsArray()) {
      auto args = info[0].As<Napi::Array>();
      std::vector<Commission> commissions;
      for (int i = 0; i < args.Length(); i++) {
        Commission commission = {};
        serialize::JsGet{}(args.Get(i), commission);
        SPDLOG_INFO("got commission from node: {}", commission.to_string());
        commissions.push_back(commission);
      }
      profile_.remove_all<Commission>();
      for (auto commission : commissions) {
        profile_.set(commission);
      }
      return Napi::Boolean::New(info.Env(), true);
    }
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to set commissions {}", ex.what());
  }
  return Napi::Boolean::New(info.Env(), false);
}

Napi::Value CommissionStore::GetAllCommission(const Napi::CallbackInfo &info) {
  auto commissions = profile_.get_all(Commission{});
  auto result = Napi::Array::New(info.Env(), commissions.size());
  for (int i = 0; i < commissions.size(); i++) {
    auto target = Napi::Object::New(info.Env());
    set(commissions[i], target);
    result.Set(i, target);
  }
  return result;
}

Napi::Value CommissionStore::RemoveCommission(const Napi::CallbackInfo &info) {
  profile_.remove(profile_.get(getCommissionFromJs(info, locator_)));
  return Napi::Value();
}

void CommissionStore::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "CommissionStore",
                                    {
                                        InstanceMethod("setCommission", &CommissionStore::SetCommission),
                                        InstanceMethod("getCommission", &CommissionStore::GetCommission),
                                        InstanceMethod("setAllCommission", &CommissionStore::SetAllCommission),
                                        InstanceMethod("getAllCommission", &CommissionStore::GetAllCommission),
                                        InstanceMethod("removeCommission", &CommissionStore::RemoveCommission),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CommissionStore", func);
}

Napi::Value CommissionStore::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }
} // namespace kungfu::node
