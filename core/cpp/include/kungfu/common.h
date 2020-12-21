//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_COMMON_H
#define KUNGFU_COMMON_H

#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

// cope with spdlog-1.5.0@conan-center
#undef SPDLOG_COMPILED_LIB
#undef SPDLOG_HEADER_ONLY
#include <spdlog/spdlog.h>

#ifdef __linux__
#define BOOST_HANA_CONFIG_ENABLE_STRING_UDL
#include <boost/hana.hpp>
using namespace boost::hana::literals;
#define HANA_STR(STR) STR##_s
#else
#include <boost/hana.hpp>
#define HANA_STR(STR) BOOST_HANA_STRING(STR)
#endif

#ifdef _WINDOWS
#define strcpy(dest, src) strcpy_s(dest, sizeof(dest), src)
#define strncpy(dest, src, max_length) strncpy_s(dest, sizeof(dest), src, max_length)
#define KF_PACK_TYPE_BEGIN __pragma(pack(push, 1))
#define KF_PACK_TYPE_END                                                                                               \
  ;                                                                                                                    \
  __pragma(pack(pop))
#else
#define KF_PACK_TYPE_BEGIN
#define KF_PACK_TYPE_END __attribute__((packed));
#endif

#define KF_DEFINE_DATA_TYPE(NAME, TAG, PRIMARY_KEYS, TIMESTAMP_KEY, ...)                                               \
  struct NAME : public kungfu::data<NAME> {                                                                            \
    static constexpr int32_t tag = TAG;                                                                                \
    static constexpr auto type_name = HANA_STR(#NAME);                                                                 \
    static constexpr auto primary_keys = PRIMARY_KEYS;                                                                 \
    static constexpr auto timestamp_key = TIMESTAMP_KEY;                                                               \
    static constexpr bool has_timestamp = boost::hana::is_just(TIMESTAMP_KEY);                                         \
    static constexpr bool has_data = true;                                                                             \
    NAME(){};                                                                                                          \
    explicit NAME(const char *address, const uint32_t length) { parse(address, length); };                             \
    BOOST_HANA_DEFINE_STRUCT(NAME, __VA_ARGS__);                                                                       \
  }

#define KF_DEFINE_PACK_TYPE(NAME, TAG, PRIMARY_KEYS, TIMESTAMP_KEY, ...)                                               \
  KF_PACK_TYPE_BEGIN KF_DEFINE_DATA_TYPE(NAME, TAG, PRIMARY_KEYS, TIMESTAMP_KEY, __VA_ARGS__) KF_PACK_TYPE_END

#define KF_DEFINE_MARK_TYPE(NAME, TAG)                                                                                 \
  struct NAME : public kungfu::data<NAME> {                                                                            \
    static constexpr int32_t tag = TAG;                                                                                \
    static constexpr auto type_name = HANA_STR(#NAME);                                                                 \
    static constexpr auto primary_keys = boost::hana::make_tuple();                                                    \
    static constexpr auto timestamp_key = boost::hana::nothing;                                                        \
    static constexpr bool has_timestamp = false;                                                                       \
    static constexpr bool has_data = false;                                                                            \
    std::string to_string() const { return {}; };                                                                      \
  }

#ifdef BOOST_HANA_WORKAROUND_MSVC_PREPROCESSOR_616033
// refer to boost hana BOOST_HANA_DEFINE_STRUCT
#define MAKE_KEY(N, ...) BOOST_HANA_PP_CONCAT(BOOST_HANA_PP_CONCAT(MAKE_KEY_IMPL_, N)(__VA_ARGS__), )
#else
#define MAKE_KEY(N, ...) BOOST_HANA_PP_CONCAT(MAKE_KEY_IMPL_, N)(__VA_ARGS__)
#endif

#define MAKE_KEY_IMPL_1(k) HANA_STR(#k)
#define MAKE_KEY_IMPL_2(k1, k2) HANA_STR(#k1), HANA_STR(#k2)
#define MAKE_KEY_IMPL_3(k1, k2, k3) HANA_STR(#k1), HANA_STR(#k2), HANA_STR(#k3)
#define MAKE_KEY_IMPL_4(k1, k2, k3, k4) HANA_STR(#k1), HANA_STR(#k2), HANA_STR(#k3), HANA_STR(#k4)

#define PK(...) boost::hana::make_tuple(MAKE_KEY(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__))

#define PERPETUAL() boost::hana::nothing
#define TIMESTAMP(FIELD) boost::hana::just(HANA_STR(#FIELD))

#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##_ptr; /** define smart ptr */
#define FORWARD_DECLARE_PTR(X)                                                                                         \
  class X;                                                                                                             \
  DECLARE_PTR(X) /** forward defile smart ptr */

namespace kungfu {
uint32_t hash_32(const unsigned char *key, int32_t length);

template <typename V, size_t N, typename = void> struct array_to_string;

template <typename V, size_t N> struct array_to_string<V, N, std::enable_if_t<std::is_same_v<V, char>>> {
  std::string operator()(const V *v) { return std::string(v); };
};

template <typename V, size_t N> struct array_to_string<V, N, std::enable_if_t<not std::is_same_v<V, char>>> {
  std::string operator()(const V *v) {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < N; i++) {
      ss << (i > 0 ? "," : "") << v[i];
    }
    ss << "]";
    return ss.str();
  };
};

KF_PACK_TYPE_BEGIN
template <typename T, size_t N> struct array {
  static constexpr size_t length = N;
  using element_type = T;
  using type = T[N];
  type value;

  array() { memset(value, 0, sizeof(value)); }

  explicit array(const T *t) { memcpy(value, t, sizeof(value)); }

  explicit array(const unsigned char *t) { memcpy(value, t, sizeof(value)); }

  [[nodiscard]] size_t size() const { return N; }

  [[nodiscard]] std::string to_string() const { return array_to_string<T, N>{}(value); }

  operator T *() { return static_cast<T *>(value); }

  operator const T *() const { return static_cast<const T *>(value); }

  operator const void *() const { return static_cast<const void *>(value); }

  operator std::string() const { return to_string(); }

  T &operator[](int i) const { return const_cast<T &>(value[i]); }

  array<T, N> &operator=(const T *data) {
    if (value == data) {
      return *this;
    }
    if constexpr (std::is_same_v<T, char>) {
      memcpy(value, data, strlen(data));
    } else {
      memcpy(value, data, sizeof(value));
    }
    return *this;
  }

  array<T, N> &operator=(const array<T, N> &other) { return operator=(other.value); }
} KF_PACK_TYPE_END;

template <typename T, size_t N> void to_json(nlohmann::json &j, const array<T, N> &value) { j = value.value; }

template <typename T, size_t N> void from_json(const nlohmann::json &j, array<T, N> &value) {
  for (int i = 0; i < N; i++) {
    value.value[i] = j[i].get<T>();
  }
}

template <typename T, size_t N> std::ostream &operator<<(std::ostream &os, array<T, N> t) {
  return os << t.to_string();
}

template <typename T> struct is_array : public std::false_type {};

template <typename T, size_t N> struct is_array<array<T, N>> : public std::true_type {};

template <typename T> static constexpr bool is_array_v = is_array<T>::value;

template <typename V, typename T> struct is_array_of : public std::false_type {};

template <typename V, size_t N, typename T> struct is_array_of<array<V, N>, T> {
  static constexpr bool value = std::is_same_v<V, T>;
};

template <typename V, typename T> static constexpr bool is_array_of_v = is_array_of<V, T>::value;

template <typename V, typename T>
static constexpr bool is_array_of_others_v = is_array_v<V> and not is_array_of_v<V, T>;

template <typename T> struct is_vector : public std::false_type {};

template <typename T, typename A> struct is_vector<std::vector<T, A>> : public std::true_type {};

template <typename DataType> static constexpr bool is_vector_v = is_vector<DataType>::value;

template <typename T, typename = void> struct size_fixed : public std::false_type {};

template <typename DataType>
struct size_fixed<DataType, std::enable_if_t<std::is_fundamental_v<DataType> or std::is_enum_v<DataType>>>
    : public std::true_type {};

template <typename DataType>
struct size_fixed<DataType, std::enable_if_t<std::is_class_v<DataType> and not DataType::has_data>> {
  static constexpr bool value = true;
};

template <typename DataType>
struct size_fixed<DataType, std::enable_if_t<std::is_class_v<DataType> and DataType::has_data>> {
  static constexpr bool value = boost::hana::fold(
      boost::hana::transform(boost::hana::accessors<DataType>(),
                             [](auto it) {
                               auto accessor = boost::hana::second(it);
                               using AttrType = std::decay_t<decltype(accessor(std::forward<DataType &>(DataType{})))>;
                               return std::is_arithmetic_v<AttrType> or std::is_enum_v<AttrType> or
                                      is_array_v<AttrType>;
                             }),
      std::logical_and<>());
};

template <typename DataType> static constexpr bool size_fixed_v = size_fixed<DataType>::value;
template <typename DataType> static constexpr bool size_unfixed_v = not size_fixed<DataType>::value;

template <typename ValueType>
static constexpr bool is_signed_int_v = std::is_integral_v<ValueType> and (sizeof(ValueType) <= 4) and
                                        not std::is_same_v<ValueType, bool> and std::is_signed_v<ValueType>;

template <typename ValueType>
static constexpr bool is_unsigned_int_v = std::is_integral_v<ValueType> and (sizeof(ValueType) <= 4) and
                                          not std::is_same_v<ValueType, bool> and std::is_unsigned_v<ValueType>;

template <typename ValueType>
static constexpr bool is_signed_bigint_v = std::is_integral_v<ValueType> and
                                           (sizeof(ValueType) > 4) and std::is_signed_v<ValueType>;

template <typename ValueType>
static constexpr bool is_unsigned_bigint_v = std::is_integral_v<ValueType> and
                                             (sizeof(ValueType) > 4) and std::is_unsigned_v<ValueType>;

template <typename ValueType>
static constexpr bool is_numeric_v = std::is_arithmetic_v<ValueType> or std::is_enum_v<ValueType>;

template <typename ValueType>
static constexpr bool is_enum_class_v = std::is_enum_v<ValueType> and not std::is_convertible_v<ValueType, int>;

template <typename, typename = void> struct hash;

template <typename T> struct hash<T, std::enable_if_t<std::is_integral_v<T> and not std::is_pointer_v<T>>> {
  uint64_t operator()(const T &value) { return value; }
};

template <typename T> struct hash<T, std::enable_if_t<std::is_pointer_v<T>>> {
  uint64_t operator()(const T &value) { return hash_32(reinterpret_cast<const unsigned char *>(value), sizeof(value)); }
};

template <typename T> struct hash<T, std::enable_if_t<is_enum_class_v<T>>> {
  uint64_t operator()(const T &value) { return static_cast<uint64_t>(value); }
};

template <> struct hash<std::string> {
  uint64_t operator()(const std::string &value) {
    return hash_32(reinterpret_cast<const unsigned char *>(value.c_str()), value.length());
  }
};

template <size_t N> struct hash<array<char, N>> {
  uint64_t operator()(const array<char, N> &value) {
    return hash_32(reinterpret_cast<const unsigned char *>(value.value), strlen(value));
  }
};

template <typename T, size_t N> struct hash<array<T, N>> {
  uint64_t operator()(const array<T, N> &value) {
    return hash_32(reinterpret_cast<const unsigned char *>(value.value), sizeof(value));
  }
};

template <typename T, typename... Other> struct type_tuple {
  static constexpr auto value =
      boost::hana::flatten(boost::hana::make_tuple(type_tuple<T>::value, type_tuple<Other...>::value));
};

template <typename T> struct type_tuple<T> {
  static constexpr auto value = boost::hana::make_tuple(boost::hana::type_c<T>);
};

template <typename T, typename... Ts> constexpr void type_check(Ts... arg) {
  constexpr auto check =
      boost::hana::transform(type_tuple<Ts...>::value, [](auto t) { return t == boost::hana::type_c<T>; });
  static_assert(boost::hana::fold(check, std::logical_and<>()), "type check of arguments failed");
}

template <typename DataType> struct data {
  static constexpr bool reflect = true;

  data() {
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it) {
      auto accessor = boost::hana::second(it);
      auto &v = accessor(*const_cast<DataType *>(reinterpret_cast<const DataType *>(this)));
      init_member(v);
    });
  }

  void parse(const char *address, const uint32_t length) {
    std::string content(address, length);
    nlohmann::json jobj = nlohmann::json::parse(content);
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it) {
      auto name = boost::hana::first(it);
      auto accessor = boost::hana::second(it);
      auto &j = jobj[name.c_str()];
      auto &v = accessor(*const_cast<DataType *>(reinterpret_cast<const DataType *>(this)));
      restore_from_json(j, v);
    });
  }

  [[nodiscard]] std::string to_string() const {
    nlohmann::json j = {};
    boost::hana::for_each(boost::hana::accessors<DataType>(), [&, this](auto it) {
      auto name = boost::hana::first(it);
      auto accessor = boost::hana::second(it);
      j[name.c_str()] = accessor(*reinterpret_cast<const DataType *>(this));
    });
    return j.dump();
  }

  explicit operator std::string() const { return to_string(); }

  [[nodiscard]] uint64_t uid() const {
    auto primary_keys = boost::hana::transform(DataType::primary_keys, [this](auto pk) {
      auto just = boost::hana::find_if(boost::hana::accessors<DataType>(),
                                       [&](auto it) { return pk == boost::hana::first(it); });
      auto accessor = boost::hana::second(*just);
      auto value = accessor(*(reinterpret_cast<const DataType *>(this)));
      return hash<decltype(value)>{}(value);
    });
    return boost::hana::fold(primary_keys, std::bit_xor<>());
  }

