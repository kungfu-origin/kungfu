//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_COMMON_H
#define KUNGFU_COMMON_H

#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <boost/hana.hpp>
#include <nlohmann/json.hpp>

#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##_ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /** forward defile smart ptr */

#define PK(...) boost::hana::make_tuple(MAKE_PK(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__))
#define MAKE_PK(N, ...) BOOST_HANA_PP_CONCAT(MAKE_PK_IMPL_, N)(__VA_ARGS__)
#define MAKE_PK_IMPL_1(k) BOOST_HANA_STRING(#k)
#define MAKE_PK_IMPL_2(k1, k2) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2)
#define MAKE_PK_IMPL_3(k1, k2, k3) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2), BOOST_HANA_STRING(#k3)
#define MAKE_PK_IMPL_4(k1, k2, k3, k4) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2), BOOST_HANA_STRING(#k3), BOOST_HANA_STRING(#k4)

#ifndef _WIN32
#define KF_DATA_TYPE_BEGIN
#define KF_DATA_TYPE_END __attribute__((packed));
#else
#define KF_DATA_TYPE_BEGIN __pragma(pack(push, 1))
#define KF_DATA_TYPE_END __pragma(pack(pop));
#endif

#define KF_DEFINE_DATA_TYPE(NAME, TAG, PRIMARY_KEYS, ...) \
KF_DATA_TYPE_BEGIN \
struct NAME : public kungfu::type<NAME> { \
    static constexpr int32_t tag = TAG; \
    static constexpr auto primary_keys = PRIMARY_KEYS; \
    NAME() {}; \
    explicit NAME(const char *address) : kungfu::type<NAME>(address) {}; \
    BOOST_HANA_DEFINE_STRUCT(NAME, __VA_ARGS__); \
} \
KF_DATA_TYPE_END

#define KF_DEFINE_MARK_TYPE(NAME, TAG) \
struct NAME : public kungfu::type<NAME> { \
    static constexpr int32_t tag = TAG; \
    static constexpr auto primary_keys = boost::hana::make_tuple(); \
}

namespace kungfu
{

    uint32_t hash_32(const unsigned char *key, int32_t length);

    template<typename V, size_t N, typename = void>
    struct array_to_string;

    template<typename V, size_t N>
    struct array_to_string<V, N, std::enable_if_t<std::is_same_v<V, char>>>
    {
        std::string operator()(const V *v)
        {
            return std::string(v);
        };
    };

    template<typename V, size_t N>
    struct array_to_string<V, N, std::enable_if_t<not std::is_same_v<V, char>>>
    {
        std::string operator()(const V *v)
        {
            std::stringstream ss;
            ss << "[";
            for (int i = 0; i < N; i++)
            {
                ss << (i > 0 ? "," : "") << v[i];
            }
            ss << "]";
            return ss.str();
        };
    };

    template<typename T, size_t N>
    struct array
    {
        using type = T[N];
        type value;

        array()
        {
            memset(value, 0, sizeof(type));
        }

        array(const T *t)
        {
            memcpy(value, t, sizeof(value));
        }

        array(const unsigned char *t)
        {
            memcpy(value, t, sizeof(value));
        }

        operator T *()
        {
            return static_cast<T *>(value);
        }

        operator const T *() const
        {
            return static_cast<const T *>(value);
        }

        operator std::string() const
        {
            return array_to_string<T, N>{}(value);
        }

        T operator[](int i) const
        {
            return value[i];
        }

        array<T, N> &operator=(const array<T, N> &v)
        {
            memcpy(value, v.value, sizeof(value));
            return *this;
        }
    };

    template<typename T, size_t N>
    void to_json(nlohmann::json &j, const array<T, N> &value)
    {
        j = value.value;
    }

    template<typename T, size_t N>
    void from_json(const nlohmann::json &j, array<T, N> &value)
    {
        for (int i = 0; i < N; i++)
        {
            value.value[i] = j[i].get<T>();
        }
    }

    template<typename T>
    struct is_array
    {
        static constexpr bool value = false;
    };

    template<typename T, size_t N>
    struct is_array<array<T, N>>
    {
        static constexpr bool value = true;
    };

    template<typename T>
    static constexpr bool is_array_v = is_array<T>::value;

    template<typename T, typename = void>
    struct size_fixed
    {
        static constexpr bool value = false;
    };

    template<typename DataType>
    struct size_fixed<DataType, std::enable_if_t<std::is_fundamental_v<DataType> or std::is_enum_v<DataType>>>
    {
        static constexpr bool value = true;
    };

