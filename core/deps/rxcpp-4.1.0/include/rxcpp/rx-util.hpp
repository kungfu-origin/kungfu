// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_UTIL_HPP)
#define RXCPP_RX_UTIL_HPP

#include "rx-includes.hpp"

#if !defined(RXCPP_ON_IOS) && !defined(RXCPP_ON_ANDROID) && !defined(RXCPP_THREAD_LOCAL)
#if defined(_MSC_VER)
#define RXCPP_THREAD_LOCAL __declspec(thread)
#else
#define RXCPP_THREAD_LOCAL __thread
#endif
#endif

#if !defined(RXCPP_DELETE)
#if defined(_MSC_VER)
#define RXCPP_DELETE __pragma(warning(disable: 4822)) =delete
#else
#define RXCPP_DELETE =delete
#endif
#endif

#define RXCPP_CONCAT(Prefix, Suffix) Prefix ## Suffix
#define RXCPP_CONCAT_EVALUATE(Prefix, Suffix) RXCPP_CONCAT(Prefix, Suffix)

#define RXCPP_MAKE_IDENTIFIER(Prefix) RXCPP_CONCAT_EVALUATE(Prefix, __LINE__)

namespace rxcpp {

namespace util {

template<class T> using value_type_t = typename std::decay<T>::type::value_type;
template<class T> using decay_t = typename std::decay<T>::type;
template<class... TN> using result_of_t = typename std::result_of<TN...>::type;

template<class T, std::size_t size>
std::vector<T> to_vector(const T (&arr) [size]) {
    return std::vector<T>(std::begin(arr), std::end(arr));
}

template<class T>
std::vector<T> to_vector(std::initializer_list<T> il) {
    return std::vector<T>(il);
}

template<class T0, class... TN>
typename std::enable_if<!std::is_array<T0>::value && std::is_pod<T0>::value, std::vector<T0>>::type to_vector(T0 t0, TN... tn) {
    return to_vector({t0, tn...});
}

// lifted from https://github.com/ericniebler/range-v3/blob/630fc70baa07cbfd222f329e44a3122ab64ce364/include/range/v3/range_fwd.hpp
// removed constexpr & noexcept to support older VC compilers
template<typename T>
/*constexpr*/ T const &as_const(T & t) /*noexcept*/
{
    return t;
}
template<typename T>
void as_const(T const &&) = delete;

template<class T, T... ValueN>
struct values {};

template<class T, int Remaining, T Step = 1, T Cursor = 0, T... ValueN>
struct values_from;

template<class T, T Step, T Cursor, T... ValueN>
struct values_from<T, 0, Step, Cursor, ValueN...>
{
    typedef values<T, ValueN...> type;
};

template<class T, int Remaining, T Step, T Cursor, T... ValueN>
struct values_from
{
    typedef typename values_from<T, Remaining - 1, Step, Cursor + Step, ValueN..., Cursor>::type type;
};

template<bool... BN>
struct all_true;

template<bool B>
struct all_true<B>
{
    static const bool value = B;
};
template<bool B, bool... BN>
struct all_true<B, BN...>
{
    static const bool value = B && all_true<BN...>::value;
};

template<bool... BN>
using enable_if_all_true_t = typename std::enable_if<all_true<BN...>::value>::type;

template<class... BN>
struct all_true_type;

template<class B>
struct all_true_type<B>
{
    static const bool value = B::value;
};
template<class B, class... BN>
struct all_true_type<B, BN...>
{
    static const bool value = B::value && all_true_type<BN...>::value;
};

template<class... BN>
using enable_if_all_true_type_t = typename std::enable_if<all_true_type<BN...>::value>::type;

struct all_values_true {
    template<class... ValueN>
    bool operator()(ValueN... vn) const;

    template<class Value0>
    bool operator()(Value0 v0) const {
        return v0;
    }

    template<class Value0, class... ValueN>
    bool operator()(Value0 v0, ValueN... vn) const {
        return v0 && all_values_true()(vn...);
    }
};

struct any_value_true {
    template<class... ValueN>
    bool operator()(ValueN... vn) const;

    template<class Value0>
    bool operator()(Value0 v0) const {
        return v0;
    }

