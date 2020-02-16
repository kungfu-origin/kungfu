//
// Created by Keren Dong on 2020/2/11.
//

#ifndef KUNGFU_NODE_SERIALIZE_H
#define KUNGFU_NODE_SERIALIZE_H

#include <napi.h>
#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::node::serialize
{
    template<typename DataType, const char *type_name>
    struct JsMake
    {
        Napi::Value operator()(const Napi::CallbackInfo &info)
        {
            Napi::Object dataObj = Napi::Object::New(info.Env());
            dataObj.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(info.Env(), type_name)));
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                using ValueType = std::decay_t<std::result_of_t<decltype(accessor)(const DataType &)>>;
                InitValue<ValueType>(dataObj, name.c_str());
            });
            return dataObj;
        }

        template<typename ValueType>
        std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, bool>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Boolean::New(obj.Env(), false));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_int_v<ValueType> or kungfu::is_unsigned_int_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_bigint_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::BigInt::New(obj.Env(), (int64_t) 0));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_unsigned_bigint_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::BigInt::New(obj.Env(), (uint64_t) 0));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_floating_point_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_v<ValueType, char>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::String::New(obj.Env(), ""));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length);
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_vector_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), 0);
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, std::string>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::String::New(obj.Env(), ""));
        }
    };

    struct JsSet
    {
        template<typename DataType>
        void operator()(const DataType &data, Napi::Value &value)
        {
            Napi::Object valueObj = value.ToObject();
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                Set(valueObj, name.c_str(), accessor(data));
            });
        }

        template<typename ValueType>
        std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>, void>
        Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), static_cast<int>(value)));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, bool>, void>
        Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Boolean::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_int_v<ValueType> or kungfu::is_unsigned_int_v<ValueType>, void>
        Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_bigint_v<ValueType> or kungfu::is_unsigned_bigint_v<ValueType>, void>
        Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::BigInt::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_floating_point_v<ValueType>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_v<ValueType, char>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::String::New(obj.Env(), value.value));
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            using ElementType = typename ValueType::element_type;
            size_t element_size = sizeof(ElementType);
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length * element_size);
            for (int i = 0; i < ValueType::length; i++)
            {
                auto addr = reinterpret_cast<ElementType*>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * element_size);
                *addr = value[i];
            }
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        void Set(Napi::Object &obj, const char *name, const std::vector<ValueType> &value)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), value.size() * sizeof(ValueType));
            for (int i = 0; i < value.size(); i++)
            {
                auto addr = reinterpret_cast<ValueType*>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * sizeof(ValueType));
                *addr = value[i];
            }
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, std::string>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::String::New(obj.Env(), value));
        }
    };


    struct JsGet
    {
        template<typename DataType>
        void operator()(const Napi::Value &value, DataType &data)
        {
            Napi::Object valueObj = value.ToObject();
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                using ValueType = std::decay_t<std::result_of_t<decltype(accessor)(const DataType &)>>;
                Get(const_cast<ValueType &>(accessor(data)), name.c_str(), valueObj);
            });
        }

        template<typename ValueType>
        std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = static_cast<ValueType>(obj.Get(name).ToNumber().Int32Value());
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, bool>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToBoolean().Value();
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_int_v<ValueType>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToNumber().Int32Value();
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_unsigned_int_v<ValueType>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToNumber().Uint32Value();
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_signed_bigint_v<ValueType>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            bool lossless;
            value = obj.Get(name).As<Napi::BigInt>().Int64Value(&lossless);
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_unsigned_bigint_v<ValueType>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            bool lossless;
            value = obj.Get(name).As<Napi::BigInt>().Uint64Value(&lossless);
        }

        template<typename ValueType>
        std::enable_if_t<std::is_floating_point_v<ValueType>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToNumber().DoubleValue();
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_v<ValueType, char>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            auto v = obj.Get(name).ToString().Utf8Value();
            strcpy(value.value, v.c_str());
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>, void>
        Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            auto buf = obj.Get(name).As<Napi::ArrayBuffer>();
            memcpy(value.value, buf.Data(), buf.ByteLength());
        }

        template<typename ValueType>
        void Get(std::vector<ValueType> &value, const char *name, const Napi::Object &obj)
        {
            auto buf = obj.Get(name).As<Napi::ArrayBuffer>();
            for (int i = 0; i < buf.ByteLength(); i += sizeof(ValueType))
            {
                auto addr = reinterpret_cast<uintptr_t>(buf.Data()) + i;
                value.push_back(*reinterpret_cast<ValueType*>(addr));
            }
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, std::string>, void> Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToString().Utf8Value();
        }
    };

    struct JsUpdateState
    {
    private:
        JsSet set;
        Napi::ObjectReference &state_;

    public:
        explicit JsUpdateState(Napi::ObjectReference &state) : state_(state)
        {};

        template<typename DataType>
        void operator()(const std::string &name, boost::hana::basic_type<DataType> type, const event_ptr &event)
        {
            auto data = event->data<DataType>();
            Napi::Object table = state_.Get(name).ToObject();
            auto key = fmt::format("{:016x}", data.uid());
            Napi::Value value = state_.Get(key);
            if (value.IsUndefined() or value.IsEmpty())
            {
                value = Napi::Object::New(state_.Env());
                table.Set(key, value);
            }
            set(data, value);
        }
    };

    struct JsPublishState
    {
    private:
        JsGet get;
        yijinjing::practice::apprentice &app_;

    public:
        explicit JsPublishState(yijinjing::practice::apprentice &app) : app_(app)
        {}

        template<typename DataType>
        void operator()(const std::string &type_name, boost::hana::basic_type<DataType> type, Napi::Value &value)
        {
            DataType result{};
            get(value, result);
            SPDLOG_WARN("publish {}", type_name, result.to_string());
            app_.write_to(0, DataType::tag, result);
        }
    };
}

#endif //KUNGFU_NODE_SERIALIZE_H
