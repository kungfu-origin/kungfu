//
// Created by Keren Dong on 2020/2/15.
//

#ifndef KUNGFU_NODE_LONGFIST_H
#define KUNGFU_NODE_LONGFIST_H

#include <napi.h>
#include <kungfu/longfist/longfist.h>

namespace kungfu::node
{
    class Longfist : public Napi::ObjectWrap<Longfist>
    {
    public:
        static void Init(Napi::Env env, Napi::Object exports);

    private:
        explicit Longfist(const Napi::CallbackInfo &info);

        static Napi::ObjectReference constructor;

        template<typename ValueType>
        static std::enable_if_t<std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        static std::enable_if_t<std::is_same_v<ValueType, bool>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Boolean::New(obj.Env(), false));
        }

        template<typename ValueType>
        static std::enable_if_t<std::is_integral_v<ValueType> and (sizeof(ValueType) <= 4) and not std::is_same_v<ValueType, bool>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        static std::enable_if_t<std::is_integral_v<ValueType> and (sizeof(ValueType) > 4), void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::BigInt::New(obj.Env(), (int64_t)0));
        }

        template<typename ValueType>
        static std::enable_if_t<std::is_floating_point_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::Number::New(obj.Env(), 0));
        }

        template<typename ValueType>
        static std::enable_if_t<kungfu::is_array_of_v<ValueType, char>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::String::New(obj.Env(), ""));
        }

        template<typename ValueType>
        static std::enable_if_t<kungfu::is_array_of_others_v<ValueType, char>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), ValueType::length);
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        static std::enable_if_t<kungfu::is_vector_v<ValueType>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            auto buffer = Napi::ArrayBuffer::New(obj.Env(), 0);
            obj.Set(name, buffer);
        }

        template<typename ValueType>
        static std::enable_if_t<std::is_same_v<ValueType, std::string>, void>
        InitValue(Napi::Object &obj, const char *name)
        {
            obj.Set(name, Napi::String::New(obj.Env(), ""));
        }

        template<typename DataType>
        static Napi::Value data(const Napi::CallbackInfo &info)
        {
            auto valueObj = Napi::Object::New(info.Env());
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                using ValueType = std::result_of_t<decltype(accessor)(const DataType &)>;
                InitValue<std::decay_t<ValueType>>(valueObj, name.c_str());
            });
            return valueObj;
        }
    };
}

#endif //KUNGFU_NODE_LONGFIST_H