    template<class Value0, class... ValueN>
    bool operator()(Value0 v0, ValueN... vn) const {
        return v0 || any_value_true()(vn...);
    }
};

template<class... TN>
struct types {};

//
// based on Walter Brown's void_t proposal
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3911.pdf
//

struct types_checked {};

namespace detail {
template<class... TN> struct types_checked_from {typedef types_checked type;};
}

template<class... TN>
struct types_checked_from {typedef typename detail::types_checked_from<TN...>::type type;};

template<class... TN>
using types_checked_t = typename types_checked_from<TN...>::type;


template<class Types, class =types_checked>
struct expand_value_types { struct type; };
template<class... TN>
struct expand_value_types<types<TN...>, types_checked_t<typename std::decay<TN>::type::value_type...>>
{
    using type = types<typename std::decay<TN>::type::value_type...>;
};
template<class... TN>
using value_types_t = typename expand_value_types<types<TN...>>::type;


template<class T, class C = types_checked>
struct value_type_from : public std::false_type {typedef types_checked type;};

template<class T>
struct value_type_from<T, typename types_checked_from<value_type_t<T>>::type>
    : public std::true_type {typedef value_type_t<T> type;};

namespace detail {
template<class F, class... ParamN, int... IndexN>
auto apply(std::tuple<ParamN...> p, values<int, IndexN...>, F&& f)
    -> decltype(f(std::forward<ParamN>(std::get<IndexN>(p))...)) {
    return      f(std::forward<ParamN>(std::get<IndexN>(p))...);
}

template<class F_inner, class F_outer, class... ParamN, int... IndexN>
auto apply_to_each(std::tuple<ParamN...>& p, values<int, IndexN...>, F_inner& f_inner, F_outer& f_outer)
    -> decltype(f_outer(std::move(f_inner(std::get<IndexN>(p)))...)) {
    return      f_outer(std::move(f_inner(std::get<IndexN>(p)))...);
}

template<class F_inner, class F_outer, class... ParamN, int... IndexN>
auto apply_to_each(std::tuple<ParamN...>& p, values<int, IndexN...>, const F_inner& f_inner, const F_outer& f_outer)
    -> decltype(f_outer(std::move(f_inner(std::get<IndexN>(p)))...)) {
    return      f_outer(std::move(f_inner(std::get<IndexN>(p)))...);
}

}
template<class F, class... ParamN>
auto apply(std::tuple<ParamN...> p, F&& f)
    -> decltype(detail::apply(std::move(p), typename values_from<int, sizeof...(ParamN)>::type(), std::forward<F>(f))) {
    return      detail::apply(std::move(p), typename values_from<int, sizeof...(ParamN)>::type(), std::forward<F>(f));
}

template<class F_inner, class F_outer, class... ParamN>
auto apply_to_each(std::tuple<ParamN...>& p, F_inner& f_inner, F_outer& f_outer)
    -> decltype(detail::apply_to_each(p, typename values_from<int, sizeof...(ParamN)>::type(), f_inner, f_outer)) {
    return      detail::apply_to_each(p, typename values_from<int, sizeof...(ParamN)>::type(), f_inner, f_outer);
}

template<class F_inner, class F_outer, class... ParamN>
auto apply_to_each(std::tuple<ParamN...>& p, const F_inner& f_inner, const F_outer& f_outer)
    -> decltype(detail::apply_to_each(p, typename values_from<int, sizeof...(ParamN)>::type(), f_inner, f_outer)) {
    return      detail::apply_to_each(p, typename values_from<int, sizeof...(ParamN)>::type(), f_inner, f_outer);
}

namespace detail {

template<class F>
struct apply_to
{
    F to;

    explicit apply_to(F f)
        : to(std::move(f))
    {
    }

