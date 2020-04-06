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
JsRestoreState::JsRestoreState(Napi::ObjectReference &state, location_ptr location)
    : state_(state), location_(std::move(location)) {}

void JsRestoreState::operator()(int64_t from, int64_t to) {
  auto source = location_->uid;
  auto locator = location_->locator;
  auto now = time::now_in_nano();
  for (auto dest : locator->list_location_dest(location_)) {
    auto db_file = locator->layout_file(location_, layout::SQLITE, fmt::format("{:08x}", dest));
    auto storage = cache::make_storage(db_file, longfist::StateDataTypes);
    if (not storage.sync_schema_simulate().empty()) {
      storage.sync_schema();
    }
    boost::hana::for_each(StateDataTypes, [&](auto it) {
      using DataType = typename decltype(+boost::hana::second(it))::type;
      auto type_name = boost::hana::first(it).c_str();
      for (auto &data : cache::time_spec<DataType>::get_all(storage, from, to)) {
        Napi::Object table = state_.Get(type_name).ToObject();
        std::string uid_key = fmt::format("{:016x}", data.uid());
        Napi::Value value = state_.Get(uid_key);
        if (value.IsUndefined() or value.IsEmpty()) {
          value = Napi::Object::New(state_.Env());
          auto vo = value.ToObject();
          vo.DefineProperty(Napi::PropertyDescriptor::Value("tag", Napi::Number::New(value.Env(), DataType::tag)));
          vo.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
          vo.DefineProperty(Napi::PropertyDescriptor::Value("uid_key", Napi::String::New(value.Env(), uid_key)));
          vo.DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), source)));
          vo.DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), dest)));
          vo.DefineProperty(Napi::PropertyDescriptor::Value("ts", Napi::BigInt::New(value.Env(), now)));
          table.Set(uid_key, value);
        }
        set(data, value);
      }
    });
  }
}

JsPublishState::JsPublishState(apprentice &app, Napi::ObjectReference &state) : app_(app), state_(state) {}

void JsPublishState::operator()(const Napi::Value &value) {
  Napi::Object vo = value.ToObject();
  auto type_name = vo.Get("type").ToString().Utf8Value();
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (strcmp(DataType::type_name.c_str(), type_name.c_str()) == 0) {
      DataType data{};
      get(value, data);
      auto now = yijinjing::time::now_in_nano();
      auto location = app_.get_io_device()->get_home();
      auto uid_key = fmt::format("{:016x}", data.uid());
      vo.DefineProperty(Napi::PropertyDescriptor::Value("tag", Napi::Number::New(value.Env(), DataType::tag)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("uid_key", Napi::String::New(value.Env(), uid_key)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), location->uid)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), 0)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("ts", Napi::BigInt::New(value.Env(), now)));
      state_.Get(type_name).ToObject().Set(uid_key, vo);
      app_.write_to(0, data);
    }
  });
}

JsResetCache::JsResetCache(apprentice &app, Napi::ObjectReference &state) : app_(app), state_(state) {}

void JsResetCache::operator()(const event_ptr &event) {
  auto request = event->data<CacheReset>();
  boost::hana::for_each(StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (DataType::tag == request.msg_type) {
      auto type_name = DataType::type_name.c_str();
      auto table = state_.Get(type_name).ToObject();
      auto names = table.GetPropertyNames();
      std::vector<std::string> delete_keys = {};
      for (int i = 0; i < names.Length(); i++) {
        auto name = names.Get(i).ToString().Utf8Value();
        auto value = table.Get(name).ToObject();
        auto source = value.Get("source").ToNumber().Uint32Value();
        auto dest = value.Get("dest").ToNumber().Uint32Value();
        if (source == event->source() and dest == event->dest()) {
          delete_keys.push_back(name);
        }
        if (source == event->dest() and dest == event->source()) {
          delete_keys.push_back(name);
        }
      }
      for (const auto &key : delete_keys) {
        table.Delete(key);
      }
    }
  });
}

void InitStateMap(const Napi::CallbackInfo &info, Napi::ObjectReference &ref) {
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    auto name = std::string(boost::hana::first(it).c_str());
    ref.Set(name, Napi::Object::New(info.Env()));
  });
}
} // namespace kungfu::node::serialize