//
// Created by Keren Dong on 2020/2/11.
//

#ifndef KUNGFU_NODE_SERIALIZE_H
#define KUNGFU_NODE_SERIALIZE_H

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/cache/backend.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/time.h>
#include <napi.h>

namespace kungfu::node::serialize {
template <typename DataType> class JsMake {

public:
  explicit JsMake(const char *_type_name) : type_name(_type_name) {}

  Napi::Value operator()(const Napi::CallbackInfo &info) {
    Napi::Object dataObj = Napi::Object::New(info.Env());
    dataObj.DefineProperty(Napi::PropertyDescriptor::Value("tag", Napi::Number::New(info.Env(), DataType::tag)));
    dataObj.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(info.Env(), type_name)));
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&](auto it) {
      auto name = boost::hana::first(it);
      auto accessor = boost::hana::second(it);
      using ValueType = std::decay_t<std::result_of_t<decltype(accessor)(const DataType &)>>;
      InitValue<ValueType>(dataObj, name.c_str());
    });
    return dataObj;
  }

private:
  const char *type_name;

  template <typename ValueType>
  std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>>
  InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::Number::New(obj.Env(), 0));
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, bool>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::Boolean::New(obj.Env(), false));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_int_v<ValueType> or kungfu::is_unsigned_int_v<ValueType>>
  InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::Number::New(obj.Env(), 0));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_bigint_v<ValueType>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::BigInt::New(obj.Env(), (int64_t)0));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_unsigned_bigint_v<ValueType>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::BigInt::New(obj.Env(), (uint64_t)0));
  }

  template <typename ValueType>
  std::enable_if_t<std::is_floating_point_v<ValueType>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::Number::New(obj.Env(), 0));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_v<ValueType, char>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::String::New(obj.Env(), ""));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>> InitValue(Napi::Object &obj, const char *name) {
    auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length);
    obj.Set(name, buffer);
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_vector_v<ValueType>> InitValue(Napi::Object &obj, const char *name) {
    auto buffer = Napi::ArrayBuffer::New(obj.Env(), 0);
    obj.Set(name, buffer);
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, std::string>> InitValue(Napi::Object &obj, const char *name) {
    obj.Set(name, Napi::String::New(obj.Env(), ""));
  }
};

class JsSet {
public:
  template <typename DataType> void operator()(const DataType &data, Napi::Value &value) {
    Napi::Object valueObj = value.ToObject();
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it) {
      auto name = boost::hana::first(it);
      auto accessor = boost::hana::second(it);
      Set(valueObj, name.c_str(), accessor(data));
    });
  }