    template<class... ParamN>
    auto operator()(std::tuple<ParamN...> p)
        -> decltype(rxcpp::util::apply(std::move(p), to)) {
        return      rxcpp::util::apply(std::move(p), to);
    }
    template<class... ParamN>
    auto operator()(std::tuple<ParamN...> p) const
        -> decltype(rxcpp::util::apply(std::move(p), to)) {
        return      rxcpp::util::apply(std::move(p), to);
    }
};

}

template<class F>
auto apply_to(F f)
    ->      detail::apply_to<F> {
    return  detail::apply_to<F>(std::move(f));
}

namespace detail {

struct pack
{
    template<class... ParamN>
    auto operator()(ParamN... pn)
        -> decltype(std::make_tuple(std::move(pn)...)) {
        return      std::make_tuple(std::move(pn)...);
    }
    template<class... ParamN>
    auto operator()(ParamN... pn) const
        -> decltype(std::make_tuple(std::move(pn)...)) {
        return      std::make_tuple(std::move(pn)...);
    }
};

}

inline auto pack()
    ->      detail::pack {
    return  detail::pack();
}

namespace detail {

template<int Index>
struct take_at
{
    template<class... ParamN>
    auto operator()(ParamN... pn)
        -> typename std::tuple_element<Index, std::tuple<decay_t<ParamN>...>>::type {
        return                std::get<Index>(std::make_tuple(std::move(pn)...));
    }
    template<class... ParamN>
    auto operator()(ParamN... pn) const
        -> typename std::tuple_element<Index, std::tuple<decay_t<ParamN>...>>::type {
        return                std::get<Index>(std::make_tuple(std::move(pn)...));
    }
};

}

template<int Index>
inline auto take_at()
    ->      detail::take_at<Index> {
    return  detail::take_at<Index>();
}

template <class D>
struct resolve_type;

template <template<class... TN> class Deferred, class... AN>
struct defer_trait
{
    template<bool R>
    struct tag_valid {static const bool valid = true; static const bool value = R;};
    struct tag_not_valid {static const bool valid = false; static const bool value = false;};
    typedef Deferred<typename resolve_type<AN>::type...> resolved_type;
    template<class... CN>
    static auto check(int) -> tag_valid<resolved_type::value>;
    template<class... CN>
    static tag_not_valid check(...);

    typedef decltype(check<AN...>(0)) tag_type;
    static const bool valid = tag_type::valid;
    static const bool value = tag_type::value;
    static const bool not_value = valid && !value;
};

template <template<class... TN> class Deferred, class... AN>
struct defer_type
{
    template<class R>
    struct tag_valid {typedef R type; static const bool value = true;};
    struct tag_not_valid {typedef void type; static const bool value = false;};
    typedef Deferred<typename resolve_type<AN>::type...> resolved_type;
    template<class... CN>
    static auto check(int) -> tag_valid<resolved_type>;
    template<class... CN>
    static tag_not_valid check(...);

    typedef decltype(check<AN...>(0)) tag_type;
    typedef typename tag_type::type type;
    static const bool value = tag_type::value;
};

template <template<class... TN> class Deferred, class... AN>
struct defer_value_type
{
    template<class R>
    struct tag_valid {typedef R type; static const bool value = true;};
    struct tag_not_valid {typedef void type; static const bool value = false;};
    typedef Deferred<typename resolve_type<AN>::type...> resolved_type;
    template<class... CN>
    static auto check(int) -> tag_valid<value_type_t<resolved_type>>;
    template<class... CN>
    static tag_not_valid check(...);

    typedef decltype(check<AN...>(0)) tag_type;
    typedef typename tag_type::type type;
    static const bool value = tag_type::value;
};

template <template<class... TN> class Deferred, class... AN>
struct defer_seed_type
{
    template<class R>
    struct tag_valid {typedef R type; static const bool value = true;};
    struct tag_not_valid {typedef void type; static const bool value = false;};
    typedef Deferred<typename resolve_type<AN>::type...> resolved_type;
    template<class... CN>
    static auto check(int) -> tag_valid<typename resolved_type::seed_type>;
    template<class... CN>
    static tag_not_valid check(...);