private:
  template <typename V> static std::enable_if_t<is_numeric_v<V>> init_member(V &v) { v = static_cast<V>(0); }

  template <typename V> static std::enable_if_t<not is_numeric_v<V>> init_member(V &v) {}

  template <typename J, typename V> static std::enable_if_t<std::is_arithmetic_v<V>> restore_from_json(J &j, V &v) {
    v = j;
  }

  template <typename J, typename V> static std::enable_if_t<not std::is_arithmetic_v<V>> restore_from_json(J &j, V &v) {
    j.get_to(v);
  }
};

template <typename> struct member_pointer_trait;

template <template <typename MemberPtr, MemberPtr ptr> typename T, typename MemberPtr, MemberPtr ptr>
struct member_pointer_trait<T<MemberPtr, ptr>> {
  static constexpr MemberPtr pointer() { return ptr; }
};

struct event {
  virtual ~event() = default;

  [[nodiscard]] virtual int64_t gen_time() const = 0;

  [[nodiscard]] virtual int64_t trigger_time() const = 0;

  [[nodiscard]] virtual int32_t msg_type() const = 0;

  [[nodiscard]] virtual uint32_t source() const = 0;

  [[nodiscard]] virtual uint32_t dest() const = 0;

  [[nodiscard]] virtual uint32_t data_length() const = 0;

