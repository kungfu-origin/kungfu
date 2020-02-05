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

#ifndef _WIN32
#define KF_DATA_STRUCT_BEGIN
#define KF_DATA_STRUCT_END __attribute__((packed))
#else
#define KF_DATA_STRUCT_BEGIN __pragma(pack(push, 1))
#define KF_DATA_STRUCT_END __pragma(pack(pop))
#endif

#define KF_DEFINE_DATA_STRUCT(NAME, TAG, ...) \
KF_DATA_STRUCT_BEGIN \
struct NAME : public kungfu::type { static constexpr int32_t tag = TAG; BOOST_HANA_DEFINE_STRUCT(NAME, __VA_ARGS__); } \
KF_DATA_STRUCT_END

#define KF_DEFINE_MARK_STRUCT(NAME, TAG) struct NAME : public kungfu::type { static constexpr int32_t tag = TAG; }

template<typename T, size_t N>
using CArray = T[N];

namespace kungfu
{

    struct type
    {
    };

    template <typename>
    struct member_pointer_trait;

    template <template<typename MemberPtr, MemberPtr ptr> typename T, typename MemberPtr, MemberPtr ptr>
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