    typedef decltype(check<AN...>(0)) tag_type;
    typedef typename tag_type::type type;
    static const bool value = tag_type::value;
};

template <class D>
struct resolve_type
{
    typedef D type;
};
template <template<class... TN> class Deferred, class... AN>
struct resolve_type<defer_type<Deferred, AN...>>
{
    typedef typename defer_type<Deferred, AN...>::type type;
};
template <template<class... TN> class Deferred, class... AN>
struct resolve_type<defer_value_type<Deferred, AN...>>
{
    typedef typename defer_value_type<Deferred, AN...>::type type;
};
template <template<class... TN> class Deferred, class... AN>
struct resolve_type<defer_seed_type<Deferred, AN...>>
{
    typedef typename defer_seed_type<Deferred, AN...>::type type;
};

struct plus
{
    template <class LHS, class RHS>
    auto operator()(LHS&& lhs, RHS&& rhs) const
        -> decltype(std::forward<LHS>(lhs) + std::forward<RHS>(rhs))
        { return std::forward<LHS>(lhs) + std::forward<RHS>(rhs); }
};

struct count
{
    template <class T>
    int operator()(int cnt, T&&) const
    { return cnt + 1; }
};

struct less
{
    template <class LHS, class RHS>
    auto operator()(LHS&& lhs, RHS&& rhs) const
        -> decltype(std::forward<LHS>(lhs) < std::forward<RHS>(rhs))
        { return std::forward<LHS>(lhs) < std::forward<RHS>(rhs); }
};

template<class T = void>
struct equal_to
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
};

template<>
struct equal_to<void>
{
    template<class LHS, class RHS>
    auto operator()(LHS&& lhs, RHS&& rhs) const
    -> decltype(std::forward<LHS>(lhs) == std::forward<RHS>(rhs))
    { return std::forward<LHS>(lhs) == std::forward<RHS>(rhs); }
};

namespace detail {
template<class OStream, class Delimit>
struct print_function
{
    OStream& os;
    Delimit delimit;
    print_function(OStream& os, Delimit d) : os(os), delimit(std::move(d)) {}

    template<class... TN>
    void operator()(const TN&... tn) const {
        bool inserts[] = {(os << tn, true)...};
        inserts[0] = *reinterpret_cast<bool*>(inserts); // silence warning
        delimit();
    }

    template<class... TN>
    void operator()(const std::tuple<TN...>& tpl) const {
        rxcpp::util::apply(tpl, *this);
    }
};

template<class OStream>
struct endline
{
    OStream& os;
    endline(OStream& os) : os(os) {}
    void operator()() const {
        os << std::endl;
    }
private:
    endline& operator=(const endline&) RXCPP_DELETE;
};

template<class OStream, class ValueType>
struct insert_value
{
    OStream& os;
    ValueType value;
    insert_value(OStream& os, ValueType v) : os(os), value(std::move(v)) {}
    void operator()() const {
        os << value;
    }
private:
    insert_value& operator=(const insert_value&) RXCPP_DELETE;
};

template<class OStream, class Function>
struct insert_function
{
    OStream& os;
    Function call;
    insert_function(OStream& os, Function f) : os(os), call(std::move(f)) {}
    void operator()() const {
        call(os);
    }
private:
    insert_function& operator=(const insert_function&) RXCPP_DELETE;
};

template<class OStream, class Delimit>
auto print_followed_with(OStream& os, Delimit d)
    ->      detail::print_function<OStream, Delimit> {
    return  detail::print_function<OStream, Delimit>(os, std::move(d));
}

}

template<class OStream>
auto endline(OStream& os)
    -> detail::endline<OStream> {
    return detail::endline<OStream>(os);
}

template<class OStream>
auto println(OStream& os)
    -> decltype(detail::print_followed_with(os, endline(os))) {
    return      detail::print_followed_with(os, endline(os));
}
template<class OStream, class Delimit>
auto print_followed_with(OStream& os, Delimit d)
    -> decltype(detail::print_followed_with(os, detail::insert_function<OStream, Delimit>(os, std::move(d)))) {
    return      detail::print_followed_with(os, detail::insert_function<OStream, Delimit>(os, std::move(d)));
}
template<class OStream, class DelimitValue>
auto print_followed_by(OStream& os, DelimitValue dv)
    -> decltype(detail::print_followed_with(os, detail::insert_value<OStream, DelimitValue>(os, std::move(dv)))) {
    return      detail::print_followed_with(os, detail::insert_value<OStream, DelimitValue>(os, std::move(dv)));
}

inline std::string what(std::exception_ptr ep) {
    try {std::rethrow_exception(ep);}
    catch (const std::exception& ex) {
        return ex.what();
    }
    return std::string();
}

namespace detail {

template <class T>
class maybe
{
    bool is_set;
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type
        storage;
public:
    maybe()
    : is_set(false)
    {
    }