    template<typename DataType>
    struct size_fixed<DataType, std::enable_if_t<std::is_class_v<DataType> and DataType::tag>>
    {
        static constexpr bool value = boost::hana::fold(boost::hana::transform(boost::hana::accessors<DataType>(), [](auto it)
        {
            auto accessor = boost::hana::second(it);
            using AttrType = std::decay_t<decltype(accessor(std::forward<DataType &>(DataType{})))>;
            return std::is_arithmetic_v<AttrType> or std::is_enum_v<AttrType> or is_array_v<AttrType>;
        }), std::logical_and());
    };

    template<typename DataType>
    static constexpr bool size_fixed_v = size_fixed<DataType>::value;

    template<typename, typename = void>
    struct hash;

    template<typename T>
    struct hash<T, std::enable_if_t<std::is_integral_v<T> and not std::is_pointer_v<T>>>
    {
        uint64_t operator()(const T &value)
        {
            return value;
        }
    };

    template<typename T>
    struct hash<T, std::enable_if_t<std::is_pointer_v<T>>>
    {
        uint64_t operator()(const T &value)
        {
            return hash_32(reinterpret_cast<const unsigned char *>(value), sizeof(value));
        }
    };

    template<>
    struct hash<std::string>
    {
        uint64_t operator()(const std::string &value)
        {
            return hash_32(reinterpret_cast<const unsigned char *>(value.c_str()), sizeof(value));
        }
    };

    template<typename T, size_t N>
    struct hash<array<T, N>>
    {
        uint64_t operator()(const array<T, N> &value)
        {
            return hash_32(reinterpret_cast<const unsigned char *>(value.value), sizeof(value));
        }
    };

    template<typename DataType>
    struct type
    {
        static constexpr bool reflect = true;

        type()
        {}

        explicit type(const char *address)
        {
            nlohmann::json j = nlohmann::json::parse(address);
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                using AttrType = std::decay_t<decltype(accessor(std::forward<DataType &>(DataType{})))>;
                j[name.c_str()].get_to(accessor(*const_cast<DataType *>(reinterpret_cast<const DataType *>(this))));
            });
        }

        std::string to_string() const
        {
            nlohmann::json j{};
            boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it)
            {
                auto name = boost::hana::first(it);
                auto accessor = boost::hana::second(it);
                using AttrType = std::decay_t<decltype(accessor(std::forward<DataType &>(DataType{})))>;
                j[name.c_str()] = accessor(*reinterpret_cast<const DataType *>(this));
            });
            return j.dump();
        }

        operator std::string() const
        {
            return to_string();
        }

        uint64_t uid() const
        {
            auto primary_keys = boost::hana::transform(DataType::primary_keys, [this](auto pk)
            {
                auto just = boost::hana::find_if(boost::hana::accessors<DataType>(), [&](auto it)
                {
                    return pk == boost::hana::first(it);
                });
                auto accessor = boost::hana::second(*just);
                auto value = accessor(*(reinterpret_cast<const DataType *>(this)));
                return hash<decltype(value)>{}(value);
            });
            return boost::hana::fold(primary_keys, std::bit_xor());
        }
    };

    template<typename>
    struct member_pointer_trait;

    template<template<typename MemberPtr, MemberPtr ptr> typename T, typename MemberPtr, MemberPtr ptr>
    struct member_pointer_trait<T<MemberPtr, ptr>>
    {
        static constexpr MemberPtr pointer()
        {
            return ptr;
        }
    };

    class event
    {
    public:

        virtual ~event() = default;

        [[nodiscard]] virtual int64_t gen_time() const = 0;

        [[nodiscard]] virtual int64_t trigger_time() const = 0;

        [[nodiscard]] virtual int32_t msg_type() const = 0;

        [[nodiscard]] virtual uint32_t source() const = 0;

        [[nodiscard]] virtual uint32_t dest() const = 0;

        [[nodiscard]] virtual uint32_t data_length() const = 0;

        [[nodiscard]] virtual const char *data_as_bytes() const = 0;

        [[nodiscard]] virtual std::string data_as_string() const = 0;

        [[nodiscard]] virtual std::string to_string() const = 0;

        /**
         * Using auto with the return mess up the reference with the undlerying memory address, DO NOT USE it.
         * @tparam T
         * @return a casted reference to the underlying memory address
         */
        template<typename T>
        std::enable_if_t<size_fixed_v<T>, const T &> data() const
        {
            return *(reinterpret_cast<const T *>(data_address()));
        }

        template<typename T>
        std::enable_if_t<not size_fixed_v<T>, const T> data() const
        {
            return T(data_as_bytes());
        }

    protected:
        virtual const void *data_address() const = 0;
    };

    DECLARE_PTR(event)
}

#endif //KUNGFU_COMMON_H
