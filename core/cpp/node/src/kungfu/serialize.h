//
// Created by Keren Dong on 2020/2/11.
//

#ifndef KUNGFU_NODE_SERIALIZE_H
#define KUNGFU_NODE_SERIALIZE_H

namespace kungfu::node::serialize
{
    struct ToJs
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
        std::enable_if_t<std::is_same_v<ValueType, bool>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Boolean::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_integral_v<ValueType> and (sizeof(ValueType) <= 4) and not std::is_same_v<ValueType, bool>, void>
        Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), value));
        }

        template<typename ValueType>
        std::enable_if_t<std::is_integral_v<ValueType> and (sizeof(ValueType) > 4), void>
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
        std::enable_if_t<kungfu::is_array_v<ValueType>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length);
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        std::enable_if_t<kungfu::is_vector_v<ValueType>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), value.size());
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, std::string>, void> Set(Napi::Object &obj, const char *name, const ValueType &value)
        {
            obj.Set(name, Napi::String::New(obj.Env(), value));
        }
    };

    class UpdateOperator
    {
    private:
        ToJs convert;
        Napi::ObjectReference &state_;

    public:
        UpdateOperator(Napi::ObjectReference &state) : state_(state)
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
            convert(data, value);
        }
    };
}

#endif //KUNGFU_NODE_SERIALIZE_H