    maybe(T value)
    : is_set(false)
    {
        new (reinterpret_cast<T*>(&storage)) T(value);
        is_set = true;
    }

    maybe(const maybe& other)
    : is_set(false)
    {
        if (other.is_set) {
            new (reinterpret_cast<T*>(&storage)) T(other.get());
            is_set = true;
        }
    }
    maybe(maybe&& other)
    : is_set(false)
    {
        if (other.is_set) {
            new (reinterpret_cast<T*>(&storage)) T(std::move(other.get()));
            is_set = true;
            other.reset();
        }
    }

    ~maybe()
    {
        reset();
    }

    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;

    bool empty() const {
        return !is_set;
    }

    std::size_t size() const {
        return is_set ? 1 : 0;
    }

    iterator begin() {
        return reinterpret_cast<T*>(&storage);
    }
    const_iterator begin() const {
        return reinterpret_cast<T*>(&storage);
    }

    iterator end() {
        return reinterpret_cast<T*>(&storage) + size();
    }
    const_iterator end() const {
        return reinterpret_cast<T*>(&storage) + size();
    }

    T* operator->() {
        if (!is_set) std::terminate();
        return reinterpret_cast<T*>(&storage);
    }
    const T* operator->() const {
        if (!is_set) std::terminate();
        return reinterpret_cast<T*>(&storage);
    }

    T& operator*() {
        if (!is_set) std::terminate();
        return *reinterpret_cast<T*>(&storage);
    }
    const T& operator*() const {
        if (!is_set) std::terminate();
        return *reinterpret_cast<T*>(&storage);
    }

    T& get() {
        if (!is_set) std::terminate();
        return *reinterpret_cast<T*>(&storage);
    }
    const T& get() const {
        if (!is_set) std::terminate();
        return *reinterpret_cast<const T*>(&storage);
    }

    void reset()
    {
        if (is_set) {
            is_set = false;
            reinterpret_cast<T*>(&storage)->~T();
            //std::fill_n(reinterpret_cast<char*>(&storage), sizeof(T), 0);
        }
    }

    template<class U>
    void reset(U&& value) {
        reset();
        new (reinterpret_cast<T*>(&storage)) T(std::forward<U>(value));
        is_set = true;
    }

    maybe& operator=(const T& other) {
        reset(other);
        return *this;
    }
    maybe& operator=(const maybe& other) {
        if (!other.empty()) {
            reset(other.get());
        } else {
            reset();
        }
        return *this;
    }
};

}
using detail::maybe;

namespace detail {
    struct surely
    {
        template<class... T>
        auto operator()(T... t)
            -> decltype(std::make_tuple(t.get()...)) {
            return      std::make_tuple(t.get()...);
        }
        template<class... T>
        auto operator()(T... t) const
            -> decltype(std::make_tuple(t.get()...)) {
            return      std::make_tuple(t.get()...);
        }
    };
}

template<class... T>
inline auto surely(const std::tuple<T...>& tpl)
    -> decltype(apply(tpl, detail::surely())) {
    return      apply(tpl, detail::surely());
}

namespace detail {

template<typename Function>
class unwinder
{
public:
    ~unwinder()
    {
        if (!!function)
        {
            try {
                (*function)();
            } catch (...) {
                std::terminate();
            }
        }
    }

    explicit unwinder(Function* functionArg)
        : function(functionArg)
    {
    }

    void dismiss()
    {
        function = nullptr;
    }

private:
    unwinder();
    unwinder(const unwinder&);
    unwinder& operator=(const unwinder&);

    Function* function;
};

}

#if !defined(RXCPP_THREAD_LOCAL)
template<typename T>
class thread_local_storage
{
private:
    pthread_key_t key;

public:
    thread_local_storage()
    {
        pthread_key_create(&key, NULL);
    }

