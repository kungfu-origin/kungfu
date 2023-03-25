// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/3/30.
//

#include "operators.h"

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::practice;

namespace kungfu::node::serialize {
void InitStateMap(Napi::ObjectReference &state, const std::string &name) {
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    auto name = std::string(boost::hana::first(it).c_str());
    state.Set(name, DataTable::NewInstance(state.Value()));
  });
  state.Value().DefineProperty(Napi::PropertyDescriptor::Value("state_name", Napi::String::New(state.Env(), name)));
}

void InitTradingDataMap(Napi::ObjectReference &state, const std::string &name) {
  boost::hana::for_each(longfist::TradingDataTypes, [&](auto it) {
    auto name = std::string(boost::hana::first(it).c_str());
    state.Set(name, DataTable::NewInstance(state.Value()));
  });
  state.Value().DefineProperty(Napi::PropertyDescriptor::Value("state_name", Napi::String::New(state.Env(), name)));
}

} // namespace kungfu::node::serialize