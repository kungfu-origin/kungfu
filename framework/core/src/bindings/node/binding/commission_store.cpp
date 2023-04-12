// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/3/16.
//

#include "commission_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference CommissionStore::constructor = {};

CommissionStore::CommissionStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(ExtractRuntimeLocatorByInfo0(info)), profile_(locator_) {}

Napi::Value CommissionStore::SetCommission(const Napi::CallbackInfo &info) {
  try {
    profile_.set(ExtractCommission(info));
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to SetCommission {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value CommissionStore::GetCommission(const Napi::CallbackInfo &info) {
  auto result = Napi::Object::New(info.Env());
  try {
    auto commission = profile_.get(ExtractCommission(info));
    set(commission, result);
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to GetCommission {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }

  return result;
}

Napi::Value CommissionStore::SetAllCommission(const Napi::CallbackInfo &info) {
  if (info[0].IsArray()) {
    auto args = info[0].As<Napi::Array>();
    std::vector<Commission> commissions;
    for (int i = 0; i < args.Length(); i++) {
      Commission commission = {};
      get(args.Get(i).ToObject(), commission);
      commissions.push_back(commission);
    }
    try {
      profile_.remove_all<Commission>();
      for (auto commission : commissions) {
        profile_.set(commission);
      }
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("failed to SetAllCommission {}", ex.what());
      yijinjing::util::print_stack_trace();
      return Napi::Boolean::New(info.Env(), false);
    }
    return Napi::Boolean::New(info.Env(), true);
  }
  return Napi::Boolean::New(info.Env(), false);
}

Napi::Value CommissionStore::GetAllCommission(const Napi::CallbackInfo &info) {
  try {
    auto commissions = profile_.get_all(Commission{});
    auto result = Napi::Array::New(info.Env(), commissions.size());
    for (int i = 0; i < commissions.size(); i++) {
      auto target = Napi::Object::New(info.Env());
      set(commissions[i], target);
      result.Set(i, target);
    }
    return result;
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to GetAllCommission {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }
}

Napi::Value CommissionStore::RemoveCommission(const Napi::CallbackInfo &info) {
  try {
    profile_.remove(profile_.get(ExtractCommission(info)));
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to GetAllCommission {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }
  return Napi::Boolean::New(info.Env(), true);
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

Commission CommissionStore::ExtractCommission(const Napi::CallbackInfo &info) {
  Commission commission = {};
  try {
    if (info[0].IsString()) {
      strcpy(commission.product_id, info[0].ToString().Utf8Value().c_str());
      strcpy(commission.exchange_id, info[1].ToString().Utf8Value().c_str());
    }
    if (info[0].IsObject()) {
      get(info[0].ToObject(), commission);
    }
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("invalid argument commission: {}", ex.what());
  }
  return commission;
}
} // namespace kungfu::node
