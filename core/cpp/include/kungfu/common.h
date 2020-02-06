//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_COMMON_H
#define KUNGFU_COMMON_H

#include <cstdlib>
#include <string>
#include <boost/hana.hpp>

#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##_ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /** forward defile smart ptr */

#define PK(...) boost::hana::make_tuple(MAKE_PK(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__))
#define MAKE_PK(N, ...) BOOST_HANA_PP_CONCAT(MAKE_PK_IMPL_, N)(__VA_ARGS__)
#define MAKE_PK_IMPL_1(k) BOOST_HANA_STRING(#k)
#define MAKE_PK_IMPL_2(k1, k2) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2)
#define MAKE_PK_IMPL_3(k1, k2, k3) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2), BOOST_HANA_STRING(#k3)
#define MAKE_PK_IMPL_4(k1, k2, k3, k4) BOOST_HANA_STRING(#k1), BOOST_HANA_STRING(#k2), BOOST_HANA_STRING(#k3), BOOST_HANA_STRING(#k4)

#ifndef _WIN32
#define KF_DATA_STRUCT_BEGIN
#define KF_DATA_STRUCT_END __attribute__((packed))
#else
#define KF_DATA_STRUCT_BEGIN __pragma(pack(push, 1))
#define KF_DATA_STRUCT_END __pragma(pack(pop))
#endif

#define KF_DEFINE_DATA_STRUCT(NAME, TAG, PRIMARY_KEYS, ...) \
KF_DATA_STRUCT_BEGIN \
struct NAME : public kungfu::type<NAME> { \
    static constexpr int32_t tag = TAG; \
    static constexpr auto primary_keys = PRIMARY_KEYS; \
    BOOST_HANA_DEFINE_STRUCT(NAME, __VA_ARGS__); \
} \
KF_DATA_STRUCT_END

#define KF_DEFINE_MARK_STRUCT(NAME, TAG) \
struct NAME : public kungfu::type<NAME> { \
    static constexpr int32_t tag = TAG; \
    static constexpr auto primary_keys = boost::hana::make_tuple(); \
}

template<typename T, size_t N>
using CArray = T[N];

namespace kungfu
{

    template<typename DataType>
    struct type
    {
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
                using RawType = decltype(value);
                using AttrType = std::decay_t<RawType>;
                if constexpr (std::is_integral<RawType>::value)
                {
                    return value;
                } else
                {
                    return std::hash<AttrType>{}(value);
                }
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
        inline const T &data() const
        {
            return *(reinterpret_cast<const T *>(data_address()));
        }

    protected:
        virtual const void *data_address() const = 0;
    };

    DECLARE_PTR(event)
}

#endif //KUNGFU_COMMON_H