private:
  template <typename ValueType>
  std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>>
  Set(Napi::Object &obj, const char *name, const ValueType &value) {
    obj.Set(name, Napi::Number::New(obj.Env(), static_cast<int>(value)));
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, bool>> Set(Napi::Object &obj, const char *name, const ValueType &value) {
    obj.Set(name, Napi::Boolean::New(obj.Env(), value));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_int_v<ValueType> or kungfu::is_unsigned_int_v<ValueType>>
  Set(Napi::Object &obj, const char *name, const ValueType &value) {
    obj.Set(name, Napi::Number::New(obj.Env(), value));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_bigint_v<ValueType> or kungfu::is_unsigned_bigint_v<ValueType>>
  Set(Napi::Object &obj, const char *name, const ValueType &value) {
    obj.Set(name, Napi::BigInt::New(obj.Env(), value));
  }

  template <typename ValueType>
  std::enable_if_t<std::is_floating_point_v<ValueType>> Set(Napi::Object &obj, const char *name,
                                                            const ValueType &value) {
    obj.Set(name, Napi::Number::New(obj.Env(), value));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_v<ValueType, char>> Set(Napi::Object &obj, const char *name,
                                                               const ValueType &value) {
    obj.Set(name, Napi::String::New(obj.Env(), value.value));
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>> Set(Napi::Object &obj, const char *name,
                                                                      const ValueType &value) {
    using ElementType = typename ValueType::element_type;
    size_t element_size = sizeof(ElementType);
    auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length * element_size);
    for (int i = 0; i < ValueType::length; i++) {
      auto addr = reinterpret_cast<ElementType *>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * element_size);
      *addr = value[i];
    }
    obj.Set(name, buffer);
  }

  template <typename ValueType> void Set(Napi::Object &obj, const char *name, const std::vector<ValueType> &value) {
    auto buffer = Napi::ArrayBuffer::New(obj.Env(), value.size() * sizeof(ValueType));
    for (int i = 0; i < value.size(); i++) {
      auto addr = reinterpret_cast<ValueType *>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * sizeof(ValueType));
      *addr = value[i];
    }
    obj.Set(name, buffer);
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, std::string>> Set(Napi::Object &obj, const char *name,
                                                               const ValueType &value) {
    obj.Set(name, Napi::String::New(obj.Env(), value));
  }
};

class JsGet {
public:
  template <typename DataType> void operator()(const Napi::Value &value, DataType &data) {
    Napi::Object valueObj = value.ToObject();
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it) {
      auto name = boost::hana::first(it);
      auto accessor = boost::hana::second(it);
      using ValueType = std::decay_t<std::result_of_t<decltype(accessor)(const DataType &)>>;
      Get(const_cast<ValueType &>(accessor(data)), name.c_str(), valueObj);
    });
    SPDLOG_TRACE("got data from node: {}", data.to_string());
  }

private:
  template <typename ValueType>
  std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>>
  Get(ValueType &value, const char *name, const Napi::Object &obj) {
    value = static_cast<ValueType>(obj.Get(name).ToNumber().Int32Value());
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, bool>> Get(ValueType &value, const char *name, const Napi::Object &obj) {
    value = obj.Get(name).ToBoolean().Value();
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_int_v<ValueType>> Get(ValueType &value, const char *name,
                                                           const Napi::Object &obj) {
    value = obj.Get(name).ToNumber().Int32Value();
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_unsigned_int_v<ValueType>> Get(ValueType &value, const char *name,
                                                             const Napi::Object &obj) {
    value = obj.Get(name).ToNumber().Uint32Value();
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_signed_bigint_v<ValueType>> Get(ValueType &value, const char *name,
                                                              const Napi::Object &obj) {
    if (obj.Get(name).IsNumber()) {
      value = obj.Get(name).ToNumber().Int32Value();
    }
    if (obj.Get(name).IsBigInt()) {
      bool lossless;
      value = obj.Get(name).As<Napi::BigInt>().Int64Value(&lossless);
    }
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_unsigned_bigint_v<ValueType>> Get(ValueType &value, const char *name,
                                                                const Napi::Object &obj) {
    if (obj.Get(name).IsNumber()) {
      value = obj.Get(name).ToNumber().Uint32Value();
    }
    if (obj.Get(name).IsBigInt()) {
      bool lossless;
      value = obj.Get(name).As<Napi::BigInt>().Uint64Value(&lossless);
    }
  }

  template <typename ValueType>
  std::enable_if_t<std::is_floating_point_v<ValueType>> Get(ValueType &value, const char *name,
                                                            const Napi::Object &obj) {
    value = obj.Get(name).ToNumber().DoubleValue();
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_v<ValueType, char>> Get(ValueType &value, const char *name,
                                                               const Napi::Object &obj) {
    auto v = obj.Get(name).ToString().Utf8Value();
    strcpy(value.value, v.c_str());
  }

  template <typename ValueType>
  std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>> Get(ValueType &value, const char *name,
                                                                      const Napi::Object &obj) {
    auto buf = obj.Get(name).As<Napi::ArrayBuffer>();
    memcpy(value.value, buf.Data(), buf.ByteLength());
  }

  template <typename ValueType> void Get(std::vector<ValueType> &value, const char *name, const Napi::Object &obj) {
    auto buf = obj.Get(name).As<Napi::ArrayBuffer>();
    for (int i = 0; i < buf.ByteLength(); i += sizeof(ValueType)) {
      auto addr = reinterpret_cast<uintptr_t>(buf.Data()) + i;
      value.push_back(*reinterpret_cast<ValueType *>(addr));
    }
  }

  template <typename ValueType>
  std::enable_if_t<std::is_same_v<ValueType, std::string>> Get(ValueType &value, const char *name,
                                                               const Napi::Object &obj) {
    value = obj.Get(name).ToString().Utf8Value();
  }
};

class JsRestoreState {
public:
  JsRestoreState(Napi::ObjectReference &state, yijinjing::data::location_ptr location);

  void operator()(int64_t from, int64_t to);

private:
  JsSet set;
  Napi::ObjectReference &state_;
  yijinjing::data::location_ptr location_;
};

class JsUpdateState {
public:
  explicit JsUpdateState(Napi::ObjectReference &state) : state_(state){};

  template <typename DataType>
  void operator()(int64_t update_time, uint32_t source, uint32_t dest, const DataType &data) {
    auto type_name = DataType::type_name.c_str();
    auto table = state_.Get(type_name).ToObject();
    auto uid = fmt::format("{:016x}", data.uid());
    auto value = state_.Get(uid);
    if (value.IsUndefined() or value.IsEmpty()) {
      value = Napi::Object::New(state_.Env());
      auto vo = value.ToObject();
      vo.DefineProperty(Napi::PropertyDescriptor::Value("tag", Napi::Number::New(value.Env(), DataType::tag)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("uid_key", Napi::String::New(value.Env(), uid)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), source)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), dest)));
      vo.DefineProperty(Napi::PropertyDescriptor::Value("ts", Napi::BigInt::New(value.Env(), update_time)));
      table.Set(uid, value);
    }
    set(data, value);
  }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    operator()(event->gen_time(), event->source(), event->dest(), event->template data<DataType>());
  }

private:
  JsSet set;
  Napi::ObjectReference &state_;
};

class JsPublishState {
public:
  JsPublishState(yijinjing::practice::apprentice &app, Napi::ObjectReference &state);

  void operator()(const Napi::Value &value);

private:
  JsGet get;
  yijinjing::practice::apprentice &app_;
  Napi::ObjectReference &state_;
};

class JsResetCache {
public:
  JsResetCache(yijinjing::practice::apprentice &app, Napi::ObjectReference &state);

  void operator()(const event_ptr &event);

private:
  yijinjing::practice::apprentice &app_;
  Napi::ObjectReference &state_;
};

void InitStateMap(const Napi::CallbackInfo &info, Napi::ObjectReference &ref);
} // namespace kungfu::node::serialize

#endif // KUNGFU_NODE_SERIALIZE_H
