//
// Created by Keren Dong on 2020/2/11.
//

#ifndef KUNGFU_NODE_SERIALIZE_H
#define KUNGFU_NODE_SERIALIZE_H

#include <napi.h>
#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/longfist/serialize/sql.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::node::serialize
{
    template<typename DataType>
    struct JsMake
    {
        const char *type_name;

        JsMake(const char *_type_name) : type_name(_type_name)
        {}

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
                auto addr = reinterpret_cast<ElementType *>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * element_size);
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
                auto addr = reinterpret_cast<ValueType *>(reinterpret_cast<uintptr_t>(buffer.Data()) + i * sizeof(ValueType));
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
                value.push_back(*reinterpret_cast<ValueType *>(addr));
            }
        }

        template<typename ValueType>
        std::enable_if_t<std::is_same_v<ValueType, std::string>, void> Get(ValueType &value, const char *name, const Napi::Object &obj)
        {
            value = obj.Get(name).ToString().Utf8Value();
        }
    };

    struct JsRestoreState
    {
    private:
        JsSet set;
        yijinjing::practice::apprentice &app_;
        Napi::ObjectReference &state_;
        yijinjing::data::location_ptr location_;

    public:
        explicit JsRestoreState(yijinjing::practice::apprentice &app, Napi::ObjectReference &state, yijinjing::data::location_ptr location) :
                app_(app), state_(state), location_(std::move(location))
        {}

        void operator()()
        {
            auto now = yijinjing::time::now_in_nano();
            for (auto dest : location_->locator->list_location_dest(location_))
            {
                auto db_file = location_->locator->layout_file(location_, longfist::enums::layout::SQLITE, fmt::format("{:08x}", dest));
                auto storage = longfist::sqlite::make_storage(db_file, longfist::StateDataTypes);
                storage.sync_schema();
                boost::hana::for_each(longfist::StateDataTypes, [&](auto it)
                {
                    using DataType = typename decltype(+boost::hana::second(it))::type;
                    auto type_name = boost::hana::first(it).c_str();
                    for (auto &data : storage.template get_all<DataType>())
                    {
                        Napi::Object table = state_.Get(type_name).ToObject();
                        std::string uid_key = fmt::format("{:016x}", data.uid());
                        Napi::Value value = state_.Get(uid_key);
                        if (value.IsUndefined() or value.IsEmpty())
                        {
                            value = Napi::Object::New(state_.Env());
                            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
                            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("uid_key", Napi::String::New(value.Env(), uid_key)));
                            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), location_->uid)));
                            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), dest)));
                            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("update_time", Napi::BigInt::New(value.Env(), now)));
                            table.Set(uid_key, value);
                        }
                        set(data, value);
                    }
                });
            }
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
        void operator()(const std::string &type_name, boost::hana::basic_type<DataType> type, const event_ptr &event)
        {
            auto data = event->data<DataType>();
            Napi::Object table = state_.Get(type_name).ToObject();
            std::string uid = fmt::format("{:016x}", data.uid());
            Napi::Value value = state_.Get(uid);
            if (value.IsUndefined() or value.IsEmpty())
            {
                value = Napi::Object::New(state_.Env());
                value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
                value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("uid", Napi::String::New(value.Env(), uid)));
                value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), event->source())));
                value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), event->dest())));
                value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("update_time", Napi::BigInt::New(value.Env(), event->gen_time())));
                table.Set(uid, value);
            }
            set(data, value);
        }
    };

    struct JsPublishState
    {
    private:
        JsGet get;
        yijinjing::practice::apprentice &app_;
        Napi::ObjectReference &state_;

    public:
        explicit JsPublishState(yijinjing::practice::apprentice &app, Napi::ObjectReference &state) : app_(app), state_(state)
        {}

        template<typename DataType>
        void operator()(const std::string &type_name, boost::hana::basic_type<DataType> type, Napi::Value &value)
        {
            DataType data{};
            get(value, data);
            auto now = yijinjing::time::now_in_nano();
            auto location = app_.get_io_device()->get_home();
            auto uid_key = fmt::format("{:016x}", data.uid());
            Napi::Object valueObj = value.ToObject();
            valueObj.DefineProperty(Napi::PropertyDescriptor::Value("type", Napi::String::New(value.Env(), type_name)));
            valueObj.DefineProperty(Napi::PropertyDescriptor::Value("uid", Napi::String::New(value.Env(), uid_key)));
            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("source", Napi::Number::New(value.Env(), location->uid)));
            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("dest", Napi::Number::New(value.Env(), 0)));
            value.ToObject().DefineProperty(Napi::PropertyDescriptor::Value("update_time", Napi::BigInt::New(value.Env(), now)));
            state_.Get(type_name).ToObject().Set(uid_key, valueObj);
            app_.write_to(0, data);
        }
    };
}

#endif //KUNGFU_NODE_SERIALIZE_H