  [[nodiscard]] virtual const void *data_address() const = 0;

  [[nodiscard]] virtual const char *data_as_bytes() const = 0;

  [[nodiscard]] virtual std::string data_as_string() const = 0;

  [[nodiscard]] virtual std::string to_string() const = 0;

  /**
   * Using auto with the return mess up the reference with the undlerying memory address, DO NOT USE it.
   * @tparam T
   * @return a casted reference to the underlying memory address
   */
  template <typename T> std::enable_if_t<size_fixed_v<T> or std::is_same_v<T, nlohmann::json>, const T &> data() const {
    return *(reinterpret_cast<const T *>(data_address()));
  }

  template <typename T>
  std::enable_if_t<not size_fixed_v<T> and not std::is_same_v<T, nlohmann::json>, const T> data() const {
    return T(data_as_bytes(), data_length());
  }
};

DECLARE_PTR(event)

template <typename DataType> struct typed_event_ptr {
  using type = DataType;

  const event_ptr &event;

  explicit typed_event_ptr(const event_ptr &orig) : event(orig) {}

  const event_ptr &operator*() const { return event; }

  const event_ptr &operator->() const { return event; }
};

template <typename DataType> struct state {
  const uint32_t source;
  const uint32_t dest;

  int64_t update_time;
  DataType data;

  explicit state(const event_ptr &event)
      : source(event->source()), dest(event->dest()), update_time(event->gen_time()), data(event->data<DataType>()) {}

  state(uint32_t s, uint32_t d, int64_t t, const DataType &data) : source(s), dest(d), update_time(t), data(data) {}
};
} // namespace kungfu

#endif // KUNGFU_COMMON_H