    ~thread_local_storage()
    {
        pthread_key_delete(key);
    }

    thread_local_storage& operator =(T* p)
    {
        pthread_setspecific(key, p);
        return *this;
    }

    bool operator !()
    {
        return pthread_getspecific(key) == NULL;
    }

    T* operator ->()
    {
        return static_cast<T*>(pthread_getspecific(key));
    }

    T* get()
    {
        return static_cast<T*>(pthread_getspecific(key));
    }
};
#endif

template<typename, typename C = types_checked>
struct is_string : std::false_type {
};

template <typename T>
struct is_string<T,
    typename types_checked_from<
        typename T::value_type,
        typename T::traits_type,
        typename T::allocator_type>::type>
    : std::is_base_of<
            std::basic_string<
                typename T::value_type,
                typename T::traits_type,
                typename T::allocator_type>, T> {
};

namespace detail {

    template <class T, class = types_checked>
    struct is_duration : std::false_type {};

    template <class T>
    struct is_duration<T, types_checked_t<T, typename T::rep, typename T::period>>
            : std::is_convertible<T*, std::chrono::duration<typename T::rep, typename T::period>*> {};

}

template <class T, class Decayed = decay_t<T>>
struct is_duration : detail::is_duration<Decayed> {};


// C++17 negation
namespace detail {
    template<class T>
    struct not_value : std::conditional<T::value, std::false_type, std::true_type>::type {
    };
}

template <class T>
struct negation : detail::not_value<T> {};

}
namespace rxu=util;


//
// due to an noisy static_assert issue in more than one std lib impl,
// rxcpp maintains a whitelist filter for the types that are allowed
// to be hashed. this allows is_hashable<T> to work.
//
// NOTE: this should eventually be removed!
//
template <class T, typename = void>
struct filtered_hash;

#if RXCPP_HASH_ENUM
template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_enum<T>::value>::type> : std::hash<T> {
};
#elif RXCPP_HASH_ENUM_UNDERLYING
template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_enum<T>::value>::type> : std::hash<typename std::underlying_type<T>::type> {
};
#endif

template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_integral<T>::value>::type> : std::hash<T> {
};
template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_pointer<T>::value>::type> : std::hash<T> {
};
template <class T>
struct filtered_hash<T, typename std::enable_if<rxu::is_string<T>::value>::type> : std::hash<T> {
};
template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_convertible<T, std::chrono::duration<typename T::rep, typename T::period>>::value>::type> {
    using argument_type = T;
    using result_type = std::size_t;

    result_type operator()(argument_type const & dur) const
    {
        return std::hash<typename argument_type::rep>{}(dur.count());
    }
};
template <class T>
struct filtered_hash<T, typename std::enable_if<std::is_convertible<T, std::chrono::time_point<typename T::clock, typename T::duration>>::value>::type> {
    using argument_type = T;
    using result_type = std::size_t;

    result_type operator()(argument_type const & tp) const
    {
        return std::hash<typename argument_type::rep>{}(tp.time_since_epoch().count());
    }
};

template<typename, typename C = rxu::types_checked>
struct is_hashable
    : std::false_type {};

template<typename T>
struct is_hashable<T,
    typename rxu::types_checked_from<
        typename filtered_hash<T>::result_type,
        typename filtered_hash<T>::argument_type,
        typename std::result_of<filtered_hash<T>(T)>::type>::type>
    : std::true_type {};

}

#define RXCPP_UNWIND(Name, Function) \
    RXCPP_UNWIND_EXPLICIT(uwfunc_ ## Name, Name, Function)

#define RXCPP_UNWIND_AUTO(Function) \
    RXCPP_UNWIND_EXPLICIT(RXCPP_MAKE_IDENTIFIER(uwfunc_), RXCPP_MAKE_IDENTIFIER(unwind_), Function)

#define RXCPP_UNWIND_EXPLICIT(FunctionName, UnwinderName, Function) \
    auto FunctionName = (Function); \
    rxcpp::util::detail::unwinder<decltype(FunctionName)> UnwinderName(std::addressof(FunctionName))

#endif
