#pragma once

#if defined(_MSC_VER)
#if defined(min)
__pragma(push_macro("min"))
#undef min
#define __RESTORE_MIN__
#endif
#if defined(max)
    __pragma(push_macro("max"))
#undef max
#define __RESTORE_MAX__
#endif
#endif  // defined(_MSC_VER)

#include <ciso646>  //  due to #166

#if __cplusplus >= 201703L  // use of C++17 or higher
// Enables use of std::optional in SQLITE_ORM.
#define SQLITE_ORM_OPTIONAL_SUPPORTED
#endif
#pragma once

#include <system_error>  // std::error_code, std::system_error
#include <string>  //  std::string
#include <sqlite3.h>
#include <stdexcept>
#include <sstream>  //  std::ostringstream

        namespace sqlite_orm {

    enum class orm_error_code {
        not_found = 1,
        type_is_not_mapped_to_storage,
        trying_to_dereference_null_iterator,
        too_many_tables_specified,
        incorrect_set_fields_specified,
        column_not_found,
        table_has_no_primary_key_column,
        cannot_start_a_transaction_within_a_transaction,
        no_active_transaction,
        incorrect_journal_mode_string,
        invalid_collate_argument_enum,
        failed_to_init_a_backup,
    };
}

namespace sqlite_orm {

    class orm_error_category : public std::error_category {
      public:
        const char *name() const noexcept override final {
            return "ORM error";
        }

        std::string message(int c) const override final {
            switch(static_cast<orm_error_code>(c)) {
                case orm_error_code::not_found:
                    return "Not found";
                case orm_error_code::type_is_not_mapped_to_storage:
                    return "Type is not mapped to storage";
                case orm_error_code::trying_to_dereference_null_iterator:
                    return "Trying to dereference null iterator";
                case orm_error_code::too_many_tables_specified:
                    return "Too many tables specified";
                case orm_error_code::incorrect_set_fields_specified:
                    return "Incorrect set fields specified";
                case orm_error_code::column_not_found:
                    return "Column not found";
                case orm_error_code::table_has_no_primary_key_column:
                    return "Table has no primary key column";
                case orm_error_code::cannot_start_a_transaction_within_a_transaction:
                    return "Cannot start a transaction within a transaction";
                case orm_error_code::no_active_transaction:
                    return "No active transaction";
                case orm_error_code::invalid_collate_argument_enum:
                    return "Invalid collate_argument enum";
                case orm_error_code::failed_to_init_a_backup:
                    return "Failed to init a backup";
                default:
                    return "unknown error";
            }
        }
    };

    class sqlite_error_category : public std::error_category {
      public:
        const char *name() const noexcept override final {
            return "SQLite error";
        }

        std::string message(int c) const override final {
            return sqlite3_errstr(c);
        }
    };

    inline const orm_error_category &get_orm_error_category() {
        static orm_error_category res;
        return res;
    }

    inline const sqlite_error_category &get_sqlite_error_category() {
        static sqlite_error_category res;
        return res;
    }

    template<typename... T>
    std::string get_error_message(sqlite3 *db, T &&... args) {
        std::ostringstream stream;
        using unpack = int[];
        static_cast<void>(unpack{0, (static_cast<void>(static_cast<void>(stream << args)), 0)...});
        stream << sqlite3_errmsg(db);
        return stream.str();
    }

    template<typename... T>
    [[noreturn]] void throw_error(sqlite3 *db, T &&... args) {
        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                get_error_message(db, std::forward<T>(args)...));
    }
}

namespace std {
    template<>
    struct is_error_code_enum<sqlite_orm::orm_error_code> : std::true_type {};

    inline std::error_code make_error_code(sqlite_orm::orm_error_code errorCode) {
        return std::error_code(static_cast<int>(errorCode), sqlite_orm::get_orm_error_category());
    }
}
#pragma once

#include <map>  //  std::map
#include <string>  //  std::string
#include <regex>  //  std::regex, std::regex_match
#include <memory>  //  std::make_unique, std::unique_ptr
#include <vector>  //  std::vector
#include <cctype>  //  std::toupper

namespace sqlite_orm {
    using int64 = sqlite_int64;
    using uint64 = sqlite_uint64;

    //  numeric and real are the same for c++
    enum class sqlite_type {
        INTEGER,
        TEXT,
        BLOB,
        REAL,
    };

    /**
     *  @param str case doesn't matter - it is uppercased before comparing.
     */
    inline std::unique_ptr<sqlite_type> to_sqlite_type(const std::string &str) {
        auto asciiStringToUpper = [](std::string &s) {
            std::transform(s.begin(), s.end(), s.begin(), [](char c) {
                return std::toupper(c);
            });
        };
        auto upperStr = str;
        asciiStringToUpper(upperStr);

        static std::map<sqlite_type, std::vector<std::regex>> typeMap = {
            {sqlite_type::INTEGER,
             {
                 std::regex("INT"),
                 std::regex("INT.*"),
                 std::regex("TINYINT"),
                 std::regex("SMALLINT"),
                 std::regex("MEDIUMINT"),
                 std::regex("BIGINT"),
                 std::regex("UNSIGNED BIG INT"),
                 std::regex("INT2"),
                 std::regex("INT8"),
             }},
            {sqlite_type::TEXT,
             {
                 std::regex("CHARACTER\\([[:digit:]]+\\)"),
                 std::regex("VARCHAR\\([[:digit:]]+\\)"),
                 std::regex("VARYING CHARACTER\\([[:digit:]]+\\)"),
                 std::regex("NCHAR\\([[:digit:]]+\\)"),
                 std::regex("NATIVE CHARACTER\\([[:digit:]]+\\)"),
                 std::regex("NVARCHAR\\([[:digit:]]+\\)"),
                 std::regex("CLOB"),
                 std::regex("TEXT"),
             }},
            {sqlite_type::BLOB,
             {
                 std::regex("BLOB"),
             }},
            {sqlite_type::REAL,
             {
                 std::regex("REAL"),
                 std::regex("DOUBLE"),
                 std::regex("DOUBLE PRECISION"),
                 std::regex("FLOAT"),
                 std::regex("NUMERIC"),
                 std::regex("DECIMAL\\([[:digit:]]+,[[:digit:]]+\\)"),
                 std::regex("BOOLEAN"),
                 std::regex("DATE"),
                 std::regex("DATETIME"),
             }},
        };
        for(auto &p: typeMap) {
            for(auto &r: p.second) {
                if(std::regex_match(upperStr, r)) {
                    return std::make_unique<sqlite_type>(p.first);
                }
            }
        }

        return {};
    }
}
#pragma once

#include <tuple>  //  std::tuple, std::get
#include <type_traits>  //  std::false_type, std::true_type

// #include "static_magic.h"

#include <type_traits>  //  std::false_type, std::true_type, std::integral_constant

namespace sqlite_orm {

    //  got from here
    //  https://stackoverflow.com/questions/37617677/implementing-a-compile-time-static-if-logic-for-different-string-types-in-a-co
    namespace internal {

        template<typename T, typename F>
        auto static_if(std::true_type, T t, F) {
            return std::move(t);
        }

        template<typename T, typename F>
        auto static_if(std::false_type, T, F f) {
            return std::move(f);
        }

        template<bool B, typename T, typename F>
        auto static_if(T t, F f) {
            return static_if(std::integral_constant<bool, B>{}, std::move(t), std::move(f));
        }

        template<bool B, typename T>
        auto static_if(T t) {
            return static_if(std::integral_constant<bool, B>{}, t, [](auto &&...) {});
        }
    }

}

namespace sqlite_orm {

    //  got from here http://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
    namespace tuple_helper {

        template<typename T, typename Tuple>
        struct has_type;

        template<typename T>
        struct has_type<T, std::tuple<>> : std::false_type {};

        template<typename T, typename U, typename... Ts>
        struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

        template<typename T, typename... Ts>
        struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

        template<typename T, typename Tuple>
        using tuple_contains_type = typename has_type<T, Tuple>::type;

        template<size_t N, class... Args>
        struct iterator {

            template<class L>
            void operator()(const std::tuple<Args...> &t, const L &l, bool reverse = true) {
                if(reverse) {
                    l(std::get<N>(t));
                    iterator<N - 1, Args...>()(t, l, reverse);
                } else {
                    iterator<N - 1, Args...>()(t, l, reverse);
                    l(std::get<N>(t));
                }
            }
        };

        template<class... Args>
        struct iterator<0, Args...> {

            template<class L>
            void operator()(const std::tuple<Args...> &t, const L &l, bool /*reverse*/ = true) {
                l(std::get<0>(t));
            }
        };

        template<size_t N>
        struct iterator<N> {

            template<class L>
            void operator()(const std::tuple<> &, const L &, bool /*reverse*/ = true) {
                //..
            }
        };

        template<size_t N, size_t I, class L, class R>
        void move_tuple_impl(L &lhs, R &rhs) {
            std::get<I>(lhs) = std::move(std::get<I>(rhs));
            internal::static_if<std::integral_constant<bool, N != I + 1>{}>([](auto &lhs, auto &rhs) {
                move_tuple_impl<N, I + 1>(lhs, rhs);
            })(lhs, rhs);
        }
    }

    namespace internal {

        template<size_t N, class L, class R>
        void move_tuple(L &lhs, R &rhs) {
            using bool_type = std::integral_constant<bool, N != 0>;
            static_if<bool_type{}>([](auto &lhs, auto &rhs) {
                tuple_helper::move_tuple_impl<N, 0>(lhs, rhs);
            })(lhs, rhs);
        }

        template<class L, class... Args>
        void iterate_tuple(const std::tuple<Args...> &t, const L &l) {
            using tuple_type = std::tuple<Args...>;
            tuple_helper::iterator<std::tuple_size<tuple_type>::value - 1, Args...>()(t, l, false);
        }

        template<typename... input_t>
        using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

        template<class... Args>
        struct conc_tuple {
            using type = tuple_cat_t<Args...>;
        };

        template<class T, template<class> class C>
        struct count_tuple;

        template<template<class> class C>
        struct count_tuple<std::tuple<>, C> {
            static constexpr const int value = 0;
        };

        template<class H, class... Args, template<class> class C>
        struct count_tuple<std::tuple<H, Args...>, C> {
            static constexpr const int value = C<H>::value + count_tuple<std::tuple<Args...>, C>::value;
        };
    }
}
#pragma once

#include <string>  //  std::string
#include <memory>  //  std::shared_ptr, std::unique_ptr
#include <vector>  //  std::vector
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

namespace sqlite_orm {

    /**
     *  This class accepts c++ type and transfers it to sqlite name (int -> INTEGER, std::string -> TEXT)
     */
    template<class T, typename Enable = void>
    struct type_printer;

    struct integer_printer {
        inline const std::string &print() {
            static const std::string res = "INTEGER";
            return res;
        }
    };

    struct text_printer {
        inline const std::string &print() {
            static const std::string res = "TEXT";
            return res;
        }
    };

    struct real_printer {
        inline const std::string &print() {
            static const std::string res = "REAL";
            return res;
        }
    };

    struct blob_printer {
        inline const std::string &print() {
            static const std::string res = "BLOB";
            return res;
        }
    };

    // Note unsigned/signed char and simple char used for storing integer values, not char values.
    template<>
    struct type_printer<unsigned char, void> : public integer_printer {};

    template<>
    struct type_printer<signed char, void> : public integer_printer {};

    template<>
    struct type_printer<char, void> : public integer_printer {};

    template<>
    struct type_printer<unsigned short int, void> : public integer_printer {};

    template<>
    struct type_printer<short, void> : public integer_printer {};

    template<>
    struct type_printer<unsigned int, void> : public integer_printer {};

    template<>
    struct type_printer<int, void> : public integer_printer {};

    template<>
    struct type_printer<unsigned long, void> : public integer_printer {};

    template<>
    struct type_printer<long, void> : public integer_printer {};

    template<>
    struct type_printer<unsigned long long, void> : public integer_printer {};

    template<>
    struct type_printer<long long, void> : public integer_printer {};

    template<>
    struct type_printer<bool, void> : public integer_printer {};

    template<>
    struct type_printer<std::string, void> : public text_printer {};

    template<>
    struct type_printer<std::wstring, void> : public text_printer {};

    template<>
    struct type_printer<const char *, void> : public text_printer {};

    template<>
    struct type_printer<float, void> : public real_printer {};

    template<>
    struct type_printer<double, void> : public real_printer {};

    template<class T>
    struct type_printer<std::shared_ptr<T>, void> : public type_printer<T> {};

    template<class T>
    struct type_printer<std::unique_ptr<T>, void> : public type_printer<T> {};

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct type_printer<std::optional<T>, void> : public type_printer<T> {};
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

    template<>
    struct type_printer<std::vector<char>, void> : public blob_printer {};
}
#pragma once

namespace sqlite_orm {

    namespace internal {

        enum class collate_argument {
            binary,
            nocase,
            rtrim,
        };
    }

}
#pragma once

#include <string>  //  std::string
#include <tuple>  //  std::tuple, std::make_tuple
#include <sstream>  //  std::stringstream
#include <type_traits>  //  std::is_base_of, std::false_type, std::true_type
#include <ostream>  //  std::ostream

namespace sqlite_orm {

    namespace constraints {

        /**
         *  AUTOINCREMENT constraint class.
         */
        struct autoincrement_t {

            operator std::string() const {
                return "AUTOINCREMENT";
            }
        };

        struct primary_key_base {
            enum class order_by {
                unspecified,
                ascending,
                descending,
            };

            order_by asc_option = order_by::unspecified;

            operator std::string() const {
                std::string res = "PRIMARY KEY";
                switch(this->asc_option) {
                    case order_by::ascending:
                        res += " ASC";
                        break;
                    case order_by::descending:
                        res += " DESC";
                        break;
                    default:
                        break;
                }
                return res;
            }
        };

        /**
         *  PRIMARY KEY constraint class.
         *  Cs is parameter pack which contains columns (member pointers and/or function pointers). Can be empty when
         * used withen `make_column` function.
         */
        template<class... Cs>
        struct primary_key_t : primary_key_base {
            using order_by = primary_key_base::order_by;

            std::tuple<Cs...> columns;

            primary_key_t(decltype(columns) c) : columns(std::move(c)) {}

            using field_type = void;  //  for column iteration. Better be deleted
            using constraints_type = std::tuple<>;

            primary_key_t<Cs...> asc() const {
                auto res = *this;
                res.asc_option = order_by::ascending;
                return res;
            }

            primary_key_t<Cs...> desc() const {
                auto res = *this;
                res.asc_option = order_by::descending;
                return res;
            }
        };

        /**
         *  UNIQUE constraint class.
         */
        struct unique_t {

            operator std::string() const {
                return "UNIQUE";
            }
        };

        /**
         *  DEFAULT constraint class.
         *  T is a value type.
         */
        template<class T>
        struct default_t {
            using value_type = T;

            value_type value;

            operator std::string() const {
                return "DEFAULT";
            }
        };

#if SQLITE_VERSION_NUMBER >= 3006019

        /**
         *  FOREIGN KEY constraint class.
         *  Cs are columns which has foreign key
         *  Rs are column which C references to
         *  Available in SQLite 3.6.19 or higher
         */

        template<class A, class B>
        struct foreign_key_t;

        enum class foreign_key_action {
            none,  //  not specified
            no_action,
            restrict_,
            set_null,
            set_default,
            cascade,
        };

        inline std::ostream &operator<<(std::ostream &os, foreign_key_action action) {
            switch(action) {
                case decltype(action)::no_action:
                    os << "NO ACTION";
                    break;
                case decltype(action)::restrict_:
                    os << "RESTRICT";
                    break;
                case decltype(action)::set_null:
                    os << "SET NULL";
                    break;
                case decltype(action)::set_default:
                    os << "SET DEFAULT";
                    break;
                case decltype(action)::cascade:
                    os << "CASCADE";
                    break;
                case decltype(action)::none:
                    break;
            }
            return os;
        }

        struct on_update_delete_base {
            const bool update;  //  true if update and false if delete

            operator std::string() const {
                if(this->update) {
                    return "ON UPDATE";
                } else {
                    return "ON DELETE";
                }
            }
        };

        /**
         *  F - foreign key class
         */
        template<class F>
        struct on_update_delete_t : on_update_delete_base {
            using foreign_key_type = F;

            const foreign_key_type &fk;

            on_update_delete_t(decltype(fk) fk_, decltype(update) update, foreign_key_action action_) :
                on_update_delete_base{update}, fk(fk_), _action(action_) {}

            foreign_key_action _action = foreign_key_action::none;

            foreign_key_type no_action() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::no_action;
                } else {
                    res.on_delete._action = foreign_key_action::no_action;
                }
                return res;
            }

            foreign_key_type restrict_() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::restrict_;
                } else {
                    res.on_delete._action = foreign_key_action::restrict_;
                }
                return res;
            }

            foreign_key_type set_null() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::set_null;
                } else {
                    res.on_delete._action = foreign_key_action::set_null;
                }
                return res;
            }

            foreign_key_type set_default() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::set_default;
                } else {
                    res.on_delete._action = foreign_key_action::set_default;
                }
                return res;
            }

            foreign_key_type cascade() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::cascade;
                } else {
                    res.on_delete._action = foreign_key_action::cascade;
                }
                return res;
            }

            operator bool() const {
                return this->_action != decltype(this->_action)::none;
            }
        };

        template<class... Cs, class... Rs>
        struct foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> {
            using columns_type = std::tuple<Cs...>;
            using references_type = std::tuple<Rs...>;
            using self = foreign_key_t<columns_type, references_type>;

            columns_type columns;
            references_type references;

            on_update_delete_t<self> on_update;
            on_update_delete_t<self> on_delete;

            static_assert(std::tuple_size<columns_type>::value == std::tuple_size<references_type>::value,
                          "Columns size must be equal to references tuple");

            foreign_key_t(columns_type columns_, references_type references_) :
                columns(std::move(columns_)), references(std::move(references_)),
                on_update(*this, true, foreign_key_action::none), on_delete(*this, false, foreign_key_action::none) {}

            foreign_key_t(const self &other) :
                columns(other.columns), references(other.references), on_update(*this, true, other.on_update._action),
                on_delete(*this, false, other.on_delete._action) {}

            self &operator=(const self &other) {
                this->columns = other.columns;
                this->references = other.references;
                this->on_update = {*this, true, other.on_update._action};
                this->on_delete = {*this, false, other.on_delete._action};
                return *this;
            }

            using field_type = void;  //  for column iteration. Better be deleted
            using constraints_type = std::tuple<>;

            template<class L>
            void for_each_column(const L &) {}

            template<class... Opts>
            constexpr bool has_every() const {
                return false;
            }
        };

        /**
         *  Cs can be a class member pointer, a getter function member pointer or setter
         *  func member pointer
         *  Available in SQLite 3.6.19 or higher
         */
        template<class... Cs>
        struct foreign_key_intermediate_t {
            using tuple_type = std::tuple<Cs...>;

            tuple_type columns;

            foreign_key_intermediate_t(tuple_type columns_) : columns(std::move(columns_)) {}

            template<class... Rs>
            foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> references(Rs... references) {
                return {std::move(this->columns), std::make_tuple(std::forward<Rs>(references)...)};
            }
        };
#endif

        struct collate_t {
            internal::collate_argument argument = internal::collate_argument::binary;

            collate_t(internal::collate_argument argument_) : argument(argument_) {}

            operator std::string() const {
                std::string res = "COLLATE " + this->string_from_collate_argument(this->argument);
                return res;
            }

            static std::string string_from_collate_argument(internal::collate_argument argument) {
                switch(argument) {
                    case decltype(argument)::binary:
                        return "BINARY";
                    case decltype(argument)::nocase:
                        return "NOCASE";
                    case decltype(argument)::rtrim:
                        return "RTRIM";
                }
                throw std::system_error(std::make_error_code(orm_error_code::invalid_collate_argument_enum));
            }
        };

        template<class T>
        struct is_constraint : std::false_type {};

        template<>
        struct is_constraint<autoincrement_t> : std::true_type {};

        template<class... Cs>
        struct is_constraint<primary_key_t<Cs...>> : std::true_type {};

        template<>
        struct is_constraint<unique_t> : std::true_type {};

        template<class T>
        struct is_constraint<default_t<T>> : std::true_type {};

        template<class C, class R>
        struct is_constraint<foreign_key_t<C, R>> : std::true_type {};

        template<>
        struct is_constraint<collate_t> : std::true_type {};

        template<class... Args>
        struct constraints_size;

        template<>
        struct constraints_size<> {
            static constexpr const int value = 0;
        };

        template<class H, class... Args>
        struct constraints_size<H, Args...> {
            static constexpr const int value = is_constraint<H>::value + constraints_size<Args...>::value;
        };
    }

#if SQLITE_VERSION_NUMBER >= 3006019

    /**
     *  FOREIGN KEY constraint construction function that takes member pointer as argument
     *  Available in SQLite 3.6.19 or higher
     */
    template<class... Cs>
    constraints::foreign_key_intermediate_t<Cs...> foreign_key(Cs... columns) {
        return {std::make_tuple(std::forward<Cs>(columns)...)};
    }
#endif

    /**
     *  UNIQUE constraint builder function.
     */
    inline constraints::unique_t unique() {
        return {};
    }

    inline constraints::autoincrement_t autoincrement() {
        return {};
    }

    template<class... Cs>
    inline constraints::primary_key_t<Cs...> primary_key(Cs... cs) {
        using ret_type = constraints::primary_key_t<Cs...>;
        return ret_type(std::make_tuple(cs...));
    }

    template<class T>
    constraints::default_t<T> default_value(T t) {
        return {std::move(t)};
    }

    inline constraints::collate_t collate_nocase() {
        return {internal::collate_argument::nocase};
    }

    inline constraints::collate_t collate_binary() {
        return {internal::collate_argument::binary};
    }

    inline constraints::collate_t collate_rtrim() {
        return {internal::collate_argument::rtrim};
    }

    namespace internal {

        /**
         *  FOREIGN KEY traits. Common case
         */
        template<class T>
        struct is_foreign_key : std::false_type {};

        /**
         *  FOREIGN KEY traits. Specialized case
         */
        template<class C, class R>
        struct is_foreign_key<constraints::foreign_key_t<C, R>> : std::true_type {};

        /**
         *  PRIMARY KEY traits. Common case
         */
        template<class T>
        struct is_primary_key : public std::false_type {};

        /**
         *  PRIMARY KEY traits. Specialized case
         */
        template<class... Cs>
        struct is_primary_key<constraints::primary_key_t<Cs...>> : public std::true_type {};
    }

}
#pragma once

#include <type_traits>  //  std::false_type, std::true_type
#include <memory>  //  std::shared_ptr, std::unique_ptr
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

namespace sqlite_orm {

    /**
     *  This is class that tells `sqlite_orm` that type is nullable. Nullable types
     *  are mapped to sqlite database as `NULL` and not-nullable are mapped as `NOT NULL`.
     *  Default nullability status for all types is `NOT NULL`. So if you want to map
     *  custom type as `NULL` (for example: boost::optional) you have to create a specialiation
     *  of type_is_nullable for your type and derive from `std::true_type`.
     */
    template<class T>
    struct type_is_nullable : public std::false_type {
        bool operator()(const T &) const {
            return true;
        }
    };

    /**
     *  This is a specialization for std::shared_ptr. std::shared_ptr is nullable in sqlite_orm.
     */
    template<class T>
    struct type_is_nullable<std::shared_ptr<T>> : public std::true_type {
        bool operator()(const std::shared_ptr<T> &t) const {
            return static_cast<bool>(t);
        }
    };

    /**
     *  This is a specialization for std::unique_ptr. std::unique_ptr is nullable too.
     */
    template<class T>
    struct type_is_nullable<std::unique_ptr<T>> : public std::true_type {
        bool operator()(const std::unique_ptr<T> &t) const {
            return static_cast<bool>(t);
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  This is a specialization for std::optional. std::optional is nullable.
     */
    template<class T>
    struct type_is_nullable<std::optional<T>> : public std::true_type {
        bool operator()(const std::optional<T> &t) const {
            return t.has_value();
        }
    };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

}
#pragma once

#include <memory>  //  std::unique_ptr
#include <string>  //  std::string
#include <sstream>  //  std::stringstream

// #include "constraints.h"

namespace sqlite_orm {

    namespace internal {

        template<class T>
        std::string serialize(const T &t);

        /**
         *  This class is used in tuple interation to know whether tuple constains `default_value_t`
         *  constraint class and what it's value if it is
         */
        struct default_value_extractor {

            template<class A>
            std::unique_ptr<std::string> operator()(const A &) {
                return {};
            }

            template<class T>
            std::unique_ptr<std::string> operator()(const constraints::default_t<T> &t) {
                return std::make_unique<std::string>(serialize(t.value));
            }
        };

    }

}
#pragma once

#include <type_traits>  //  std::false_type, std::true_type

namespace sqlite_orm {

    namespace internal {

        /**
         *  Inherit this class to support arithmetic types overloading
         */
        struct arithmetic_t {};

        template<class L, class R, class... Ds>
        struct binary_operator : Ds... {
            using left_type = L;
            using right_type = R;

            left_type lhs;
            right_type rhs;

            binary_operator(left_type lhs_, right_type rhs_) : lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}
        };

        struct conc_string {
            operator std::string() const {
                return "||";
            }
        };

        /**
         *  Result of concatenation || operator
         */
        template<class L, class R>
        using conc_t = binary_operator<L, R, conc_string>;

        struct add_string {
            operator std::string() const {
                return "+";
            }
        };

        /**
         *  Result of addition + operator
         */
        template<class L, class R>
        using add_t = binary_operator<L, R, add_string, arithmetic_t>;

        struct sub_string {
            operator std::string() const {
                return "-";
            }
        };

        /**
         *  Result of substitute - operator
         */
        template<class L, class R>
        using sub_t = binary_operator<L, R, sub_string, arithmetic_t>;

        struct mul_string {
            operator std::string() const {
                return "*";
            }
        };

        /**
         *  Result of multiply * operator
         */
        template<class L, class R>
        using mul_t = binary_operator<L, R, mul_string, arithmetic_t>;

        struct div_string {
            operator std::string() const {
                return "/";
            }
        };

        /**
         *  Result of divide / operator
         */
        template<class L, class R>
        using div_t = binary_operator<L, R, div_string, arithmetic_t>;

        struct mod_string {
            operator std::string() const {
                return "%";
            }
        };

        /**
         *  Result of mod % operator
         */
        template<class L, class R>
        using mod_t = binary_operator<L, R, mod_string, arithmetic_t>;

        struct assign_string {
            operator std::string() const {
                return "=";
            }
        };

        /**
         *  Result of assign = operator
         */
        template<class L, class R>
        using assign_t = binary_operator<L, R, assign_string>;

        /**
         *  Assign operator traits. Common case
         */
        template<class T>
        struct is_assign_t : public std::false_type {};

        /**
         *  Assign operator traits. Specialized case
         */
        template<class L, class R>
        struct is_assign_t<assign_t<L, R>> : public std::true_type {};

        /**
         *  Is not an operator but a result of c(...) function. Has operator= overloaded which returns assign_t
         */
        template<class T>
        struct expression_t {
            T t;

            expression_t(T t_) : t(std::move(t_)) {}

            template<class R>
            assign_t<T, R> operator=(R r) const {
                return {this->t, std::move(r)};
            }

            assign_t<T, std::nullptr_t> operator=(std::nullptr_t) const {
                return {this->t, nullptr};
            }
        };

    }

    /**
     *  Public interface for syntax sugar for columns. Example: `where(c(&User::id) == 5)` or
     * `storage.update(set(c(&User::name) = "Dua Lipa"));
     */
    template<class T>
    internal::expression_t<T> c(T t) {
        return {std::move(t)};
    }

    /**
     *  Public interface for || concatenation operator. Example: `select(conc(&User::name, "@gmail.com"));` => SELECT
     * name || '@gmail.com' FROM users
     */
    template<class L, class R>
    internal::conc_t<L, R> conc(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for + operator. Example: `select(add(&User::age, 100));` => SELECT age + 100 FROM users
     */
    template<class L, class R>
    internal::add_t<L, R> add(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for - operator. Example: `select(add(&User::age, 1));` => SELECT age - 1 FROM users
     */
    template<class L, class R>
    internal::sub_t<L, R> sub(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::mul_t<L, R> mul(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::div_t<L, R> div(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::mod_t<L, R> mod(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::assign_t<L, R> assign(L l, R r) {
        return {std::move(l), std::move(r)};
    }

}
#pragma once

#include <tuple>  //  std::tuple
#include <string>  //  std::string
#include <memory>  //  std::unique_ptr
#include <type_traits>  //  std::true_type, std::false_type, std::is_same, std::enable_if, std::is_member_pointer, std::is_member_function_pointer

// #include "type_is_nullable.h"

// #include "tuple_helper.h"

// #include "default_value_extractor.h"

// #include "constraints.h"

namespace sqlite_orm {

    namespace internal {

        struct column_base {

            /**
             *  Column name. Specified during construction in `make_column`.
             */
            const std::string name;
        };

        /**
         *  This class stores single column info. column_t is a pair of [column_name:member_pointer] mapped to a storage
         *  O is a mapped class, e.g. User
         *  T is a mapped class'es field type, e.g. &User::name
         *  Op... is a constraints pack, e.g. primary_key_t, autoincrement_t etc
         */
        template<class O, class T, class G /* = const T& (O::*)() const*/, class S /* = void (O::*)(T)*/, class... Op>
        struct column_t : column_base {
            using object_type = O;
            using field_type = T;
            using constraints_type = std::tuple<Op...>;
            using member_pointer_t = field_type object_type::*;
            using getter_type = G;
            using setter_type = S;

            /**
             *  Member pointer used to read/write member
             */
            member_pointer_t member_pointer /* = nullptr*/;

            /**
             *  Getter member function pointer to get a value. If member_pointer is null than
             *  `getter` and `setter` must be not null
             */
            getter_type getter /* = nullptr*/;

            /**
             *  Setter member function
             */
            setter_type setter /* = nullptr*/;

            /**
             *  Constraints tuple
             */
            constraints_type constraints;

            column_t(std::string name,
                     member_pointer_t member_pointer_,
                     getter_type getter_,
                     setter_type setter_,
                     constraints_type constraints_) :
                column_base{std::move(name)},
                member_pointer(member_pointer_), getter(getter_), setter(setter_),
                constraints(std::move(constraints_)) {}

            /**
             *  Simplified interface for `NOT NULL` constraint
             */
            bool not_null() const {
                return !type_is_nullable<field_type>::value;
            }

            template<class Opt>
            constexpr bool has() const {
                return tuple_helper::tuple_contains_type<Opt, constraints_type>::value;
            }

            template<class O1, class O2, class... Opts>
            constexpr bool has_every() const {
                if(has<O1>() && has<O2>()) {
                    return true;
                } else {
                    return has_every<Opts...>();
                }
            }

            template<class O1>
            constexpr bool has_every() const {
                return has<O1>();
            }

            /**
             *  Simplified interface for `DEFAULT` constraint
             *  @return string representation of default value if it exists otherwise nullptr
             */
            std::unique_ptr<std::string> default_value() const {
                std::unique_ptr<std::string> res;
                iterate_tuple(this->constraints, [&res](auto &v) {
                    auto dft = internal::default_value_extractor()(v);
                    if(dft) {
                        res = std::move(dft);
                    }
                });
                return res;
            }
        };

        /**
         *  Column traits. Common case.
         */
        template<class T>
        struct is_column : public std::false_type {};

        /**
         *  Column traits. Specialized case case.
         */
        template<class O, class T, class... Op>
        struct is_column<column_t<O, T, Op...>> : public std::true_type {};

        template<class T, class SFINAE = void>
        struct is_field_member_pointer : std::false_type {};

        template<class T>
        struct is_field_member_pointer<T,
                                       typename std::enable_if<std::is_member_pointer<T>::value &&
                                                               !std::is_member_function_pointer<T>::value>::type>
            : std::true_type {};

        /**
         *  Getters aliases
         */
        template<class O, class T>
        using getter_by_value_const = T (O::*)() const;

        template<class O, class T>
        using getter_by_value = T (O::*)();

        template<class O, class T>
        using getter_by_ref_const = T &(O::*)() const;

        template<class O, class T>
        using getter_by_ref = T &(O::*)();

        template<class O, class T>
        using getter_by_const_ref_const = const T &(O::*)() const;

        template<class O, class T>
        using getter_by_const_ref = const T &(O::*)();

        /**
         *  Setters aliases
         */
        template<class O, class T>
        using setter_by_value = void (O::*)(T);

        template<class O, class T>
        using setter_by_ref = void (O::*)(T &);

        template<class O, class T>
        using setter_by_const_ref = void (O::*)(const T &);

        template<class T>
        struct is_getter : std::false_type {};

        template<class O, class T>
        struct is_getter<getter_by_value_const<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_getter<getter_by_value<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_getter<getter_by_ref_const<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_getter<getter_by_ref<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_getter<getter_by_const_ref_const<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_getter<getter_by_const_ref<O, T>> : std::true_type {};

        template<class T>
        struct is_setter : std::false_type {};

        template<class O, class T>
        struct is_setter<setter_by_value<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_setter<setter_by_ref<O, T>> : std::true_type {};

        template<class O, class T>
        struct is_setter<setter_by_const_ref<O, T>> : std::true_type {};

        template<class T>
        struct getter_traits;

        template<class O, class T>
        struct getter_traits<getter_by_value_const<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = false;
        };

        template<class O, class T>
        struct getter_traits<getter_by_value<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = false;
        };

        template<class O, class T>
        struct getter_traits<getter_by_ref_const<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = true;
        };

        template<class O, class T>
        struct getter_traits<getter_by_ref<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = true;
        };

        template<class O, class T>
        struct getter_traits<getter_by_const_ref_const<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = true;
        };

        template<class O, class T>
        struct getter_traits<getter_by_const_ref<O, T>> {
            using object_type = O;
            using field_type = T;

            static constexpr const bool returns_lvalue = true;
        };

        template<class T>
        struct setter_traits;

        template<class O, class T>
        struct setter_traits<setter_by_value<O, T>> {
            using object_type = O;
            using field_type = T;
        };

        template<class O, class T>
        struct setter_traits<setter_by_ref<O, T>> {
            using object_type = O;
            using field_type = T;
        };

        template<class O, class T>
        struct setter_traits<setter_by_const_ref<O, T>> {
            using object_type = O;
            using field_type = T;
        };
    }

    /**
     *  Column builder function. You should use it to create columns instead of constructor
     */
    template<class O,
             class T,
             typename = typename std::enable_if<!std::is_member_function_pointer<T O::*>::value>::type,
             class... Op>
    internal::column_t<O, T, const T &(O::*)() const, void (O::*)(T), Op...>
    make_column(const std::string &name, T O::*m, Op... constraints) {
        static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        return {name, m, nullptr, nullptr, std::make_tuple(constraints...)};
    }

    /**
     *  Column builder function with setter and getter. You should use it to create columns instead of constructor
     */
    template<class G,
             class S,
             typename = typename std::enable_if<internal::is_getter<G>::value>::type,
             typename = typename std::enable_if<internal::is_setter<S>::value>::type,
             class... Op>
    internal::column_t<typename internal::setter_traits<S>::object_type,
                       typename internal::setter_traits<S>::field_type,
                       G,
                       S,
                       Op...>
    make_column(const std::string &name, S setter, G getter, Op... constraints) {
        static_assert(std::is_same<typename internal::setter_traits<S>::field_type,
                                   typename internal::getter_traits<G>::field_type>::value,
                      "Getter and setter must get and set same data type");
        static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        return {name, nullptr, getter, setter, std::make_tuple(constraints...)};
    }

    /**
     *  Column builder function with getter and setter (reverse order). You should use it to create columns instead of
     * constructor
     */
    template<class G,
             class S,
             typename = typename std::enable_if<internal::is_getter<G>::value>::type,
             typename = typename std::enable_if<internal::is_setter<S>::value>::type,
             class... Op>
    internal::column_t<typename internal::setter_traits<S>::object_type,
                       typename internal::setter_traits<S>::field_type,
                       G,
                       S,
                       Op...>
    make_column(const std::string &name, G getter, S setter, Op... constraints) {
        static_assert(std::is_same<typename internal::setter_traits<S>::field_type,
                                   typename internal::getter_traits<G>::field_type>::value,
                      "Getter and setter must get and set same data type");
        static_assert(constraints::constraints_size<Op...>::value == std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        return {name, nullptr, getter, setter, std::make_tuple(constraints...)};
    }

}
#pragma once

#include <string>  //  std::string
#include <sstream>  //  std::stringstream
#include <vector>  //  std::vector
#include <cstddef>  //  std::nullptr_t
#include <memory>  //  std::shared_ptr, std::unique_ptr
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

namespace sqlite_orm {

    /**
     *  Is used to print members mapped to objects in storage_t::dump member function.
     *  Other developers can create own specialization to map custom types
     */
    template<class T>
    struct field_printer {
        std::string operator()(const T &t) const {
            std::stringstream stream;
            stream << t;
            return stream.str();
        }
    };

    /**
     *  Upgrade to integer is required when using unsigned char(uint8_t)
     */
    template<>
    struct field_printer<unsigned char> {
        std::string operator()(const unsigned char &t) const {
            std::stringstream stream;
            stream << +t;
            return stream.str();
        }
    };

    /**
     *  Upgrade to integer is required when using signed char(int8_t)
     */
    template<>
    struct field_printer<signed char> {
        std::string operator()(const signed char &t) const {
            std::stringstream stream;
            stream << +t;
            return stream.str();
        }
    };

    /**
     *  char is neigher signer char nor unsigned char so it has its own specialization
     */
    template<>
    struct field_printer<char> {
        std::string operator()(const char &t) const {
            std::stringstream stream;
            stream << +t;
            return stream.str();
        }
    };

    template<>
    struct field_printer<std::string> {
        std::string operator()(const std::string &t) const {
            return t;
        }
    };

    template<>
    struct field_printer<std::vector<char>> {
        std::string operator()(const std::vector<char> &t) const {
            std::stringstream ss;
            ss << std::hex;
            for(auto c: t) {
                ss << c;
            }
            return ss.str();
        }
    };

    template<>
    struct field_printer<std::nullptr_t> {
        std::string operator()(const std::nullptr_t &) const {
            return "null";
        }
    };

    template<class T>
    struct field_printer<std::shared_ptr<T>> {
        std::string operator()(const std::shared_ptr<T> &t) const {
            if(t) {
                return field_printer<T>()(*t);
            } else {
                return field_printer<std::nullptr_t>()(nullptr);
            }
        }
    };

    template<class T>
    struct field_printer<std::unique_ptr<T>> {
        std::string operator()(const std::unique_ptr<T> &t) const {
            if(t) {
                return field_printer<T>()(*t);
            } else {
                return field_printer<std::nullptr_t>()(nullptr);
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct field_printer<std::optional<T>> {
        std::string operator()(const std::optional<T> &t) const {
            if(t.has_value()) {
                return field_printer<T>()(*t);
            } else {
                return field_printer<std::nullptr_t>()(nullptr);
            }
        }
    };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
}
#pragma once

#include <string>  //  std::string
#include <type_traits>  //  std::enable_if, std::is_same
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple

// #include "collate_argument.h"

// #include "constraints.h"

// #include "optional_container.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  This is a cute class which allows storing something or nothing
         *  depending on template argument. Useful for optional class members
         */
        template<class T>
        struct optional_container {
            using type = T;

            type field;

            template<class L>
            void apply(const L &l) const {
                l(this->field);
            }
        };

        template<>
        struct optional_container<void> {
            using type = void;

            template<class L>
            void apply(const L &) const {
                //..
            }
        };
    }
}

namespace sqlite_orm {

    namespace internal {
        struct arithmetic_t;
    }

    namespace conditions {

        struct limit_string {
            operator std::string() const {
                return "LIMIT";
            }
        };

        /**
         *  Stores LIMIT/OFFSET info
         */
        template<class T, bool has_offset, bool offset_is_implicit, class O>
        struct limit_t : limit_string {
            T lim;
            internal::optional_container<O> off;

            limit_t() = default;

            limit_t(decltype(lim) lim_) : lim(std::move(lim_)) {}

            limit_t(decltype(lim) lim_, decltype(off) off_) : lim(std::move(lim_)), off(std::move(off_)) {}
        };

        template<class T>
        struct is_limit : std::false_type {};

        template<class T, bool has_offset, bool offset_is_implicit, class O>
        struct is_limit<limit_t<T, has_offset, offset_is_implicit, O>> : std::true_type {};

        /**
         *  Stores OFFSET only info
         */
        template<class T>
        struct offset_t {
            T off;
        };

        template<class T>
        struct is_offset : std::false_type {};

        template<class T>
        struct is_offset<offset_t<T>> : std::true_type {};

        /**
         *  Inherit from this class if target class can be chained with other conditions with '&&' and '||' operators
         */
        struct condition_t {};

        /**
         *  Collated something
         */
        template<class T>
        struct collate_t : public condition_t {
            T expr;
            internal::collate_argument argument;

            collate_t(T expr_, internal::collate_argument argument_) : expr(expr_), argument(argument_) {}

            operator std::string() const {
                return constraints::collate_t{this->argument};
            }
        };

        struct named_collate_base {
            std::string name;

            operator std::string() const {
                return "COLLATE " + this->name;
            }
        };

        /**
         *  Collated something with custom collate function
         */
        template<class T>
        struct named_collate : named_collate_base {
            T expr;

            named_collate(T expr_, std::string name_) : named_collate_base{std::move(name_)}, expr(std::move(expr_)) {}
        };

        struct negated_condition_string {
            operator std::string() const {
                return "NOT";
            }
        };

        /**
         *  Result of not operator
         */
        template<class C>
        struct negated_condition_t : condition_t, negated_condition_string {
            C c;

            negated_condition_t(C c_) : c(std::move(c_)) {}
        };

        /**
         *  Base class for binary conditions
         */
        template<class L, class R>
        struct binary_condition : public condition_t {
            using left_type = L;
            using right_type = R;

            left_type l;
            right_type r;

            binary_condition() = default;

            binary_condition(left_type l_, right_type r_) : l(std::move(l_)), r(std::move(r_)) {}
        };

        struct and_condition_string {
            operator std::string() const {
                return "AND";
            }
        };

        /**
         *  Result of and operator
         */
        template<class L, class R>
        struct and_condition_t : binary_condition<L, R>, and_condition_string {
            using super = binary_condition<L, R>;

            using super::super;
        };

        struct or_condition_string {
            operator std::string() const {
                return "OR";
            }
        };

        /**
         *  Result of or operator
         */
        template<class L, class R>
        struct or_condition_t : binary_condition<L, R>, or_condition_string {
            using super = binary_condition<L, R>;

            using super::super;
        };

        struct is_equal_string {
            operator std::string() const {
                return "=";
            }
        };

        /**
         *  = and == operators object
         */
        template<class L, class R>
        struct is_equal_t : binary_condition<L, R>, is_equal_string {
            using self = is_equal_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<self> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }

            named_collate<self> collate(std::string name) const {
                return {*this, std::move(name)};
            }
        };

        struct is_not_equal_string {
            operator std::string() const {
                return "!=";
            }
        };

        /**
         *  != operator object
         */
        template<class L, class R>
        struct is_not_equal_t : binary_condition<L, R>, is_not_equal_string {
            using self = is_not_equal_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<self> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }
        };

        struct greater_than_string {
            operator std::string() const {
                return ">";
            }
        };

        /**
         *  > operator object.
         */
        template<class L, class R>
        struct greater_than_t : binary_condition<L, R>, greater_than_string {
            using self = greater_than_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<self> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }
        };

        struct greater_or_equal_string {
            operator std::string() const {
                return ">=";
            }
        };

        /**
         *  >= operator object.
         */
        template<class L, class R>
        struct greater_or_equal_t : binary_condition<L, R>, greater_or_equal_string {
            using self = greater_or_equal_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<self> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }
        };

        struct lesser_than_string {
            operator std::string() const {
                return "<";
            }
        };

        /**
         *  < operator object.
         */
        template<class L, class R>
        struct lesser_than_t : binary_condition<L, R>, lesser_than_string {
            using self = lesser_than_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<self> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }
        };

        struct lesser_or_equal_string {
            operator std::string() const {
                return "<=";
            }
        };

        /**
         *  <= operator object.
         */
        template<class L, class R>
        struct lesser_or_equal_t : binary_condition<L, R>, lesser_or_equal_string {
            using self = lesser_or_equal_t<L, R>;

            using binary_condition<L, R>::binary_condition;

            negated_condition_t<lesser_or_equal_t<L, R>> operator!() const {
                return {*this};
            }

            collate_t<self> collate_binary() const {
                return {*this, internal::collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, internal::collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, internal::collate_argument::rtrim};
            }
        };

        struct in_base {
            bool negative = false;  //  used in not_in

            operator std::string() const {
                if(!this->negative) {
                    return "IN";
                } else {
                    return "NOT IN";
                }
            }
        };

        /**
         *  IN operator object.
         */
        template<class L, class A>
        struct in_t : condition_t, in_base {
            using self = in_t<L, A>;

            L l;  //  left expression
            A arg;  //  in arg

            in_t(L l_, A arg_, bool negative) : in_base{negative}, l(l_), arg(std::move(arg_)) {}

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct is_null_string {
            operator std::string() const {
                return "IS NULL";
            }
        };

        /**
         *  IS NULL operator object.
         */
        template<class T>
        struct is_null_t : is_null_string {
            using self = is_null_t<T>;

            T t;

            is_null_t(T t_) : t(std::move(t_)) {}

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct is_not_null_string {
            operator std::string() const {
                return "IS NOT NULL";
            }
        };

        /**
         *  IS NOT NULL operator object.
         */
        template<class T>
        struct is_not_null_t : is_not_null_string {
            using self = is_not_null_t<T>;

            T t;

            is_not_null_t(T t_) : t(std::move(t_)) {}

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct where_string {
            operator std::string() const {
                return "WHERE";
            }
        };

        /**
         *  WHERE argument holder.
         *  C is conditions type. Can be any condition like: is_equal_t, is_null_t, exists_t etc
         */
        template<class C>
        struct where_t : where_string {
            C c;

            where_t(C c_) : c(std::move(c_)) {}
        };

        template<class T>
        struct is_where : std::false_type {};

        template<class T>
        struct is_where<where_t<T>> : std::true_type {};

        struct order_by_base {
            int asc_desc = 0;  //  1: asc, -1: desc
            std::string _collate_argument;
        };

        struct order_by_string {
            operator std::string() const {
                return "ORDER BY";
            }
        };

        /**
         *  ORDER BY argument holder.
         */
        template<class O>
        struct order_by_t : order_by_base, order_by_string {
            using self = order_by_t<O>;

            O o;

            order_by_t(O o_) : o(std::move(o_)) {}

            self asc() {
                auto res = *this;
                res.asc_desc = 1;
                return res;
            }

            self desc() {
                auto res = *this;
                res.asc_desc = -1;
                return res;
            }

            self collate_binary() const {
                auto res = *this;
                res._collate_argument = constraints::collate_t::string_from_collate_argument(
                    sqlite_orm::internal::collate_argument::binary);
                return res;
            }

            self collate_nocase() const {
                auto res = *this;
                res._collate_argument = constraints::collate_t::string_from_collate_argument(
                    sqlite_orm::internal::collate_argument::nocase);
                return res;
            }

            self collate_rtrim() const {
                auto res = *this;
                res._collate_argument =
                    constraints::collate_t::string_from_collate_argument(sqlite_orm::internal::collate_argument::rtrim);
                return res;
            }

            self collate(std::string name) const {
                auto res = *this;
                res._collate_argument = std::move(name);
                return res;
            }
        };

        /**
         *  ORDER BY pack holder.
         */
        template<class... Args>
        struct multi_order_by_t : order_by_string {
            using args_type = std::tuple<Args...>;

            args_type args;

            multi_order_by_t(args_type &&args_) : args(std::move(args_)) {}
        };

        /**
         *  S - storage class
         */
        template<class S>
        struct dynamic_order_by_t : order_by_string {
            using storage_type = S;

            struct entry_t : order_by_base {
                std::string name;

                entry_t(decltype(name) name_, int asc_desc, std::string collate_argument) :
                    order_by_base{asc_desc, move(collate_argument)}, name(move(name_)) {}
            };

            using const_iterator = typename std::vector<entry_t>::const_iterator;

            dynamic_order_by_t(const storage_type &storage_) : storage(storage_) {}

            template<class O>
            void push_back(order_by_t<O> order_by) {
                auto columnName = this->storage.string_from_expression(order_by.o, true);
                entries.emplace_back(move(columnName), order_by.asc_desc, move(order_by._collate_argument));
            }

            const_iterator begin() const {
                return this->entries.begin();
            }

            const_iterator end() const {
                return this->entries.end();
            }

            void clear() {
                this->entries.clear();
            }

          protected:
            std::vector<entry_t> entries;
            const storage_type &storage;
        };

        template<class T>
        struct is_order_by : std::false_type {};

        template<class O>
        struct is_order_by<order_by_t<O>> : std::true_type {};

        template<class... Args>
        struct is_order_by<multi_order_by_t<Args...>> : std::true_type {};

        template<class S>
        struct is_order_by<dynamic_order_by_t<S>> : std::true_type {};

        struct group_by_string {
            operator std::string() const {
                return "GROUP BY";
            }
        };

        /**
         *  GROUP BY pack holder.
         */
        template<class... Args>
        struct group_by_t : group_by_string {
            using args_type = std::tuple<Args...>;
            args_type args;

            group_by_t(args_type &&args_) : args(std::move(args_)) {}
        };

        template<class T>
        struct is_group_by : std::false_type {};

        template<class... Args>
        struct is_group_by<group_by_t<Args...>> : std::true_type {};

        struct between_string {
            operator std::string() const {
                return "BETWEEN";
            }
        };

        /**
         *  BETWEEN operator object.
         */
        template<class A, class T>
        struct between_t : condition_t, between_string {
            using expression_type = A;
            using lower_type = T;
            using upper_type = T;

            expression_type expr;
            lower_type b1;
            upper_type b2;

            between_t(expression_type expr_, lower_type b1_, upper_type b2_) :
                expr(std::move(expr_)), b1(std::move(b1_)), b2(std::move(b2_)) {}
        };

        struct like_string {
            operator std::string() const {
                return "LIKE";
            }
        };

        /**
         *  LIKE operator object.
         */
        template<class A, class T, class E>
        struct like_t : condition_t, like_string {
            using self = like_t<A, T, E>;
            using arg_t = A;
            using pattern_t = T;
            using escape_t = E;

            arg_t arg;
            pattern_t pattern;
            sqlite_orm::internal::optional_container<escape_t>
                arg3;  //  not escape cause escape exists as a function here

            like_t(arg_t arg_, pattern_t pattern_, sqlite_orm::internal::optional_container<escape_t> escape) :
                arg(std::move(arg_)), pattern(std::move(pattern_)), arg3(std::move(escape)) {}

            template<class C>
            like_t<A, T, C> escape(C c) const {
                sqlite_orm::internal::optional_container<C> arg3{std::move(c)};
                return {std::move(this->arg), std::move(this->pattern), std::move(arg3)};
            }

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct glob_string {
            operator std::string() const {
                return "GLOB";
            }
        };

        template<class A, class T>
        struct glob_t : condition_t, glob_string {
            using self = glob_t<A, T>;
            using arg_t = A;
            using pattern_t = T;

            arg_t arg;
            pattern_t pattern;

            glob_t(arg_t arg_, pattern_t pattern_) : arg(std::move(arg_)), pattern(std::move(pattern_)) {}

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct cross_join_string {
            operator std::string() const {
                return "CROSS JOIN";
            }
        };

        /**
         *  CROSS JOIN holder.
         *  T is joined type which represents any mapped table.
         */
        template<class T>
        struct cross_join_t : cross_join_string {
            using type = T;
        };

        struct natural_join_string {
            operator std::string() const {
                return "NATURAL JOIN";
            }
        };

        /**
         *  NATURAL JOIN holder.
         *  T is joined type which represents any mapped table.
         */
        template<class T>
        struct natural_join_t : natural_join_string {
            using type = T;
        };

        struct left_join_string {
            operator std::string() const {
                return "LEFT JOIN";
            }
        };

        /**
         *  LEFT JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct left_join_t : left_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            left_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct join_string {
            operator std::string() const {
                return "JOIN";
            }
        };

        /**
         *  Simple JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct join_t : join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct left_outer_join_string {
            operator std::string() const {
                return "LEFT OUTER JOIN";
            }
        };

        /**
         *  LEFT OUTER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct left_outer_join_t : left_outer_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            left_outer_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct on_string {
            operator std::string() const {
                return "ON";
            }
        };

        /**
         *  on(...) argument holder used for JOIN, LEFT JOIN, LEFT OUTER JOIN and INNER JOIN
         *  T is on type argument.
         */
        template<class T>
        struct on_t : on_string {
            using arg_type = T;

            arg_type arg;

            on_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        /**
         *  USING argument holder.
         */
        template<class F, class O>
        struct using_t {
            F O::*column = nullptr;

            operator std::string() const {
                return "USING";
            }
        };

        struct inner_join_string {
            operator std::string() const {
                return "INNER JOIN";
            }
        };

        /**
         *  INNER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct inner_join_t : inner_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            inner_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct exists_string {
            operator std::string() const {
                return "EXISTS";
            }
        };

        template<class T>
        struct exists_t : condition_t, exists_string {
            using type = T;
            using self = exists_t<type>;

            type t;

            exists_t(T t_) : t(std::move(t_)) {}

            negated_condition_t<self> operator!() const {
                return {*this};
            }
        };

        struct having_string {
            operator std::string() const {
                return "HAVING";
            }
        };

        /**
         *  HAVING holder.
         *  T is having argument type.
         */
        template<class T>
        struct having_t : having_string {
            using type = T;

            type t;

            having_t(type t_) : t(std::move(t_)) {}
        };

        template<class T>
        struct is_having : std::false_type {};

        template<class T>
        struct is_having<having_t<T>> : std::true_type {};

        struct cast_string {
            operator std::string() const {
                return "CAST";
            }
        };

        /**
         *  CAST holder.
         *  T is a type to cast to
         *  E is an expression type
         *  Example: cast<std::string>(&User::id)
         */
        template<class T, class E>
        struct cast_t : cast_string {
            using to_type = T;
            using expression_type = E;

            expression_type expression;

            cast_t(expression_type expression_) : expression(std::move(expression_)) {}
        };

    }

    /**
     *  Cute operators for columns
     */
    template<class T, class R>
    conditions::lesser_than_t<T, R> operator<(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::lesser_than_t<L, T> operator<(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    conditions::lesser_or_equal_t<T, R> operator<=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::lesser_or_equal_t<L, T> operator<=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    conditions::greater_than_t<T, R> operator>(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::greater_than_t<L, T> operator>(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    conditions::greater_or_equal_t<T, R> operator>=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::greater_or_equal_t<L, T> operator>=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    conditions::is_equal_t<T, R> operator==(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::is_equal_t<L, T> operator==(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    conditions::is_not_equal_t<T, R> operator!=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    conditions::is_not_equal_t<L, T> operator!=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class T, class R>
    internal::conc_t<T, R> operator||(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::conc_t<L, T> operator||(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::conc_t<L, R> operator||(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class T, class R>
    internal::add_t<T, R> operator+(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::add_t<L, T> operator+(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::add_t<L, R> operator+(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class T, class R>
    internal::sub_t<T, R> operator-(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::sub_t<L, T> operator-(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::sub_t<L, R> operator-(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class T, class R>
    internal::mul_t<T, R> operator*(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::mul_t<L, T> operator*(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::mul_t<L, R> operator*(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class T, class R>
    internal::div_t<T, R> operator/(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::div_t<L, T> operator/(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::div_t<L, R> operator/(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class T, class R>
    internal::mod_t<T, R> operator%(internal::expression_t<T> expr, R r) {
        return {std::move(expr.t), std::move(r)};
    }

    template<class L, class T>
    internal::mod_t<L, T> operator%(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.t)};
    }

    template<class L, class R>
    internal::mod_t<L, R> operator%(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.t), std::move(r.t)};
    }

    template<class F, class O>
    conditions::using_t<F, O> using_(F O::*p) {
        return {std::move(p)};
    }

    template<class T>
    conditions::on_t<T> on(T t) {
        return {std::move(t)};
    }

    template<class T>
    conditions::cross_join_t<T> cross_join() {
        return {};
    }

    template<class T>
    conditions::natural_join_t<T> natural_join() {
        return {};
    }

    template<class T, class O>
    conditions::left_join_t<T, O> left_join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    conditions::join_t<T, O> join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    conditions::left_outer_join_t<T, O> left_outer_join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    conditions::inner_join_t<T, O> inner_join(O o) {
        return {std::move(o)};
    }

    template<class T>
    conditions::offset_t<T> offset(T off) {
        return {std::move(off)};
    }

    template<class T>
    conditions::limit_t<T, false, false, void> limit(T lim) {
        return {std::move(lim)};
    }

    template<class T, class O>
    typename std::enable_if<!conditions::is_offset<T>::value, conditions::limit_t<T, true, true, O>>::type
    limit(O off, T lim) {
        return {std::move(lim), {std::move(off)}};
    }

    template<class T, class O>
    conditions::limit_t<T, true, false, O> limit(T lim, conditions::offset_t<O> offt) {
        return {std::move(lim), {std::move(offt.off)}};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<std::is_base_of<conditions::condition_t, L>::value ||
                                                std::is_base_of<conditions::condition_t, R>::value>::type>
    conditions::and_condition_t<L, R> operator&&(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<std::is_base_of<conditions::condition_t, L>::value ||
                                                std::is_base_of<conditions::condition_t, R>::value>::type>
    conditions::or_condition_t<L, R> operator||(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class T>
    conditions::is_not_null_t<T> is_not_null(T t) {
        return {std::move(t)};
    }

    template<class T>
    conditions::is_null_t<T> is_null(T t) {
        return {std::move(t)};
    }

    template<class L, class E>
    conditions::in_t<L, std::vector<E>> in(L l, std::vector<E> values) {
        return {std::move(l), std::move(values), false};
    }

    template<class L, class E>
    conditions::in_t<L, std::vector<E>> in(L l, std::initializer_list<E> values) {
        return {std::move(l), std::move(values), false};
    }

    template<class L, class A>
    conditions::in_t<L, A> in(L l, A arg) {
        return {std::move(l), std::move(arg), false};
    }

    template<class L, class E>
    conditions::in_t<L, std::vector<E>> not_in(L l, std::vector<E> values) {
        return {std::move(l), std::move(values), true};
    }

    template<class L, class E>
    conditions::in_t<L, std::vector<E>> not_in(L l, std::initializer_list<E> values) {
        return {std::move(l), std::move(values), true};
    }

    template<class L, class A>
    conditions::in_t<L, A> not_in(L l, A arg) {
        return {std::move(l), std::move(arg), true};
    }

    template<class L, class R>
    conditions::is_equal_t<L, R> is_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::is_equal_t<L, R> eq(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::is_not_equal_t<L, R> is_not_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::is_not_equal_t<L, R> ne(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::greater_than_t<L, R> greater_than(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::greater_than_t<L, R> gt(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::greater_or_equal_t<L, R> greater_or_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::greater_or_equal_t<L, R> ge(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::lesser_than_t<L, R> lesser_than(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::lesser_than_t<L, R> lt(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::lesser_or_equal_t<L, R> lesser_or_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    conditions::lesser_or_equal_t<L, R> le(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class C>
    conditions::where_t<C> where(C c) {
        return {std::move(c)};
    }

    /**
     * ORDER BY column
     * Example: storage.select(&User::name, order_by(&User::id))
     */
    template<class O>
    conditions::order_by_t<O> order_by(O o) {
        return {std::move(o)};
    }

    /**
     * ORDER BY column1, column2
     * Example: storage.get_all<Singer>(multi_order_by(order_by(&Singer::name).asc(), order_by(&Singer::gender).desc())
     */
    template<class... Args>
    conditions::multi_order_by_t<Args...> multi_order_by(Args &&... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     * ORDER BY column1, column2
     * Difference from `multi_order_by` is that `dynamic_order_by` can be changed at runtime using `push_back` member
     * function Example:
     *  auto orderBy = dynamic_order_by(storage);
     *  if(someCondition) {
     *      orderBy.push_back(&User::id);
     *  } else {
     *      orderBy.push_back(&User::name);
     *      orderBy.push_back(&User::birthDate);
     *  }
     */
    template<class S>
    conditions::dynamic_order_by_t<S> dynamic_order_by(const S &storage) {
        return {storage};
    }

    /**
     *  GROUP BY column.
     *  Example: storage.get_all<Employee>(group_by(&Employee::name))
     */
    template<class... Args>
    conditions::group_by_t<Args...> group_by(Args &&... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  X BETWEEN Y AND Z
     *  Example: storage.select(between(&User::id, 10, 20))
     */
    template<class A, class T>
    conditions::between_t<A, T> between(A expr, T b1, T b2) {
        return {std::move(expr), std::move(b1), std::move(b2)};
    }

    /**
     *  X LIKE Y
     *  Example: storage.select(like(&User::name, "T%"))
     */
    template<class A, class T>
    conditions::like_t<A, T, void> like(A a, T t) {
        return {std::move(a), std::move(t), {}};
    }

    /**
     *  X GLOB Y
     *  Example: storage.select(glob(&User::name, "*S"))
     */
    template<class A, class T>
    conditions::glob_t<A, T> glob(A a, T t) {
        return {std::move(a), std::move(t)};
    }

    /**
     *  X LIKE Y ESCAPE Z
     *  Example: storage.select(like(&User::name, "T%", "%"))
     */
    template<class A, class T, class E>
    conditions::like_t<A, T, E> like(A a, T t, E e) {
        return {std::move(a), std::move(t), {std::move(e)}};
    }

    /**
     *  EXISTS(condition).
     *  Example: storage.select(columns(&Agent::code, &Agent::name, &Agent::workingArea, &Agent::comission),
         where(exists(select(asterisk<Customer>(),
         where(is_equal(&Customer::grade, 3) and
         is_equal(&Agent::code, &Customer::agentCode))))),
         order_by(&Agent::comission));
     */
    template<class T>
    conditions::exists_t<T> exists(T t) {
        return {std::move(t)};
    }

    /**
     *  HAVING(expression).
     *  Example: storage.get_all<Employee>(group_by(&Employee::name), having(greater_than(count(&Employee::name), 2)));
     */
    template<class T>
    conditions::having_t<T> having(T t) {
        return {std::move(t)};
    }

    /**
     *  CAST(X AS type).
     *  Example: cast<std::string>(&User::id)
     */
    template<class T, class E>
    conditions::cast_t<T, E> cast(E e) {
        return {std::move(e)};
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_base_of, std::is_member_pointer
#include <sstream>  //  std::stringstream
#include <string>  //  std::string

namespace sqlite_orm {

    /**
     *  This is base class for every class which is used as a custom table alias.
     *  For more information please look through self_join.cpp example
     */
    struct alias_tag {};

    namespace internal {

        /**
         *  This is a common built-in class used for custom single character table aliases.
         *  Also you can use language aliases `alias_a`, `alias_b` etc. instead
         */
        template<class T, char A>
        struct table_alias : alias_tag {
            using type = T;

            static char get() {
                return A;
            }
        };

        /**
         *  Column expression with table alias attached like 'C.ID'. This is not a column alias
         */
        template<class T, class C>
        struct alias_column_t {
            using alias_type = T;
            using column_type = C;

            column_type column;

            alias_column_t(){};

            alias_column_t(column_type column_) : column(column_) {}
        };

        template<class T, class SFINAE = void>
        struct alias_extractor;

        template<class T>
        struct alias_extractor<T, typename std::enable_if<std::is_base_of<alias_tag, T>::value>::type> {
            static std::string get() {
                std::stringstream ss;
                ss << T::get();
                return ss.str();
            }
        };

        template<class T>
        struct alias_extractor<T, typename std::enable_if<!std::is_base_of<alias_tag, T>::value>::type> {
            static std::string get() {
                return {};
            }
        };

        template<class T, class E>
        struct as_t {
            using alias_type = T;
            using expression_type = E;

            expression_type expression;
        };

        template<class T>
        struct alias_holder {
            using type = T;
        };
    }

    /**
     *  @return column with table alias attached. Place it instead of a column statement in case you need to specify a
     *  column with table alias prefix like 'a.column'. For more information please look through self_join.cpp example
     */
    template<class T, class C>
    internal::alias_column_t<T, C> alias_column(C c) {
        static_assert(std::is_member_pointer<C>::value,
                      "alias_column argument must be a member pointer mapped to a storage");
        return {c};
    }

    template<class T, class E>
    internal::as_t<T, E> as(E expression) {
        return {std::move(expression)};
    }

    template<class T>
    internal::alias_holder<T> get() {
        return {};
    }

    template<class T>
    using alias_a = internal::table_alias<T, 'a'>;
    template<class T>
    using alias_b = internal::table_alias<T, 'b'>;
    template<class T>
    using alias_c = internal::table_alias<T, 'c'>;
    template<class T>
    using alias_d = internal::table_alias<T, 'd'>;
    template<class T>
    using alias_e = internal::table_alias<T, 'e'>;
    template<class T>
    using alias_f = internal::table_alias<T, 'f'>;
    template<class T>
    using alias_g = internal::table_alias<T, 'g'>;
    template<class T>
    using alias_h = internal::table_alias<T, 'h'>;
    template<class T>
    using alias_i = internal::table_alias<T, 'i'>;
    template<class T>
    using alias_j = internal::table_alias<T, 'j'>;
    template<class T>
    using alias_k = internal::table_alias<T, 'k'>;
    template<class T>
    using alias_l = internal::table_alias<T, 'l'>;
    template<class T>
    using alias_m = internal::table_alias<T, 'm'>;
    template<class T>
    using alias_n = internal::table_alias<T, 'n'>;
    template<class T>
    using alias_o = internal::table_alias<T, 'o'>;
    template<class T>
    using alias_p = internal::table_alias<T, 'p'>;
    template<class T>
    using alias_q = internal::table_alias<T, 'q'>;
    template<class T>
    using alias_r = internal::table_alias<T, 'r'>;
    template<class T>
    using alias_s = internal::table_alias<T, 's'>;
    template<class T>
    using alias_t = internal::table_alias<T, 't'>;
    template<class T>
    using alias_u = internal::table_alias<T, 'u'>;
    template<class T>
    using alias_v = internal::table_alias<T, 'v'>;
    template<class T>
    using alias_w = internal::table_alias<T, 'w'>;
    template<class T>
    using alias_x = internal::table_alias<T, 'x'>;
    template<class T>
    using alias_y = internal::table_alias<T, 'y'>;
    template<class T>
    using alias_z = internal::table_alias<T, 'z'>;
}
#pragma once

// #include "conditions.h"

namespace sqlite_orm {

    namespace internal {

        template<class... Args>
        struct join_iterator {

            template<class L>
            void operator()(const L &) {
                //..
            }
        };

        template<>
        struct join_iterator<> {

            template<class L>
            void operator()(const L &) {
                //..
            }
        };

        template<class H, class... Tail>
        struct join_iterator<H, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;

            template<class L>
            void operator()(const L &l) {
                this->super::operator()(l);
            }
        };

        template<class T, class... Tail>
        struct join_iterator<conditions::cross_join_t<T>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::cross_join_t<T>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class... Tail>
        struct join_iterator<conditions::natural_join_t<T>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::natural_join_t<T>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<conditions::left_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::left_join_t<T, O>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<conditions::join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::join_t<T, O>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<conditions::left_outer_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::left_outer_join_t<T, O>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<conditions::inner_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = conditions::inner_join_t<T, O>;

            template<class L>
            void operator()(const L &l) {
                l(*this);
                this->super::operator()(l);
            }
        };
    }
}
#pragma once

#include <string>  //  std::string
#include <tuple>  //  std::make_tuple, std::tuple_size
#include <type_traits>  //  std::forward, std::is_base_of, std::enable_if
#include <memory>  //  std::unique_ptr
#include <vector>  //  std::vector

// #include "conditions.h"

// #include "operators.h"

// #include "is_base_of_template.h"

#include <type_traits>  //  std::true_type, std::false_type, std::declval

namespace sqlite_orm {

    namespace internal {

        /*
         * This is because of bug in MSVC, for more information, please visit
         * https://stackoverflow.com/questions/34672441/stdis-base-of-for-template-classes/34672753#34672753
         */
#if defined(_MSC_VER)
        template<template<typename...> class Base, typename Derived>
        struct is_base_of_template_impl {
            template<typename... Ts>
            static constexpr std::true_type test(const Base<Ts...> *);

            static constexpr std::false_type test(...);

            using type = decltype(test(std::declval<Derived *>()));
        };

        template<typename Derived, template<typename...> class Base>
        using is_base_of_template = typename is_base_of_template_impl<Base, Derived>::type;

#else
        template<template<typename...> class C, typename... Ts>
        std::true_type is_base_of_template_impl(const C<Ts...> *);

        template<template<typename...> class C>
        std::false_type is_base_of_template_impl(...);

        template<typename T, template<typename...> class C>
        using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T *>()));
#endif
    }
}

namespace sqlite_orm {

    namespace core_functions {

        /**
         *  Base class for operator overloading
         *  R - return type
         *  S - class with operator std::string
         *  Args - function arguments types
         */
        template<class R, class S, class... Args>
        struct core_function_t : S, internal::arithmetic_t {
            using return_type = R;
            using string_type = S;
            using args_type = std::tuple<Args...>;

            static constexpr const size_t args_size = std::tuple_size<args_type>::value;

            args_type args;

            core_function_t(args_type &&args_) : args(std::move(args_)) {}
        };

        struct length_string {
            operator std::string() const {
                return "LENGTH";
            }
        };

        struct abs_string {
            operator std::string() const {
                return "ABS";
            }
        };

        struct lower_string {
            operator std::string() const {
                return "LOWER";
            }
        };

        struct upper_string {
            operator std::string() const {
                return "UPPER";
            }
        };

        struct changes_string {
            operator std::string() const {
                return "CHANGES";
            }
        };

        struct trim_string {
            operator std::string() const {
                return "TRIM";
            }
        };

        struct ltrim_string {
            operator std::string() const {
                return "LTRIM";
            }
        };

        struct rtrim_string {
            operator std::string() const {
                return "RTRIM";
            }
        };

#if SQLITE_VERSION_NUMBER >= 3007016

        struct char_string {
            operator std::string() const {
                return "CHAR";
            }
        };

        struct random_string {
            operator std::string() const {
                return "RANDOM";
            }
        };

#endif

        struct coalesce_string {
            operator std::string() const {
                return "COALESCE";
            }
        };

        struct date_string {
            operator std::string() const {
                return "DATE";
            }
        };

        struct datetime_string {
            operator std::string() const {
                return "DATETIME";
            }
        };

        struct julianday_string {
            operator std::string() const {
                return "JULIANDAY";
            }
        };

        struct zeroblob_string {
            operator std::string() const {
                return "ZEROBLOB";
            }
        };

        struct substr_string {
            operator std::string() const {
                return "SUBSTR";
            }
        };
    }

    /**
     *  Cute operators for core functions
     */

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::lesser_than_t<F, R> operator<(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::lesser_or_equal_t<F, R> operator<=(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::greater_than_t<F, R> operator>(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::greater_or_equal_t<F, R> operator>=(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::is_equal_t<F, R> operator==(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             typename = typename std::enable_if<
                 internal::is_base_of_template<F, core_functions::core_function_t>::value>::type>
    conditions::is_not_equal_t<F, R> operator!=(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    /**
     *  LENGTH(x) function https://sqlite.org/lang_corefunc.html#length
     */
    template<class T>
    core_functions::core_function_t<int, core_functions::length_string, T> length(T t) {
        std::tuple<T> args{std::forward<T>(t)};
        return {std::move(args)};
    }

    /**
     *  ABS(x) function https://sqlite.org/lang_corefunc.html#abs
     */
    template<class T>
    core_functions::core_function_t<std::unique_ptr<double>, core_functions::abs_string, T> abs(T t) {
        std::tuple<T> args{std::forward<T>(t)};
        return {std::move(args)};
    }

    /**
     *  LOWER(x) function https://sqlite.org/lang_corefunc.html#lower
     */
    template<class T>
    core_functions::core_function_t<std::string, core_functions::lower_string, T> lower(T t) {
        std::tuple<T> args{std::forward<T>(t)};
        return {std::move(args)};
    }

    /**
     *  UPPER(x) function https://sqlite.org/lang_corefunc.html#upper
     */
    template<class T>
    core_functions::core_function_t<std::string, core_functions::upper_string, T> upper(T t) {
        std::tuple<T> args{std::forward<T>(t)};
        return {std::move(args)};
    }

    /**
     *  CHANGES() function https://sqlite.org/lang_corefunc.html#changes
     */
    inline core_functions::core_function_t<int, core_functions::changes_string> changes() {
        return {{}};
    }

    /**
     *  TRIM(X) function https://sqlite.org/lang_corefunc.html#trim
     */
    template<class T>
    core_functions::core_function_t<std::string, core_functions::trim_string, T> trim(T t) {
        std::tuple<T> args{std::forward<T>(t)};
        return {std::move(args)};
    }

    /**
     *  TRIM(X,Y) function https://sqlite.org/lang_corefunc.html#trim
     */
    template<class X, class Y>
    core_functions::core_function_t<std::string, core_functions::trim_string, X, Y> trim(X x, Y y) {
        std::tuple<X, Y> args{std::forward<X>(x), std::forward<Y>(y)};
        return {std::move(args)};
    }

    /**
     *  LTRIM(X) function https://sqlite.org/lang_corefunc.html#ltrim
     */
    template<class X>
    core_functions::core_function_t<std::string, core_functions::ltrim_string, X> ltrim(X x) {
        std::tuple<X> args{std::forward<X>(x)};
        return {std::move(args)};
    }

    /**
     *  LTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#ltrim
     */
    template<class X, class Y>
    core_functions::core_function_t<std::string, core_functions::ltrim_string, X, Y> ltrim(X x, Y y) {
        std::tuple<X, Y> args{std::forward<X>(x), std::forward<Y>(y)};
        return {std::move(args)};
    }

    /**
     *  RTRIM(X) function https://sqlite.org/lang_corefunc.html#rtrim
     */
    template<class X>
    core_functions::core_function_t<std::string, core_functions::rtrim_string, X> rtrim(X x) {
        std::tuple<X> args{std::forward<X>(x)};
        return {std::move(args)};
    }

    /**
     *  RTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#rtrim
     */
    template<class X, class Y>
    core_functions::core_function_t<std::string, core_functions::rtrim_string, X, Y> rtrim(X x, Y y) {
        std::tuple<X, Y> args{std::forward<X>(x), std::forward<Y>(y)};
        return {std::move(args)};
    }

#if SQLITE_VERSION_NUMBER >= 3007016

    /**
     *  CHAR(X1,X2,...,XN) function https://sqlite.org/lang_corefunc.html#char
     */
    template<class... Args>
    core_functions::core_function_t<std::string, core_functions::char_string, Args...> char_(Args... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  RANDOM() function https://www.sqlite.org/lang_corefunc.html#random
     */
    inline core_functions::core_function_t<int, core_functions::random_string> random() {
        return {{}};
    }

#endif

    /**
     *  COALESCE(X,Y,...) function https://www.sqlite.org/lang_corefunc.html#coalesce
     */
    template<class R, class... Args>
    core_functions::core_function_t<R, core_functions::coalesce_string, Args...> coalesce(Args... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  DATE(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    core_functions::core_function_t<std::string, core_functions::date_string, Args...> date(Args... args) {
        std::tuple<Args...> t{std::forward<Args>(args)...};
        return {std::move(t)};
    }

    /**
     *  DATETIME(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    core_functions::core_function_t<std::string, core_functions::datetime_string, Args...> datetime(Args... args) {
        std::tuple<Args...> t{std::forward<Args>(args)...};
        return {std::move(t)};
    }

    /**
     *  JULIANDAY(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    core_functions::core_function_t<double, core_functions::julianday_string, Args...> julianday(Args... args) {
        std::tuple<Args...> t{std::forward<Args>(args)...};
        return {std::move(t)};
    }

    /**
     *  ZEROBLOB(N) function https://www.sqlite.org/lang_corefunc.html#zeroblob
     */
    template<class N>
    core_functions::core_function_t<std::vector<char>, core_functions::zeroblob_string, N> zeroblob(N n) {
        std::tuple<N> args{std::forward<N>(n)};
        return {std::move(args)};
    }

    /**
     *  SUBSTR(X,Y) function https://www.sqlite.org/lang_corefunc.html#substr
     */
    template<class X, class Y>
    core_functions::core_function_t<std::string, core_functions::substr_string, X, Y> substr(X x, Y y) {
        std::tuple<X, Y> args{std::forward<X>(x), std::forward<Y>(y)};
        return {std::move(args)};
    }

    /**
     *  SUBSTR(X,Y,Z) function https://www.sqlite.org/lang_corefunc.html#substr
     */
    template<class X, class Y, class Z>
    core_functions::core_function_t<std::string, core_functions::substr_string, X, Y, Z> substr(X x, Y y, Z z) {
        std::tuple<X, Y, Z> args{std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z)};
        return {std::move(args)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value +
                                                     std::is_base_of<internal::arithmetic_t, R>::value >
                                                 0)>::type>
    internal::add_t<L, R> operator+(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value +
                                                     std::is_base_of<internal::arithmetic_t, R>::value >
                                                 0)>::type>
    internal::sub_t<L, R> operator-(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value +
                                                     std::is_base_of<internal::arithmetic_t, R>::value >
                                                 0)>::type>
    internal::mul_t<L, R> operator*(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value +
                                                     std::is_base_of<internal::arithmetic_t, R>::value >
                                                 0)>::type>
    internal::div_t<L, R> operator/(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             typename = typename std::enable_if<(std::is_base_of<internal::arithmetic_t, L>::value +
                                                     std::is_base_of<internal::arithmetic_t, R>::value >
                                                 0)>::type>
    internal::mod_t<L, R> operator%(L l, R r) {
        return {std::move(l), std::move(r)};
    }
}
#pragma once

namespace sqlite_orm {

    namespace aggregate_functions {

        struct avg_string {
            operator std::string() const {
                return "AVG";
            }
        };

        /**
         *  Result of avg(...) call.
         *  T is an argument type
         */
        template<class T>
        struct avg_t : avg_string {
            using arg_type = T;

            arg_type arg;

            avg_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct count_string {
            operator std::string() const {
                return "COUNT";
            }
        };

        /**
         *  Result of count(...) call.
         *  T is an argument type
         */
        template<class T>
        struct count_t : count_string {
            using arg_type = T;

            arg_type arg;

            count_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        /**
         *  T is use to specify type explicitly for queries like
         *  SELECT COUNT(*) FROM table_name;
         *  T can be omitted with void.
         */
        template<class T>
        struct count_asterisk_t : count_string {
            using type = T;
        };

        /**
         *  The same thing as count<T>() but without T arg.
         *  Is used in cases like this:
         *    SELECT cust_code, cust_name, cust_city, grade
         *    FROM customer
         *    WHERE grade=2 AND EXISTS
         *        (SELECT COUNT(*)
         *        FROM customer
         *        WHERE grade=2
         *        GROUP BY grade
         *        HAVING COUNT(*)>2);
         *  `c++`
         *  auto rows =
         *      storage.select(columns(&Customer::code, &Customer::name, &Customer::city, &Customer::grade),
         *          where(is_equal(&Customer::grade, 2)
         *              and exists(select(count<Customer>(),
         *                  where(is_equal(&Customer::grade, 2)),
         *          group_by(&Customer::grade),
         *          having(greater_than(count(), 2))))));
         */
        struct count_asterisk_without_type : count_string {};

        struct sum_string {
            operator std::string() const {
                return "SUM";
            }
        };

        /**
         *  Result of sum(...) call.
         *  T is an argument type
         */
        template<class T>
        struct sum_t : sum_string {
            using arg_type = T;

            arg_type arg;

            sum_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct total_string {
            operator std::string() const {
                return "TOTAL";
            }
        };

        /**
         *  Result of total(...) call.
         *  T is an argument type
         */
        template<class T>
        struct total_t : total_string {
            using arg_type = T;

            arg_type arg;

            total_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct max_string {
            operator std::string() const {
                return "MAX";
            }
        };

        /**
         *  Result of max(...) call.
         *  T is an argument type
         */
        template<class T>
        struct max_t : max_string {
            using arg_type = T;

            arg_type arg;

            max_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct min_string {
            operator std::string() const {
                return "MIN";
            }
        };

        /**
         *  Result of min(...) call.
         *  T is an argument type
         */
        template<class T>
        struct min_t : min_string {
            using arg_type = T;

            arg_type arg;

            min_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct group_concat_string {
            operator std::string() const {
                return "GROUP_CONCAT";
            }
        };

        /**
         *  Result of group_concat(X) call.
         *  T is an argument type
         */
        template<class T>
        struct group_concat_single_t : group_concat_string {
            using arg_type = T;

            arg_type arg;

            group_concat_single_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        struct group_concat_double_base : group_concat_string {
            std::string y;

            group_concat_double_base(std::string y_) : y(move(y_)) {}
        };

        /**
         *  Result of group_concat(X, Y) call.
         *  T is an argument type
         */
        template<class T>
        struct group_concat_double_t : group_concat_double_base {
            using arg_type = T;

            arg_type arg;

            group_concat_double_t(arg_type arg_, std::string y) :
                group_concat_double_base(move(y)), arg(std::move(arg_)) {}
        };
    }

    /**
     *  AVG(X) aggregate function.
     */
    template<class T>
    aggregate_functions::avg_t<T> avg(T t) {
        return {std::move(t)};
    }

    /**
     *  COUNT(X) aggregate function.
     */
    template<class T>
    aggregate_functions::count_t<T> count(T t) {
        return {std::move(t)};
    }

    /**
     *  COUNT(*) without FROM function.
     */
    inline aggregate_functions::count_asterisk_without_type count() {
        return {};
    }

    /**
     *  COUNT(*) with FROM function. Specified type T will be serializeed as
     *  a from argument.
     */
    template<class T>
    aggregate_functions::count_asterisk_t<T> count() {
        return {};
    }

    /**
     *  SUM(X) aggregate function.
     */
    template<class T>
    aggregate_functions::sum_t<T> sum(T t) {
        return {std::move(t)};
    }

    /**
     *  MAX(X) aggregate function.
     */
    template<class T>
    aggregate_functions::max_t<T> max(T t) {
        return {std::move(t)};
    }

    /**
     *  MIN(X) aggregate function.
     */
    template<class T>
    aggregate_functions::min_t<T> min(T t) {
        return {std::move(t)};
    }

    /**
     *  TOTAL(X) aggregate function.
     */
    template<class T>
    aggregate_functions::total_t<T> total(T t) {
        return {std::move(t)};
    }

    /**
     *  GROUP_CONCAT(X) aggregate function.
     */
    template<class T>
    aggregate_functions::group_concat_single_t<T> group_concat(T t) {
        return {std::move(t)};
    }

    /**
     *  GROUP_CONCAT(X, Y) aggregate function.
     */
    template<class T, class Y>
    aggregate_functions::group_concat_double_t<T> group_concat(T t, Y y) {
        return {std::move(t), std::move(y)};
    }
}
#pragma once

namespace sqlite_orm {

    namespace internal {

        /**
         *  Cute class used to compare setters/getters and member pointers with each other.
         */
        template<class L, class R>
        struct typed_comparator {
            bool operator()(const L &, const R &) const {
                return false;
            }
        };

        template<class O>
        struct typed_comparator<O, O> {
            bool operator()(const O &lhs, const O &rhs) const {
                return lhs == rhs;
            }
        };

        template<class L, class R>
        bool compare_any(const L &lhs, const R &rhs) {
            return typed_comparator<L, R>()(lhs, rhs);
        }
    }
}
#pragma once

#include <string>  //  std::string
#include <utility>  //  std::declval
#include <tuple>  //  std::tuple, std::get, std::tuple_size

// #include "is_base_of_template.h"

// #include "tuple_helper.h"

// #include "optional_container.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  DISCTINCT generic container.
         */
        template<class T>
        struct distinct_t {
            T t;

            operator std::string() const {
                return "DISTINCT";
            }
        };

        /**
         *  ALL generic container.
         */
        template<class T>
        struct all_t {
            T t;

            operator std::string() const {
                return "ALL";
            }
        };

        template<class... Args>
        struct columns_t {
            using columns_type = std::tuple<Args...>;

            columns_type columns;
            bool distinct = false;

            static constexpr const int count = std::tuple_size<columns_type>::value;
        };

        struct set_string {
            operator std::string() const {
                return "SET";
            }
        };

        template<class... Args>
        struct set_t : set_string {
            using assigns_type = std::tuple<Args...>;

            assigns_type assigns;

            set_t(assigns_type assigns_) : assigns(move(assigns_)) {}
        };

        /**
         *  This class is used to store explicit mapped type T and its column descriptor (member pointer/getter/setter).
         *  Is useful when mapped type is derived from other type and base class has members mapped to a storage.
         */
        template<class T, class F>
        struct column_pointer {
            using type = T;
            using field_type = F;

            field_type field;
        };

        /**
         *  Subselect object type.
         */
        template<class T, class... Args>
        struct select_t {
            using return_type = T;
            using conditions_type = std::tuple<Args...>;

            return_type col;
            conditions_type conditions;
            bool highest_level = false;
        };

        /**
         *  Base for UNION, UNION ALL, EXCEPT and INTERSECT
         */
        template<class L, class R>
        struct compound_operator {
            using left_type = L;
            using right_type = R;

            left_type left;
            right_type right;

            compound_operator(left_type l, right_type r) : left(std::move(l)), right(std::move(r)) {
                this->left.highest_level = true;
                this->right.highest_level = true;
            }
        };

        struct union_base {
            bool all = false;

            operator std::string() const {
                if(!this->all) {
                    return "UNION";
                } else {
                    return "UNION ALL";
                }
            }
        };

        /**
         *  UNION object type.
         */
        template<class L, class R>
        struct union_t : public compound_operator<L, R>, union_base {
            using left_type = typename compound_operator<L, R>::left_type;
            using right_type = typename compound_operator<L, R>::right_type;

            union_t(left_type l, right_type r, decltype(all) all) :
                compound_operator<L, R>(std::move(l), std::move(r)), union_base{all} {}

            union_t(left_type l, right_type r) : union_t(std::move(l), std::move(r), false) {}
        };

        /**
         *  EXCEPT object type.
         */
        template<class L, class R>
        struct except_t : public compound_operator<L, R> {
            using super = compound_operator<L, R>;
            using left_type = typename super::left_type;
            using right_type = typename super::right_type;

            using super::super;

            operator std::string() const {
                return "EXCEPT";
            }
        };

        /**
         *  INTERSECT object type.
         */
        template<class L, class R>
        struct intersect_t : public compound_operator<L, R> {
            using super = compound_operator<L, R>;
            using left_type = typename super::left_type;
            using right_type = typename super::right_type;

            using super::super;

            operator std::string() const {
                return "INTERSECT";
            }
        };

        /**
         *  Generic way to get DISTINCT value from any type.
         */
        template<class T>
        bool get_distinct(const T &) {
            return false;
        }

        template<class... Args>
        bool get_distinct(const columns_t<Args...> &cols) {
            return cols.distinct;
        }

        template<class T>
        struct asterisk_t {
            using type = T;
        };

        template<class T>
        struct then_t {
            using expression_type = T;

            expression_type expression;
        };

        template<class R, class T, class E, class... Args>
        struct simple_case_t {
            using return_type = R;
            using case_expression_type = T;
            using args_type = std::tuple<Args...>;
            using else_expression_type = E;

            optional_container<case_expression_type> case_expression;
            args_type args;
            optional_container<else_expression_type> else_expression;
        };

        /**
         *  T is a case expression type
         *  E is else type (void is ELSE is omitted)
         *  Args... is a pack of WHEN expressions
         */
        template<class R, class T, class E, class... Args>
        struct simple_case_builder {
            using return_type = R;
            using case_expression_type = T;
            using args_type = std::tuple<Args...>;
            using else_expression_type = E;

            optional_container<case_expression_type> case_expression;
            args_type args;
            optional_container<else_expression_type> else_expression;

            template<class W, class Th>
            simple_case_builder<R, T, E, Args..., std::pair<W, Th>> when(W w, then_t<Th> t) {
                using result_args_type = std::tuple<Args..., std::pair<W, Th>>;
                result_args_type result_args;
                move_tuple<std::tuple_size<args_type>::value>(result_args, this->args);
                std::pair<W, Th> newPair{std::move(w), std::move(t.expression)};
                std::get<std::tuple_size<result_args_type>::value - 1>(result_args) = std::move(newPair);
                return {std::move(this->case_expression), std::move(result_args), std::move(this->else_expression)};
            }

            simple_case_t<R, T, E, Args...> end() {
                return {std::move(this->case_expression), std::move(args), std::move(this->else_expression)};
            }

            template<class El>
            simple_case_builder<R, T, El, Args...> else_(El el) {
                return {{std::move(this->case_expression)}, std::move(args), {std::move(el)}};
            }
        };

        template<class T>
        void validate_conditions() {
            static_assert(count_tuple<T, conditions::is_where>::value <= 1,
                          "a single query cannot contain > 1 WHERE blocks");
            static_assert(count_tuple<T, conditions::is_group_by>::value <= 1,
                          "a single query cannot contain > 1 GROUP BY blocks");
            static_assert(count_tuple<T, conditions::is_order_by>::value <= 1,
                          "a single query cannot contain > 1 ORDER BY blocks");
            static_assert(count_tuple<T, conditions::is_limit>::value <= 1,
                          "a single query cannot contain > 1 LIMIT blocks");
        }
    }

    template<class T>
    internal::then_t<T> then(T t) {
        return {std::move(t)};
    }

    template<class R, class T>
    internal::simple_case_builder<R, T, void> case_(T t) {
        return {{std::move(t)}};
    }

    template<class R>
    internal::simple_case_builder<R, void, void> case_() {
        return {};
    }

    template<class T>
    internal::distinct_t<T> distinct(T t) {
        return {std::move(t)};
    }

    template<class T>
    internal::all_t<T> all(T t) {
        return {std::move(t)};
    }

    template<class... Args>
    internal::columns_t<Args...> distinct(internal::columns_t<Args...> cols) {
        cols.distinct = true;
        return cols;
    }

    /**
     *  SET keyword used in UPDATE ... SET queries.
     *  Args must have `assign_t` type. E.g. set(assign(&User::id, 5)) or set(c(&User::id) = 5)
     */
    template<class... Args>
    internal::set_t<Args...> set(Args... args) {
        using arg_tuple = std::tuple<Args...>;
        static_assert(std::tuple_size<arg_tuple>::value ==
                          internal::count_tuple<arg_tuple, internal::is_assign_t>::value,
                      "set function accepts assign operators only");
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    template<class... Args>
    internal::columns_t<Args...> columns(Args... args) {
        return {std::make_tuple<Args...>(std::forward<Args>(args)...)};
    }

    /**
     *  Use it like this:
     *  struct MyType : BaseType { ... };
     *  storage.select(column<MyType>(&BaseType::id));
     */
    template<class T, class F>
    internal::column_pointer<T, F> column(F f) {
        return {std::move(f)};
    }

    /**
     *  Public function for subselect query. Is useful in UNION queries.
     */
    template<class T, class... Args>
    internal::select_t<T, Args...> select(T t, Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        return {std::move(t), std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  Public function for UNION operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
    template<class L, class R>
    internal::union_t<L, R> union_(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    /**
     *  Public function for EXCEPT operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/except.cpp
     */
    template<class L, class R>
    internal::except_t<L, R> except(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    template<class L, class R>
    internal::intersect_t<L, R> intersect(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    /**
     *  Public function for UNION ALL operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
    template<class L, class R>
    internal::union_t<L, R> union_all(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs), true};
    }

    template<class T>
    internal::asterisk_t<T> asterisk() {
        return {};
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_member_pointer

// #include "select_constraints.h"

// #include "column.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  Trait class used to define table mapped type by setter/getter/member
         *  T - member pointer
         */
        template<class T, class SFINAE = void>
        struct table_type;

        template<class O, class F>
        struct table_type<F O::*,
                          typename std::enable_if<std::is_member_pointer<F O::*>::value &&
                                                  !std::is_member_function_pointer<F O::*>::value>::type> {
            using type = O;
        };

        template<class T>
        struct table_type<T, typename std::enable_if<is_getter<T>::value>::type> {
            using type = typename getter_traits<T>::object_type;
        };

        template<class T>
        struct table_type<T, typename std::enable_if<is_setter<T>::value>::type> {
            using type = typename setter_traits<T>::object_type;
        };

        template<class T, class F>
        struct table_type<column_pointer<T, F>, void> {
            using type = T;
        };
    }
}
#pragma once

#include <string>  //  std::string

namespace sqlite_orm {

    struct table_info {
        int cid;
        std::string name;
        std::string type;
        bool notnull;
        std::string dflt_value;
        int pk;
    };

}
#pragma once

#include <sqlite3.h>

namespace sqlite_orm {

    /**
     *  Guard class which finalizes `sqlite3_stmt` in dtor
     */
    struct statement_finalizer {
        sqlite3_stmt *stmt = nullptr;

        statement_finalizer(decltype(stmt) stmt_) : stmt(stmt_) {}

        inline ~statement_finalizer() {
            sqlite3_finalize(this->stmt);
        }
    };
}
#pragma once

namespace sqlite_orm {

    /**
     *  Helper classes used by statement_binder and row_extractor.
     */
    struct int_or_smaller_tag {};
    struct bigint_tag {};
    struct real_tag {};

    template<class V>
    struct arithmetic_tag {
        using type = std::conditional_t<std::is_integral<V>::value,
                                        // Integer class
                                        std::conditional_t<sizeof(V) <= sizeof(int), int_or_smaller_tag, bigint_tag>,
                                        // Floating-point class
                                        real_tag>;
    };

    template<class V>
    using arithmetic_tag_t = typename arithmetic_tag<V>::type;
}
#pragma once

#include <sqlite3.h>
#include <type_traits>  //  std::enable_if_t, std::is_arithmetic, std::is_same, std::true_type, std::false_type
#include <string>  //  std::string, std::wstring
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt>  //  std::wstring_convert, std::codecvt_utf8_utf16
#endif  //  SQLITE_ORM_OMITS_CODECVT
#include <vector>  //  std::vector
#include <cstddef>  //  std::nullptr_t
#include <utility>  //  std::declval

// #include "is_std_ptr.h"

namespace sqlite_orm {

    /**
     *  Specialization for optional type (std::shared_ptr / std::unique_ptr).
     */
    template<typename T>
    struct is_std_ptr : std::false_type {};

    template<typename T>
    struct is_std_ptr<std::shared_ptr<T>> : std::true_type {
        using element_type = T;

        static std::shared_ptr<T> make(const T &v) {
            return std::make_shared<T>(v);
        }
    };

    template<typename T>
    struct is_std_ptr<std::unique_ptr<T>> : std::true_type {
        using element_type = T;

        static std::unique_ptr<T> make(const T &v) {
            return std::make_unique<T>(v);
        }
    };
}

namespace sqlite_orm {

    /**
     *  Helper class used for binding fields to sqlite3 statements.
     */
    template<class V, typename Enable = void>
    struct statement_binder : std::false_type {};

    /**
     *  Specialization for arithmetic types.
     */
    template<class V>
    struct statement_binder<V, std::enable_if_t<std::is_arithmetic<V>::value>> {
        int bind(sqlite3_stmt *stmt, int index, const V &value) {
            return bind(stmt, index, value, tag());
        }

      private:
        using tag = arithmetic_tag_t<V>;

        int bind(sqlite3_stmt *stmt, int index, const V &value, const int_or_smaller_tag &) {
            return sqlite3_bind_int(stmt, index, static_cast<int>(value));
        }

        int bind(sqlite3_stmt *stmt, int index, const V &value, const bigint_tag &) {
            return sqlite3_bind_int64(stmt, index, static_cast<sqlite3_int64>(value));
        }

        int bind(sqlite3_stmt *stmt, int index, const V &value, const real_tag &) {
            return sqlite3_bind_double(stmt, index, static_cast<double>(value));
        }
    };

    /**
     *  Specialization for std::string and C-string.
     */
    template<class V>
    struct statement_binder<
        V,
        std::enable_if_t<std::is_same<V, std::string>::value || std::is_same<V, const char *>::value>> {
        int bind(sqlite3_stmt *stmt, int index, const V &value) {
            return sqlite3_bind_text(stmt, index, string_data(value), -1, SQLITE_TRANSIENT);
        }

      private:
        const char *string_data(const std::string &s) const {
            return s.c_str();
        }

        const char *string_data(const char *s) const {
            return s;
        }
    };

#ifndef SQLITE_ORM_OMITS_CODECVT
    /**
     *  Specialization for std::wstring and C-wstring.
     */
    template<class V>
    struct statement_binder<
        V,
        std::enable_if_t<std::is_same<V, std::wstring>::value || std::is_same<V, const wchar_t *>::value>> {
        int bind(sqlite3_stmt *stmt, int index, const V &value) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string utf8Str = converter.to_bytes(value);
            return statement_binder<decltype(utf8Str)>().bind(stmt, index, utf8Str);
        }
    };
#endif  //  SQLITE_ORM_OMITS_CODECVT

    /**
     *  Specialization for std::nullptr_t.
     */
    template<>
    struct statement_binder<std::nullptr_t, void> {
        int bind(sqlite3_stmt *stmt, int index, const std::nullptr_t &) {
            return sqlite3_bind_null(stmt, index);
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<>
    struct statement_binder<std::nullopt_t, void> {
        int bind(sqlite3_stmt *stmt, int index, const std::nullopt_t &) {
            return sqlite3_bind_null(stmt, index);
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

    template<class V>
    struct statement_binder<V, std::enable_if_t<is_std_ptr<V>::value>> {
        using value_type = typename is_std_ptr<V>::element_type;

        int bind(sqlite3_stmt *stmt, int index, const V &value) {
            if(value) {
                return statement_binder<value_type>().bind(stmt, index, *value);
            } else {
                return statement_binder<std::nullptr_t>().bind(stmt, index, nullptr);
            }
        }
    };

    /**
     *  Specialization for optional type (std::vector<char>).
     */
    template<>
    struct statement_binder<std::vector<char>, void> {
        int bind(sqlite3_stmt *stmt, int index, const std::vector<char> &value) {
            if(value.size()) {
                return sqlite3_bind_blob(stmt,
                                         index,
                                         (const void *)&value.front(),
                                         int(value.size()),
                                         SQLITE_TRANSIENT);
            } else {
                return sqlite3_bind_blob(stmt, index, "", 0, SQLITE_TRANSIENT);
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct statement_binder<std::optional<T>, void> {
        using value_type = T;

        int bind(sqlite3_stmt *stmt, int index, const std::optional<T> &value) {
            if(value) {
                return statement_binder<value_type>().bind(stmt, index, *value);
            } else {
                return statement_binder<std::nullopt_t>().bind(stmt, index, std::nullopt);
            }
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

    namespace internal {

        template<class T>
        using is_bindable = std::integral_constant<bool, !std::is_base_of<std::false_type, statement_binder<T>>::value>;

        struct conditional_binder_base {
            sqlite3_stmt *stmt = nullptr;
            int &index;

            conditional_binder_base(decltype(stmt) stmt_, decltype(index) index_) : stmt(stmt_), index(index_) {}
        };

        template<class T, class C>
        struct conditional_binder;

        template<class T>
        struct conditional_binder<T, std::true_type> : conditional_binder_base {

            using conditional_binder_base::conditional_binder_base;

            int operator()(const T &t) const {
                return statement_binder<T>().bind(this->stmt, this->index++, t);
            }
        };

        template<class T>
        struct conditional_binder<T, std::false_type> : conditional_binder_base {
            using conditional_binder_base::conditional_binder_base;

            int operator()(const T &) const {
                return SQLITE_OK;
            }
        };

        template<class T, class SFINAE = void>
        struct bindable_filter_single;

        template<class T>
        struct bindable_filter_single<T, typename std::enable_if<is_bindable<T>::value>::type> {
            using type = std::tuple<T>;
        };

        template<class T>
        struct bindable_filter_single<T, typename std::enable_if<!is_bindable<T>::value>::type> {
            using type = std::tuple<>;
        };

        template<class T>
        struct bindable_filter;

        template<class... Args>
        struct bindable_filter<std::tuple<Args...>> {
            using type = typename conc_tuple<typename bindable_filter_single<Args>::type...>::type;
        };
    }
}
#pragma once

#include <sqlite3.h>
#include <type_traits>  //  std::enable_if_t, std::is_arithmetic, std::is_same, std::enable_if
#include <cstdlib>  //  atof, atoi, atoll
#include <string>  //  std::string, std::wstring
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt>  //  std::wstring_convert, std::codecvt_utf8_utf16
#endif  //  SQLITE_ORM_OMITS_CODECVT
#include <vector>  //  std::vector
#include <cstring>  //  strlen
#include <algorithm>  //  std::copy
#include <iterator>  //  std::back_inserter
#include <tuple>  //  std::tuple, std::tuple_size, std::tuple_element

// #include "arithmetic_tag.h"

// #include "journal_mode.h"

#include <string>  //  std::string
#include <memory>  //  std::unique_ptr
#include <array>  //  std::array
#include <algorithm>  //  std::transform
#include <locale>  // std::toupper

namespace sqlite_orm {

/**
 *  Caps case cause of 1) delete keyword; 2) https://www.sqlite.org/pragma.html#pragma_journal_mode original spelling
 */
#ifdef DELETE
#undef DELETE
#endif
    enum class journal_mode : signed char {
        DELETE = 0,
        TRUNCATE = 1,
        PERSIST = 2,
        MEMORY = 3,
        WAL = 4,
        OFF = 5,
    };

    namespace internal {

        inline const std::string &to_string(journal_mode j) {
            static std::string res[] = {
                "DELETE",
                "TRUNCATE",
                "PERSIST",
                "MEMORY",
                "WAL",
                "OFF",
            };
            return res[static_cast<int>(j)];
        }

        inline std::unique_ptr<journal_mode> journal_mode_from_string(const std::string &str) {
            std::string upper_str;
            std::transform(str.begin(), str.end(), std::back_inserter(upper_str), ::toupper);
            static std::array<journal_mode, 6> all = {
                journal_mode::DELETE,
                journal_mode::TRUNCATE,
                journal_mode::PERSIST,
                journal_mode::MEMORY,
                journal_mode::WAL,
                journal_mode::OFF,
            };
            for(auto j: all) {
                if(to_string(j) == upper_str) {
                    return std::make_unique<journal_mode>(j);
                }
            }
            return {};
        }
    }
}

// #include "error_code.h"

namespace sqlite_orm {

    /**
     *  Helper class used to cast values from argv to V class
     *  which depends from column type.
     *
     */
    template<class V, typename Enable = void>
    struct row_extractor {
        //  used in sqlite3_exec (select)
        V extract(const char *row_value);

        //  used in sqlite_column (iteration, get_all)
        V extract(sqlite3_stmt *stmt, int columnIndex);
    };

    /**
     *  Specialization for arithmetic types.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_arithmetic<V>::value>> {
        V extract(const char *row_value) {
            return extract(row_value, tag());
        }

        V extract(sqlite3_stmt *stmt, int columnIndex) {
            return extract(stmt, columnIndex, tag());
        }

      private:
        using tag = arithmetic_tag_t<V>;

        V extract(const char *row_value, const int_or_smaller_tag &) {
            return static_cast<V>(atoi(row_value));
        }

        V extract(sqlite3_stmt *stmt, int columnIndex, const int_or_smaller_tag &) {
            return static_cast<V>(sqlite3_column_int(stmt, columnIndex));
        }

        V extract(const char *row_value, const bigint_tag &) {
            return static_cast<V>(atoll(row_value));
        }

        V extract(sqlite3_stmt *stmt, int columnIndex, const bigint_tag &) {
            return static_cast<V>(sqlite3_column_int64(stmt, columnIndex));
        }

        V extract(const char *row_value, const real_tag &) {
            return static_cast<V>(atof(row_value));
        }

        V extract(sqlite3_stmt *stmt, int columnIndex, const real_tag &) {
            return static_cast<V>(sqlite3_column_double(stmt, columnIndex));
        }
    };

    /**
     *  Specialization for std::string.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_same<V, std::string>::value>> {
        std::string extract(const char *row_value) {
            if(row_value) {
                return row_value;
            } else {
                return {};
            }
        }

        std::string extract(sqlite3_stmt *stmt, int columnIndex) {
            auto cStr = (const char *)sqlite3_column_text(stmt, columnIndex);
            if(cStr) {
                return cStr;
            } else {
                return {};
            }
        }
    };
#ifndef SQLITE_ORM_OMITS_CODECVT
    /**
     *  Specialization for std::wstring.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_same<V, std::wstring>::value>> {
        std::wstring extract(const char *row_value) {
            if(row_value) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.from_bytes(row_value);
            } else {
                return {};
            }
        }

        std::wstring extract(sqlite3_stmt *stmt, int columnIndex) {
            auto cStr = (const char *)sqlite3_column_text(stmt, columnIndex);
            if(cStr) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.from_bytes(cStr);
            } else {
                return {};
            }
        }
    };
#endif  //  SQLITE_ORM_OMITS_CODECVT
    /**
     *  Specialization for std::vector<char>.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_same<V, std::vector<char>>::value>> {
        std::vector<char> extract(const char *row_value) {
            if(row_value) {
                auto len = ::strlen(row_value);
                return this->go(row_value, len);
            } else {
                return {};
            }
        }

        std::vector<char> extract(sqlite3_stmt *stmt, int columnIndex) {
            auto bytes = static_cast<const char *>(sqlite3_column_blob(stmt, columnIndex));
            auto len = sqlite3_column_bytes(stmt, columnIndex);
            return this->go(bytes, len);
        }

      protected:
        std::vector<char> go(const char *bytes, size_t len) {
            if(len) {
                std::vector<char> res;
                res.reserve(len);
                std::copy(bytes, bytes + len, std::back_inserter(res));
                return res;
            } else {
                return {};
            }
        }
    };

    template<class V>
    struct row_extractor<V, std::enable_if_t<is_std_ptr<V>::value>> {
        using value_type = typename is_std_ptr<V>::element_type;

        V extract(const char *row_value) {
            if(row_value) {
                return is_std_ptr<V>::make(row_extractor<value_type>().extract(row_value));
            } else {
                return {};
            }
        }

        V extract(sqlite3_stmt *stmt, int columnIndex) {
            auto type = sqlite3_column_type(stmt, columnIndex);
            if(type != SQLITE_NULL) {
                return is_std_ptr<V>::make(row_extractor<value_type>().extract(stmt, columnIndex));
            } else {
                return {};
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct row_extractor<std::optional<T>, void> {
        using value_type = T;

        std::optional<T> extract(const char *row_value) {
            if(row_value) {
                return std::make_optional(row_extractor<value_type>().extract(row_value));
            } else {
                return std::nullopt;
            }
        }

        std::optional<T> extract(sqlite3_stmt *stmt, int columnIndex) {
            auto type = sqlite3_column_type(stmt, columnIndex);
            if(type != SQLITE_NULL) {
                return std::make_optional(row_extractor<value_type>().extract(stmt, columnIndex));
            } else {
                return std::nullopt;
            }
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Specialization for std::vector<char>.
     */
    template<>
    struct row_extractor<std::vector<char>> {
        std::vector<char> extract(const char *row_value) {
            if(row_value) {
                auto len = ::strlen(row_value);
                return this->go(row_value, len);
            } else {
                return {};
            }
        }

        std::vector<char> extract(sqlite3_stmt *stmt, int columnIndex) {
            auto bytes = static_cast<const char *>(sqlite3_column_blob(stmt, columnIndex));
            auto len = static_cast<size_t>(sqlite3_column_bytes(stmt, columnIndex));
            return this->go(bytes, len);
        }

      protected:
        std::vector<char> go(const char *bytes, size_t len) {
            if(len) {
                std::vector<char> res;
                res.reserve(len);
                std::copy(bytes, bytes + len, std::back_inserter(res));
                return res;
            } else {
                return {};
            }
        }
    };

    template<class... Args>
    struct row_extractor<std::tuple<Args...>> {

        std::tuple<Args...> extract(char **argv) {
            std::tuple<Args...> res;
            this->extract<std::tuple_size<decltype(res)>::value>(res, argv);
            return res;
        }

        std::tuple<Args...> extract(sqlite3_stmt *stmt, int /*columnIndex*/) {
            std::tuple<Args...> res;
            this->extract<std::tuple_size<decltype(res)>::value>(res, stmt);
            return res;
        }

      protected:
        template<size_t I, typename std::enable_if<I != 0>::type * = nullptr>
        void extract(std::tuple<Args...> &t, sqlite3_stmt *stmt) {
            using tuple_type = typename std::tuple_element<I - 1, typename std::tuple<Args...>>::type;
            std::get<I - 1>(t) = row_extractor<tuple_type>().extract(stmt, I - 1);
            this->extract<I - 1>(t, stmt);
        }

        template<size_t I, typename std::enable_if<I == 0>::type * = nullptr>
        void extract(std::tuple<Args...> &, sqlite3_stmt *) {
            //..
        }

        template<size_t I, typename std::enable_if<I != 0>::type * = nullptr>
        void extract(std::tuple<Args...> &t, char **argv) {
            using tuple_type = typename std::tuple_element<I - 1, typename std::tuple<Args...>>::type;
            std::get<I - 1>(t) = row_extractor<tuple_type>().extract(argv[I - 1]);
            this->extract<I - 1>(t, argv);
        }

        template<size_t I, typename std::enable_if<I == 0>::type * = nullptr>
        void extract(std::tuple<Args...> &, char **) {
            //..
        }
    };

    /**
     *  Specialization for journal_mode.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_same<V, journal_mode>::value>> {
        journal_mode extract(const char *row_value) {
            if(row_value) {
                if(auto res = internal::journal_mode_from_string(row_value)) {
                    return std::move(*res);
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::incorrect_journal_mode_string));
                }
            } else {
                throw std::system_error(std::make_error_code(orm_error_code::incorrect_journal_mode_string));
            }
        }

        journal_mode extract(sqlite3_stmt *stmt, int columnIndex) {
            auto cStr = (const char *)sqlite3_column_text(stmt, columnIndex);
            return this->extract(cStr);
        }
    };
}
#pragma once

#include <ostream>

namespace sqlite_orm {

    enum class sync_schema_result {

        /**
         *  created new table, table with the same tablename did not exist
         */
        new_table_created,

        /**
         *  table schema is the same as storage, nothing to be done
         */
        already_in_sync,

        /**
         *  removed excess columns in table (than storage) without dropping a table
         */
        old_columns_removed,

        /**
         *  lacking columns in table (than storage) added without dropping a table
         */
        new_columns_added,

        /**
         *  both old_columns_removed and new_columns_added
         */
        new_columns_added_and_old_columns_removed,

        /**
         *  old table is dropped and new is recreated. Reasons :
         *      1. delete excess columns in the table than storage if preseve = false
         *      2. Lacking columns in the table cannot be added due to NULL and DEFAULT constraint
         *      3. Reasons 1 and 2 both together
         *      4. data_type mismatch between table and storage.
         */
        dropped_and_recreated,
    };

    inline std::ostream &operator<<(std::ostream &os, sync_schema_result value) {
        switch(value) {
            case sync_schema_result::new_table_created:
                return os << "new table created";
            case sync_schema_result::already_in_sync:
                return os << "table and storage is already in sync.";
            case sync_schema_result::old_columns_removed:
                return os << "old excess columns removed";
            case sync_schema_result::new_columns_added:
                return os << "new columns added";
            case sync_schema_result::new_columns_added_and_old_columns_removed:
                return os << "old excess columns removed and new columns added";
            case sync_schema_result::dropped_and_recreated:
                return os << "old table dropped and recreated";
        }
        return os;
    }
}
#pragma once

#include <tuple>  //  std::tuple, std::make_tuple
#include <string>  //  std::string
#include <utility>  //  std::forward

namespace sqlite_orm {

    namespace internal {

        template<class... Cols>
        struct index_t {
            using columns_type = std::tuple<Cols...>;
            using object_type = void;

            std::string name;
            bool unique;
            columns_type columns;

            template<class L>
            void for_each_column_with_constraints(const L &) {}
        };
    }

    template<class... Cols>
    internal::index_t<Cols...> make_index(const std::string &name, Cols... cols) {
        return {name, false, std::make_tuple(std::forward<Cols>(cols)...)};
    }

    template<class... Cols>
    internal::index_t<Cols...> make_unique_index(const std::string &name, Cols... cols) {
        return {name, true, std::make_tuple(std::forward<Cols>(cols)...)};
    }
}
#pragma once

// #include "alias.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  If T is alias than mapped_type_proxy<T>::type is alias::type
         *  otherwise T is T.
         */
        template<class T, class sfinae = void>
        struct mapped_type_proxy {
            using type = T;
        };

        template<class T>
        struct mapped_type_proxy<T, typename std::enable_if<std::is_base_of<alias_tag, T>::value>::type> {
            using type = typename T::type;
        };
    }
}
#pragma once

#include <string>  //  std::string

namespace sqlite_orm {

    namespace internal {

        struct rowid_t {
            operator std::string() const {
                return "rowid";
            }
        };

        struct oid_t {
            operator std::string() const {
                return "oid";
            }
        };

        struct _rowid_t {
            operator std::string() const {
                return "_rowid_";
            }
        };

        template<class T>
        struct table_rowid_t : public rowid_t {
            using type = T;
        };

        template<class T>
        struct table_oid_t : public oid_t {
            using type = T;
        };
        template<class T>
        struct table__rowid_t : public _rowid_t {
            using type = T;
        };

    }

    inline internal::rowid_t rowid() {
        return {};
    }

    inline internal::oid_t oid() {
        return {};
    }

    inline internal::_rowid_t _rowid_() {
        return {};
    }

    template<class T>
    internal::table_rowid_t<T> rowid() {
        return {};
    }

    template<class T>
    internal::table_oid_t<T> oid() {
        return {};
    }

    template<class T>
    internal::table__rowid_t<T> _rowid_() {
        return {};
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_same, std::decay
#include <tuple>  //  std::tuple
#include <functional>  //  std::reference_wrapper

// #include "core_functions.h"

// #include "aggregate_functions.h"

// #include "select_constraints.h"

// #include "operators.h"

// #include "rowid.h"

// #include "alias.h"

// #include "column.h"

// #include "storage_traits.h"
#include <type_traits>  //  std::is_same, std::enable_if, std::true_type, std::false_type, std::integral_constant
#include <tuple>  //  std::tuple

namespace sqlite_orm {

    namespace internal {

        template<class... Ts>
        struct storage_impl;

        template<class T, class... Args>
        struct table_t;

        namespace storage_traits {

            /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
            template<class S, class T, class SFINAE = void>
            struct type_is_mapped_impl;

            /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
            template<class S, class T>
            struct type_is_mapped : type_is_mapped_impl<typename S::impl_type, T> {};

            /**
             *  Final specialisation
             */
            template<class T>
            struct type_is_mapped_impl<storage_impl<>, T, void> : std::false_type {};

            template<class S, class T>
            struct type_is_mapped_impl<
                S,
                T,
                typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type>
                : std::true_type {};

            template<class S, class T>
            struct type_is_mapped_impl<
                S,
                T,
                typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type>
                : type_is_mapped_impl<typename S::super, T> {};

            /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
            template<class S, class T, class SFINAE = void>
            struct storage_columns_count_impl;

            /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
            template<class S, class T>
            struct storage_columns_count : storage_columns_count_impl<typename S::impl_type, T> {};

            /**
             *  Final specialisation
             */
            template<class T>
            struct storage_columns_count_impl<storage_impl<>, T, void> : std::integral_constant<int, 0> {};

            template<class S, class T>
            struct storage_columns_count_impl<
                S,
                T,
                typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type>
                : std::integral_constant<int, S::table_type::columns_count> {};

            template<class S, class T>
            struct storage_columns_count_impl<
                S,
                T,
                typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type>
                : storage_columns_count_impl<typename S::super, T> {};

            /**
             *  T - table type.
             */
            template<class T>
            struct table_types;

            /**
             *  type is std::tuple of field types of mapped colums.
             */
            template<class T, class... Args>
            struct table_types<table_t<T, Args...>> {
                using type = std::tuple<typename Args::field_type...>;
            };

            /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
            template<class S, class T, class SFINAE = void>
            struct storage_mapped_columns_impl;

            /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
            template<class S, class T>
            struct storage_mapped_columns : storage_mapped_columns_impl<typename S::impl_type, T> {};

            /**
             *  Final specialisation
             */
            template<class T>
            struct storage_mapped_columns_impl<storage_impl<>, T, void> {
                using type = std::tuple<>;
            };

            template<class S, class T>
            struct storage_mapped_columns_impl<
                S,
                T,
                typename std::enable_if<std::is_same<T, typename S::table_type::object_type>::value>::type> {
                using table_type = typename S::table_type;
                using type = typename table_types<table_type>::type;
            };

            template<class S, class T>
            struct storage_mapped_columns_impl<
                S,
                T,
                typename std::enable_if<!std::is_same<T, typename S::table_type::object_type>::value>::type>
                : storage_mapped_columns_impl<typename S::super, T> {};

        }
    }
}

namespace sqlite_orm {

    namespace internal {

        /**
         *  This is a proxy class used to define what type must have result type depending on select
         *  arguments (member pointer, aggregate functions, etc). Below you can see specializations
         *  for different types. E.g. specialization for core_functions::length_t has `type` int cause
         *  LENGTH returns INTEGER in sqlite. Every column_result_t must have `type` type that equals
         *  c++ SELECT return type for T
         *  T - C++ type
         *  SFINAE - sfinae argument
         */
        template<class St, class T, class SFINAE = void>
        struct column_result_t;

        template<class St, class O, class F>
        struct column_result_t<St,
                               F O::*,
                               typename std::enable_if<std::is_member_pointer<F O::*>::value &&
                                                       !std::is_member_function_pointer<F O::*>::value>::type> {
            using type = F;
        };

        /**
         *  Common case for all getter types. Getter types are defined in column.h file
         */
        template<class St, class T>
        struct column_result_t<St, T, typename std::enable_if<is_getter<T>::value>::type> {
            using type = typename getter_traits<T>::field_type;
        };

        /**
         *  Common case for all setter types. Setter types are defined in column.h file
         */
        template<class St, class T>
        struct column_result_t<St, T, typename std::enable_if<is_setter<T>::value>::type> {
            using type = typename setter_traits<T>::field_type;
        };

        template<class St, class T>
        struct column_result_t<
            St,
            T,
            typename std::enable_if<is_base_of_template<T, core_functions::core_function_t>::value>::type> {
            using type = typename T::return_type;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::avg_t<T>, void> {
            using type = double;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::count_t<T>, void> {
            using type = int;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::count_asterisk_t<T>, void> {
            using type = int;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::sum_t<T>, void> {
            using type = std::unique_ptr<double>;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::total_t<T>, void> {
            using type = double;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::group_concat_single_t<T>, void> {
            using type = std::string;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::group_concat_double_t<T>, void> {
            using type = std::string;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::max_t<T>, void> {
            using type = std::unique_ptr<typename column_result_t<St, T>::type>;
        };

        template<class St, class T>
        struct column_result_t<St, aggregate_functions::min_t<T>, void> {
            using type = std::unique_ptr<typename column_result_t<St, T>::type>;
        };

        template<class St>
        struct column_result_t<St, aggregate_functions::count_asterisk_without_type, void> {
            using type = int;
        };

        template<class St, class T>
        struct column_result_t<St, distinct_t<T>, void> {
            using type = typename column_result_t<St, T>::type;
        };

        template<class St, class T>
        struct column_result_t<St, all_t<T>, void> {
            using type = typename column_result_t<St, T>::type;
        };

        template<class St, class L, class R>
        struct column_result_t<St, conc_t<L, R>, void> {
            using type = std::string;
        };

        template<class St, class L, class R>
        struct column_result_t<St, add_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, sub_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, mul_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, internal::div_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, mod_t<L, R>, void> {
            using type = double;
        };

        template<class St>
        struct column_result_t<St, rowid_t, void> {
            using type = int64;
        };

        template<class St>
        struct column_result_t<St, oid_t, void> {
            using type = int64;
        };

        template<class St>
        struct column_result_t<St, _rowid_t, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table_rowid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table_oid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table__rowid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T, class C>
        struct column_result_t<St, alias_column_t<T, C>, void> {
            using type = typename column_result_t<St, C>::type;
        };

        template<class St, class T, class F>
        struct column_result_t<St, column_pointer<T, F>> : column_result_t<St, F, void> {};

        template<class St, class... Args>
        struct column_result_t<St, columns_t<Args...>, void> {
            using type = std::tuple<typename column_result_t<St, typename std::decay<Args>::type>::type...>;
        };

        template<class St, class T, class... Args>
        struct column_result_t<St, select_t<T, Args...>> : column_result_t<St, T, void> {};

        template<class St, class T>
        struct column_result_t<St, T, typename std::enable_if<is_base_of_template<T, compound_operator>::value>::type> {
            using left_type = typename T::left_type;
            using right_type = typename T::right_type;
            using left_result = typename column_result_t<St, left_type>::type;
            using right_result = typename column_result_t<St, right_type>::type;
            static_assert(std::is_same<left_result, right_result>::value,
                          "Compound subselect queries must return same types");
            using type = left_result;
        };

        /**
         *  Result for the most simple queries like `SELECT 1`
         */
        template<class St, class T>
        struct column_result_t<St, T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
            using type = T;
        };

        /**
         *  Result for the most simple queries like `SELECT 'ototo'`
         */
        template<class St>
        struct column_result_t<St, const char *, void> {
            using type = std::string;
        };

        template<class St>
        struct column_result_t<St, std::string, void> {
            using type = std::string;
        };

        template<class St, class T, class E>
        struct column_result_t<St, as_t<T, E>, void> : column_result_t<St, typename std::decay<E>::type, void> {};

        template<class St, class T>
        struct column_result_t<St, asterisk_t<T>, void> {
            using type = typename storage_traits::storage_mapped_columns<St, T>::type;
        };

        template<class St, class T, class E>
        struct column_result_t<St, conditions::cast_t<T, E>, void> {
            using type = T;
        };

        template<class St, class R, class T, class E, class... Args>
        struct column_result_t<St, simple_case_t<R, T, E, Args...>, void> {
            using type = R;
        };

        template<class St, class A, class T, class E>
        struct column_result_t<St, conditions::like_t<A, T, E>, void> {
            using type = bool;
        };

        template<class St, class A, class T>
        struct column_result_t<St, conditions::glob_t<A, T>, void> {
            using type = bool;
        };

        template<class St, class C>
        struct column_result_t<St, conditions::negated_condition_t<C>, void> {
            using type = bool;
        };

        template<class St, class T>
        struct column_result_t<St, std::reference_wrapper<T>, void> : column_result_t<St, T, void> {};
    }
}
#pragma once

#include <string>  //  std::string
#include <type_traits>  //  std::remove_reference, std::is_same, std::is_base_of
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple_size, std::tuple_element
#include <algorithm>  //  std::reverse, std::find_if

// #include "column_result.h"

// #include "static_magic.h"

// #include "typed_comparator.h"

// #include "constraints.h"

// #include "tuple_helper.h"

// #include "table_info.h"

// #include "type_printer.h"

// #include "column.h"

namespace sqlite_orm {

    namespace internal {

        struct table_base {

            /**
             *  Table name.
             */
            const std::string name;

            bool _without_rowid = false;
        };

        /**
         *  Table interface class. Implementation is hidden in `table_impl` class.
         */
        template<class T, class... Cs>
        struct table_t : table_base {
            using object_type = T;
            using columns_type = std::tuple<Cs...>;

            static constexpr const int columns_count = static_cast<int>(std::tuple_size<columns_type>::value);

            columns_type columns;

            table_t(decltype(name) name_, columns_type columns_) :
                table_base{std::move(name_)}, columns(std::move(columns_)) {}

            table_t<T, Cs...> without_rowid() const {
                auto res = *this;
                res._without_rowid = true;
                return res;
            }

            /**
             *  Function used to get field value from object by mapped member pointer/setter/getter
             */
            template<class F, class C>
            const F *get_object_field_pointer(const object_type &obj, C c) const {
                const F *res = nullptr;
                this->for_each_column_with_field_type<F>([&res, &c, &obj](auto &col) {
                    using column_type = typename std::remove_reference<decltype(col)>::type;
                    using member_pointer_t = typename column_type::member_pointer_t;
                    using getter_type = typename column_type::getter_type;
                    using setter_type = typename column_type::setter_type;
                    // Make static_if have at least one input as a workaround for GCC bug:
                    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64095
                    if(!res) {
                        static_if<std::is_same<C, member_pointer_t>{}>([&res, &obj, &col](const C &c) {
                            if(compare_any(col.member_pointer, c)) {
                                res = &(obj.*col.member_pointer);
                            }
                        })(c);
                    }
                    if(!res) {
                        static_if<std::is_same<C, getter_type>{}>([&res, &obj, &col](const C &c) {
                            if(compare_any(col.getter, c)) {
                                res = &((obj).*(col.getter))();
                            }
                        })(c);
                    }
                    if(!res) {
                        static_if<std::is_same<C, setter_type>{}>([&res, &obj, &col](const C &c) {
                            if(compare_any(col.setter, c)) {
                                res = &((obj).*(col.getter))();
                            }
                        })(c);
                    }
                });
                return res;
            }

            /**
             *  @return vector of column names of table.
             */
            std::vector<std::string> column_names() const {
                std::vector<std::string> res;
                this->for_each_column([&res](auto &c) {
                    res.push_back(c.name);
                });
                return res;
            }

            /**
             *  Calls **l** with every primary key dedicated constraint
             */
            template<class L>
            void for_each_primary_key(const L &l) const {
                iterate_tuple(this->columns, [&l](auto &column) {
                    using column_type = typename std::decay<decltype(column)>::type;
                    static_if<internal::is_primary_key<column_type>{}>(l)(column);
                });
            }

            std::vector<std::string> composite_key_columns_names() const {
                std::vector<std::string> res;
                this->for_each_primary_key([this, &res](auto &c) {
                    res = this->composite_key_columns_names(c);
                });
                return res;
            }

            std::vector<std::string> primary_key_column_names() const {
                std::vector<std::string> res;
                this->for_each_column_with<constraints::primary_key_t<>>([&res](auto &c) {
                    res.push_back(c.name);
                });
                if(!res.size()) {
                    res = this->composite_key_columns_names();
                }
                return res;
            }

            template<class... Args>
            std::vector<std::string> composite_key_columns_names(const constraints::primary_key_t<Args...> &pk) const {
                std::vector<std::string> res;
                using pk_columns_tuple = decltype(pk.columns);
                res.reserve(std::tuple_size<pk_columns_tuple>::value);
                iterate_tuple(pk.columns, [this, &res](auto &v) {
                    res.push_back(this->find_column_name(v));
                });
                return res;
            }

            /**
             *  Searches column name by class member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
            template<class F,
                     class O,
                     typename = typename std::enable_if<std::is_member_pointer<F O::*>::value &&
                                                        !std::is_member_function_pointer<F O::*>::value>::type>
            std::string find_column_name(F O::*m) const {
                std::string res;
                this->template for_each_column_with_field_type<F>([&res, m](auto &c) {
                    if(c.member_pointer == m) {
                        res = c.name;
                    }
                });
                return res;
            }

            /**
             *  Searches column name by class getter function member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
            template<class G>
            std::string find_column_name(G getter,
                                         typename std::enable_if<is_getter<G>::value>::type * = nullptr) const {
                std::string res;
                using field_type = typename getter_traits<G>::field_type;
                this->template for_each_column_with_field_type<field_type>([&res, getter](auto &c) {
                    if(compare_any(c.getter, getter)) {
                        res = c.name;
                    }
                });
                return res;
            }

            /**
             *  Searches column name by class setter function member pointer passed as first argument.
             *  @return column name or empty string if nothing found.
             */
            template<class S>
            std::string find_column_name(S setter,
                                         typename std::enable_if<is_setter<S>::value>::type * = nullptr) const {
                std::string res;
                using field_type = typename setter_traits<S>::field_type;
                this->template for_each_column_with_field_type<field_type>([&res, setter](auto &c) {
                    if(compare_any(c.setter, setter)) {
                        res = c.name;
                    }
                });
                return res;
            }

            /**
             *  Iterates all columns and fires passed lambda. Lambda must have one and only templated argument Otherwise
             * code will not compile. Excludes table constraints (e.g. foreign_key_t) at the end of the columns list. To
             * iterate columns with table constraints use for_each_column_with_constraints instead. L is lambda type. Do
             * not specify it explicitly.
             *  @param l Lambda to be called per column itself. Must have signature like this [] (auto col) -> void {}
             */
            template<class L>
            void for_each_column(const L &l) const {
                iterate_tuple(this->columns, [&l](auto &column) {
                    using column_type = typename std::decay<decltype(column)>::type;
                    static_if<is_column<column_type>{}>(l)(column);
                });
            }

            template<class L>
            void for_each_column_with_constraints(const L &l) const {
                iterate_tuple(this->columns, l);
            }

            template<class F, class L>
            void for_each_column_with_field_type(const L &l) const {
                iterate_tuple(this->columns, [&l](auto &column) {
                    using column_type = typename std::decay<decltype(column)>::type;
                    static_if<std::is_same<F, typename column_type::field_type>{}>(l)(column);
                });
            }

            /**
             *  Iterates all columns that have specified constraints and fires passed lambda.
             *  Lambda must have one and only templated argument Otherwise code will not compile.
             *  L is lambda type. Do not specify it explicitly.
             *  @param l Lambda to be called per column itself. Must have signature like this [] (auto col) -> void {}
             */
            template<class Op, class L>
            void for_each_column_with(const L &l) const {
                using tuple_helper::tuple_contains_type;
                iterate_tuple(this->columns, [&l](auto &column) {
                    using column_type = typename std::decay<decltype(column)>::type;
                    static_if<tuple_contains_type<Op, typename column_type::constraints_type>{}>(l)(column);
                });
            }

            std::vector<table_info> get_table_info() {
                std::vector<table_info> res;
                res.reserve(size_t(this->columns_count));
                this->for_each_column([&res](auto &col) {
                    std::string dft;
                    using field_type = typename std::decay<decltype(col)>::type::field_type;
                    if(auto d = col.default_value()) {
                        dft = *d;
                    }
                    table_info i{
                        -1,
                        col.name,
                        type_printer<field_type>().print(),
                        col.not_null(),
                        dft,
                        col.template has<constraints::primary_key_t<>>(),
                    };
                    res.emplace_back(i);
                });
                auto compositeKeyColumnNames = this->composite_key_columns_names();
                for(size_t i = 0; i < compositeKeyColumnNames.size(); ++i) {
                    auto &columnName = compositeKeyColumnNames[i];
                    auto it = std::find_if(res.begin(), res.end(), [&columnName](const table_info &ti) {
                        return ti.name == columnName;
                    });
                    if(it != res.end()) {
                        it->pk = static_cast<int>(i + 1);
                    }
                }
                return res;
            }
        };
    }

    /**
     *  Function used for table creation. Do not use table constructor - use this function
     *  cause table class is templated and its constructing too (just like std::make_unique or std::make_pair).
     */
    template<class... Cs, class T = typename std::tuple_element<0, std::tuple<Cs...>>::type::object_type>
    internal::table_t<T, Cs...> make_table(const std::string &name, Cs... args) {
        return {name, std::make_tuple<Cs...>(std::forward<Cs>(args)...)};
    }

    template<class T, class... Cs>
    internal::table_t<T, Cs...> make_table(const std::string &name, Cs... args) {
        return {name, std::make_tuple<Cs...>(std::forward<Cs>(args)...)};
    }
}
#pragma once

#include <string>  //  std::string
#include <sqlite3.h>
#include <cstddef>  //  std::nullptr_t
#include <system_error>  //  std::system_error, std::error_code
#include <sstream>  //  std::stringstream
#include <cstdlib>  //  std::atoi
#include <type_traits>  //  std::forward, std::enable_if, std::is_same, std::remove_reference, std::false_type, std::true_type
#include <utility>  //  std::pair, std::make_pair
#include <vector>  //  std::vector
#include <algorithm>  //  std::find_if

// #include "error_code.h"

// #include "statement_finalizer.h"

// #include "row_extractor.h"

// #include "constraints.h"

// #include "select_constraints.h"

// #include "field_printer.h"

// #include "table_info.h"

// #include "sync_schema_result.h"

// #include "sqlite_type.h"

// #include "field_value_holder.h"

#include <type_traits>  //  std::enable_if

// #include "column.h"

namespace sqlite_orm {
    namespace internal {

        template<class T, class SFINAE = void>
        struct field_value_holder;

        template<class T>
        struct field_value_holder<T, typename std::enable_if<getter_traits<T>::returns_lvalue>::type> {
            using type = typename getter_traits<T>::field_type;

            const type &value;
        };

        template<class T>
        struct field_value_holder<T, typename std::enable_if<!getter_traits<T>::returns_lvalue>::type> {
            using type = typename getter_traits<T>::field_type;

            type value;
        };
    }
}

namespace sqlite_orm {

    namespace internal {

        struct storage_impl_base {

            bool table_exists(const std::string &tableName, sqlite3 *db) {
                auto res = false;
                std::stringstream ss;
                ss << "SELECT COUNT(*) FROM sqlite_master WHERE type = '"
                   << "table"
                   << "' AND name = '" << tableName << "'";
                auto query = ss.str();
                auto rc = sqlite3_exec(
                    db,
                    query.c_str(),
                    [](void *data, int argc, char **argv, char * * /*azColName*/) -> int {
                        auto &res = *(bool *)data;
                        if(argc) {
                            res = !!std::atoi(argv[0]);
                        }
                        return 0;
                    },
                    &res,
                    nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }

            void rename_table(sqlite3 *db, const std::string &oldName, const std::string &newName) {
                std::stringstream ss;
                ss << "ALTER TABLE " << oldName << " RENAME TO " << newName;
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            static bool get_remove_add_columns(std::vector<table_info *> &columnsToAdd,
                                               std::vector<table_info> &storageTableInfo,
                                               std::vector<table_info> &dbTableInfo) {
                bool notEqual = false;

                //  iterate through storage columns
                for(size_t storageColumnInfoIndex = 0; storageColumnInfoIndex < storageTableInfo.size();
                    ++storageColumnInfoIndex) {

                    //  get storage's column info
                    auto &storageColumnInfo = storageTableInfo[storageColumnInfoIndex];
                    auto &columnName = storageColumnInfo.name;

                    //  search for a column in db eith the same name
                    auto dbColumnInfoIt = std::find_if(dbTableInfo.begin(), dbTableInfo.end(), [&columnName](auto &ti) {
                        return ti.name == columnName;
                    });
                    if(dbColumnInfoIt != dbTableInfo.end()) {
                        auto &dbColumnInfo = *dbColumnInfoIt;
                        auto dbColumnInfoType = to_sqlite_type(dbColumnInfo.type);
                        auto storageColumnInfoType = to_sqlite_type(storageColumnInfo.type);
                        if(dbColumnInfoType && storageColumnInfoType) {
                            auto columnsAreEqual =
                                dbColumnInfo.name == storageColumnInfo.name &&
                                *dbColumnInfoType == *storageColumnInfoType &&
                                dbColumnInfo.notnull == storageColumnInfo.notnull &&
                                (dbColumnInfo.dflt_value.length() > 0) == (storageColumnInfo.dflt_value.length() > 0) &&
                                dbColumnInfo.pk == storageColumnInfo.pk;
                            if(!columnsAreEqual) {
                                notEqual = true;
                                break;
                            }
                            dbTableInfo.erase(dbColumnInfoIt);
                            storageTableInfo.erase(storageTableInfo.begin() +
                                                   static_cast<ptrdiff_t>(storageColumnInfoIndex));
                            --storageColumnInfoIndex;
                        } else {

                            //  undefined type/types
                            notEqual = true;
                            break;
                        }
                    } else {
                        columnsToAdd.push_back(&storageColumnInfo);
                    }
                }
                return notEqual;
            }

            std::vector<table_info> get_table_info(const std::string &tableName, sqlite3 *db) {
                std::vector<table_info> res;
                auto query = "PRAGMA table_info('" + tableName + "')";
                auto rc = sqlite3_exec(
                    db,
                    query.c_str(),
                    [](void *data, int argc, char **argv, char **) -> int {
                        auto &res = *(std::vector<table_info> *)data;
                        if(argc) {
                            auto index = 0;
                            auto cid = std::atoi(argv[index++]);
                            std::string name = argv[index++];
                            std::string type = argv[index++];
                            bool notnull = !!std::atoi(argv[index++]);
                            std::string dflt_value = argv[index] ? argv[index] : "";
                            index++;
                            auto pk = std::atoi(argv[index++]);
                            res.push_back(table_info{cid, name, type, notnull, dflt_value, pk});
                        }
                        return 0;
                    },
                    &res,
                    nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }
        };

        /**
         *  This is a generic implementation. Used as a tail in storage_impl inheritance chain
         */
        template<class... Ts>
        struct storage_impl;

        template<class H, class... Ts>
        struct storage_impl<H, Ts...> : public storage_impl<Ts...> {
            using table_type = H;
            using super = storage_impl<Ts...>;

            storage_impl(H h, Ts... ts) : super(std::forward<Ts>(ts)...), table(std::move(h)) {}

            table_type table;

            template<class L>
            void for_each(const L &l) {
                this->super::for_each(l);
                l(this);
            }

#if SQLITE_VERSION_NUMBER >= 3006019

            /**
             *  Returns foreign keys count in table definition
             */
            int foreign_keys_count() {
                auto res = 0;
                this->table.for_each_column_with_constraints([&res](auto &c) {
                    if(internal::is_foreign_key<typename std::decay<decltype(c)>::type>::value) {
                        ++res;
                    }
                });
                return res;
            }

#endif

            /**
             *  Is used to get column name by member pointer to a base class.
             *  Main difference between `column_name` and `column_name_simple` is that
             *  `column_name` has SFINAE check for type equality but `column_name_simple` has not.
             */
            template<class O, class F>
            std::string column_name_simple(F O::*m) const {
                return this->table.find_column_name(m);
            }

            /**
             *  Same thing as above for getter.
             */
            template<class T, typename std::enable_if<is_getter<T>::value>::type>
            std::string column_name_simple(T g) const {
                return this->table.find_column_name(g);
            }

            /**
             *  Same thing as above for setter.
             */
            template<class T, typename std::enable_if<is_setter<T>::value>::type>
            std::string column_name_simple(T s) const {
                return this->table.find_column_name(s);
            }

            /**
             *  Cute function used to find column name by its type and member pointer. Uses SFINAE to
             *  skip inequal type O.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(F O::*m,
                                    typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->table.find_column_name(m);
            }

            /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(F O::*m,
                                    typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->super::column_name(m);
            }

            /**
             *  Cute function used to find column name by its type and getter pointer. Uses SFINAE to
             *  skip inequal type O.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(const F &(O::*g)() const,
                                    typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->table.find_column_name(g);
            }

            /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(const F &(O::*g)() const,
                                    typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->super::column_name(g);
            }

            /**
             *  Cute function used to find column name by its type and setter pointer. Uses SFINAE to
             *  skip inequal type O.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(void (O::*s)(F),
                                    typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->table.find_column_name(s);
            }

            /**
             *  Opposite version of function defined above. Just calls same function in superclass.
             */
            template<class O, class F, class HH = typename H::object_type>
            std::string column_name(void (O::*s)(F),
                                    typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->super::column_name(s);
            }

            template<class T, class F, class HH = typename H::object_type>
            std::string column_name(const column_pointer<T, F> &c,
                                    typename std::enable_if<std::is_same<T, HH>::value>::type * = nullptr) const {
                return this->column_name_simple(c.field);
            }

            template<class T, class F, class HH = typename H::object_type>
            std::string column_name(const column_pointer<T, F> &c,
                                    typename std::enable_if<!std::is_same<T, HH>::value>::type * = nullptr) const {
                return this->super::column_name(c);
            }

            template<class O, class HH = typename H::object_type>
            auto &get_impl(typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) const {
                return *this;
            }

            template<class O, class HH = typename H::object_type>
            auto &get_impl(typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->super::template get_impl<O>();
            }

            template<class O, class HH = typename H::object_type>
            std::string find_table_name(typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->table.name;
            }

            template<class O, class HH = typename H::object_type>
            std::string find_table_name(typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) const {
                return this->super::template find_table_name<O>();
            }

            template<class O, class HH = typename H::object_type>
            std::string dump(const O &o, typename std::enable_if<!std::is_same<O, HH>::value>::type * = nullptr) {
                return this->super::dump(o, nullptr);
            }

            template<class O, class HH = typename H::object_type>
            std::string dump(const O &o, typename std::enable_if<std::is_same<O, HH>::value>::type * = nullptr) {
                std::stringstream ss;
                ss << "{ ";
                using pair = std::pair<std::string, std::string>;
                std::vector<pair> pairs;
                this->table.for_each_column([&pairs, &o](auto &c) {
                    using column_type = typename std::decay<decltype(c)>::type;
                    using field_type = typename column_type::field_type;
                    pair p{c.name, ""};
                    if(c.member_pointer) {
                        p.second = field_printer<field_type>()(o.*c.member_pointer);
                    } else {
                        using getter_type = typename column_type::getter_type;
                        field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                        p.second = field_printer<field_type>()(valueHolder.value);
                    }
                    pairs.push_back(std::move(p));
                });
                for(size_t i = 0; i < pairs.size(); ++i) {
                    auto &p = pairs[i];
                    ss << p.first << " : '" << p.second << "'";
                    if(i < pairs.size() - 1) {
                        ss << ", ";
                    } else {
                        ss << " }";
                    }
                }
                return ss.str();
            }

            void add_column(const table_info &ti, sqlite3 *db) {
                std::stringstream ss;
                ss << "ALTER TABLE " << this->table.name << " ADD COLUMN " << ti.name << " ";
                ss << ti.type << " ";
                if(ti.pk) {
                    ss << "PRIMARY KEY ";
                }
                if(ti.notnull) {
                    ss << "NOT NULL ";
                }
                if(ti.dflt_value.length()) {
                    ss << "DEFAULT " << ti.dflt_value << " ";
                }
                auto query = ss.str();
                sqlite3_stmt *stmt;
                auto prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
                if(prepareResult == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            /**
             *  Copies current table to another table with a given **name**.
             *  Performs CREATE TABLE %name% AS SELECT %this->table.columns_names()% FROM &this->table.name%;
             */
            void copy_table(sqlite3 *db, const std::string &name) {
                std::stringstream ss;
                std::vector<std::string> columnNames;
                this->table.for_each_column([&columnNames](auto &c) {
                    columnNames.emplace_back(c.name);
                });
                auto columnNamesCount = columnNames.size();
                ss << "INSERT INTO " << name << " (";
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << columnNames[i];
                    if(i < columnNamesCount - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << ") ";
                ss << "SELECT ";
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << columnNames[i];
                    if(i < columnNamesCount - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << "FROM '" << this->table.name << "' ";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            sync_schema_result schema_status(sqlite3 *db, bool preserve) {

                auto res = sync_schema_result::already_in_sync;

                //  first let's see if table with such name exists..
                auto gottaCreateTable = !this->table_exists(this->table.name, db);
                if(!gottaCreateTable) {

                    //  get table info provided in `make_table` call..
                    auto storageTableInfo = this->table.get_table_info();

                    //  now get current table info from db using `PRAGMA table_info` query..
                    auto dbTableInfo = this->get_table_info(this->table.name, db);

                    //  this vector will contain pointers to columns that gotta be added..
                    std::vector<table_info *> columnsToAdd;

                    if(this->get_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo)) {
                        gottaCreateTable = true;
                    }

                    if(!gottaCreateTable) {  //  if all storage columns are equal to actual db columns but there are
                        //  excess columns at the db..
                        if(dbTableInfo.size() > 0) {
                            // extra table columns than storage columns
                            if(!preserve) {
                                gottaCreateTable = true;
                            } else {
                                res = decltype(res)::old_columns_removed;
                            }
                        }
                    }
                    if(gottaCreateTable) {
                        res = decltype(res)::dropped_and_recreated;
                    } else {
                        if(columnsToAdd.size()) {
                            // extra storage columns than table columns
                            for(auto columnPointer: columnsToAdd) {
                                if(columnPointer->notnull && columnPointer->dflt_value.empty()) {
                                    gottaCreateTable = true;
                                    break;
                                }
                            }
                            if(!gottaCreateTable) {
                                if(res == decltype(res)::old_columns_removed) {
                                    res = decltype(res)::new_columns_added_and_old_columns_removed;
                                } else {
                                    res = decltype(res)::new_columns_added;
                                }
                            } else {
                                res = decltype(res)::dropped_and_recreated;
                            }
                        } else {
                            if(res != decltype(res)::old_columns_removed) {
                                res = decltype(res)::already_in_sync;
                            }
                        }
                    }
                } else {
                    res = decltype(res)::new_table_created;
                }
                return res;
            }

          private:
            using self = storage_impl<H, Ts...>;
        };

        template<>
        struct storage_impl<> : storage_impl_base {

            template<class O>
            std::string find_table_name() const {
                return {};
            }

            template<class L>
            void for_each(const L &) {}

            int foreign_keys_count() {
                return 0;
            }

            template<class O>
            std::string dump(const O &, sqlite3 *, std::nullptr_t) {
                throw std::system_error(std::make_error_code(orm_error_code::type_is_not_mapped_to_storage));
            }
        };

        template<class T>
        struct is_storage_impl : std::false_type {};

        template<class... Ts>
        struct is_storage_impl<storage_impl<Ts...>> : std::true_type {};
    }
}
#pragma once

#include <memory>  //  std::unique/shared_ptr, std::make_unique/shared
#include <string>  //  std::string
#include <sqlite3.h>
#include <type_traits>  //  std::remove_reference, std::is_base_of, std::decay, std::false_type, std::true_type
#include <cstddef>  //  std::ptrdiff_t
#include <iterator>  //  std::input_iterator_tag, std::iterator_traits, std::distance
#include <functional>  //  std::function
#include <sstream>  //  std::stringstream
#include <map>  //  std::map
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple_size, std::tuple, std::make_tuple
#include <utility>  //  std::forward, std::pair
#include <set>  //  std::set
#include <algorithm>  //  std::find

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "alias.h"

// #include "row_extractor.h"

// #include "error_code.h"

// #include "type_printer.h"

// #include "tuple_helper.h"

// #include "constraints.h"

// #include "table_type.h"

// #include "type_is_nullable.h"

// #include "field_printer.h"

// #include "rowid.h"

// #include "aggregate_functions.h"

// #include "operators.h"

// #include "select_constraints.h"

// #include "core_functions.h"

// #include "conditions.h"

// #include "statement_binder.h"

// #include "column_result.h"

// #include "mapped_type_proxy.h"

// #include "sync_schema_result.h"

// #include "table_info.h"

// #include "storage_impl.h"

// #include "journal_mode.h"

// #include "field_value_holder.h"

// #include "view.h"

#include <memory>  //  std::shared_ptr
#include <string>  //  std::string
#include <utility>  //  std::forward, std::move
#include <sqlite3.h>
#include <system_error>  //  std::system_error
#include <tuple>  //  std::tuple, std::make_tuple

// #include "row_extractor.h"

// #include "statement_finalizer.h"

// #include "error_code.h"

// #include "iterator.h"

#include <memory>  //  std::shared_ptr, std::unique_ptr, std::make_shared
#include <sqlite3.h>
#include <type_traits>  //  std::decay
#include <utility>  //  std::move
#include <cstddef>  //  std::ptrdiff_t
#include <iterator>  //  std::input_iterator_tag
#include <system_error>  //  std::system_error
#include <ios>  //  std::make_error_code

// #include "row_extractor.h"

// #include "statement_finalizer.h"

// #include "error_code.h"

namespace sqlite_orm {

    namespace internal {

        template<class V>
        struct iterator_t {
            using view_type = V;
            using value_type = typename view_type::mapped_type;

          protected:
            /**
             *  The double-indirection is so that copies of the iterator
             *  share the same sqlite3_stmt from a sqlite3_prepare_v2()
             *  call. When one finishes iterating it the pointer
             *  inside the shared_ptr is nulled out in all copies.
             */
            std::shared_ptr<sqlite3_stmt *> stmt;
            view_type &view;

            /**
             *  shared_ptr is used over unique_ptr here
             *  so that the iterator can be copyable.
             */
            std::shared_ptr<value_type> current;

            void extract_value(std::unique_ptr<value_type> &temp) {
                temp = std::make_unique<value_type>();
                auto &storage = this->view.storage;
                auto &impl = storage.template get_impl<value_type>();
                auto index = 0;
                impl.table.for_each_column([&index, &temp, this](auto &c) {
                    using field_type = typename std::decay<decltype(c)>::type::field_type;
                    auto value = row_extractor<field_type>().extract(*this->stmt, index++);
                    if(c.member_pointer) {
                        auto member_pointer = c.member_pointer;
                        (*temp).*member_pointer = std::move(value);
                    } else {
                        ((*temp).*(c.setter))(std::move(value));
                    }
                });
            }

          public:
            using difference_type = std::ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::input_iterator_tag;

            iterator_t(sqlite3_stmt *stmt_, view_type &view_) :
                stmt(std::make_shared<sqlite3_stmt *>(stmt_)), view(view_) {
                this->operator++();
            }

            iterator_t(const iterator_t &) = default;

            iterator_t(iterator_t &&) = default;

            iterator_t &operator=(iterator_t &&) = default;

            iterator_t &operator=(const iterator_t &) = default;

            ~iterator_t() {
                if(this->stmt) {
                    statement_finalizer f{*this->stmt};
                }
            }

            value_type &operator*() {
                if(!this->stmt) {
                    throw std::system_error(std::make_error_code(orm_error_code::trying_to_dereference_null_iterator));
                }
                if(!this->current) {
                    std::unique_ptr<value_type> value;
                    this->extract_value(value);
                    this->current = move(value);
                }
                return *this->current;
            }

            value_type *operator->() {
                return &(this->operator*());
            }

            void operator++() {
                if(this->stmt && *this->stmt) {
                    auto ret = sqlite3_step(*this->stmt);
                    switch(ret) {
                        case SQLITE_ROW:
                            this->current = nullptr;
                            break;
                        case SQLITE_DONE: {
                            statement_finalizer f{*this->stmt};
                            *this->stmt = nullptr;
                        } break;
                        default: {
                            auto db = this->view.connection.get();
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                }
            }

            void operator++(int) {
                this->operator++();
            }

            bool operator==(const iterator_t &other) const {
                if(this->stmt && other.stmt) {
                    return *this->stmt == *other.stmt;
                } else {
                    if(!this->stmt && !other.stmt) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }

            bool operator!=(const iterator_t &other) const {
                return !(*this == other);
            }
        };
    }
}

// #include "ast_iterator.h"

#include <vector>  //  std::vector
#include <functional>  //  std::reference_wrapper

// #include "conditions.h"

// #include "select_constraints.h"

// #include "operators.h"

// #include "tuple_helper.h"

// #include "core_functions.h"

// #include "prepared_statement.h"

#include <sqlite3.h>
#include <iterator>  //  std::iterator_traits
#include <string>  //  std::string
#include <type_traits>  //  std::true_type, std::false_type
#include <utility>  //  std::pair

// #include "connection_holder.h"

#include <sqlite3.h>
#include <string>  //  std::string
#include <system_error>  //  std::system_error

// #include "error_code.h"

namespace sqlite_orm {

    namespace internal {

        struct connection_holder {

            connection_holder(std::string filename_) : filename(move(filename_)) {}

            void retain() {
                ++this->_retain_count;
                if(1 == this->_retain_count) {
                    auto rc = sqlite3_open(this->filename.c_str(), &this->db);
                    if(rc != SQLITE_OK) {
                        throw std::system_error(std::error_code(sqlite3_errcode(this->db), get_sqlite_error_category()),
                                                sqlite3_errmsg(this->db));
                    }
                }
            }

            void release() {
                --this->_retain_count;
                if(0 == this->_retain_count) {
                    auto rc = sqlite3_close(this->db);
                    if(rc != SQLITE_OK) {
                        throw std::system_error(std::error_code(sqlite3_errcode(this->db), get_sqlite_error_category()),
                                                sqlite3_errmsg(this->db));
                    }
                }
            }

            sqlite3 *get() const {
                return this->db;
            }

            int retain_count() const {
                return this->_retain_count;
            }

            const std::string filename;

          protected:
            sqlite3 *db = nullptr;
            int _retain_count = 0;
        };

        struct connection_ref {
            connection_ref(connection_holder &holder_) : holder(holder_) {
                this->holder.retain();
            }

            connection_ref(const connection_ref &other) : holder(other.holder) {
                this->holder.retain();
            }

            connection_ref(connection_ref &&other) : holder(other.holder) {
                this->holder.retain();
            }

            ~connection_ref() {
                this->holder.release();
            }

            sqlite3 *get() const {
                return this->holder.get();
            }

          protected:
            connection_holder &holder;
        };
    }
}

// #include "select_constraints.h"

namespace sqlite_orm {

    namespace internal {

        struct prepared_statement_base {
            sqlite3_stmt *stmt = nullptr;
            connection_ref con;

            ~prepared_statement_base() {
                if(this->stmt) {
                    sqlite3_finalize(this->stmt);
                    this->stmt = nullptr;
                }
            }

            std::string sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_sql(this->stmt)) {
                        return res;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }

#if SQLITE_VERSION_NUMBER >= 3014000
            std::string expanded_sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_expanded_sql(this->stmt)) {
                        std::string result = res;
                        sqlite3_free(res);
                        return result;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }
#endif
#if SQLITE_VERSION_NUMBER >= 3026000 and defined(SQLITE_ENABLE_NORMALIZE)
            std::string normalized_sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_normalized_sql(this->stmt)) {
                        return res;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }
#endif
        };

        template<class T>
        struct prepared_statement_t : prepared_statement_base {
            using expression_type = T;

            expression_type t;

            prepared_statement_t(T t_, sqlite3_stmt *stmt, connection_ref con_) :
                prepared_statement_base{stmt, std::move(con_)}, t(std::move(t_)) {}
        };

        template<class T, class... Args>
        struct get_all_t {
            using type = T;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

        template<class T, class... Args>
        struct get_all_pointer_t {
            using type = T;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct get_all_optional_t {
            using type = T;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T, class... Wargs>
        struct update_all_t;

        template<class... Args, class... Wargs>
        struct update_all_t<set_t<Args...>, Wargs...> {
            using set_type = set_t<Args...>;
            using conditions_type = std::tuple<Wargs...>;

            set_type set;
            conditions_type conditions;
        };

        template<class T, class... Args>
        struct remove_all_t {
            using type = T;
            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

        template<class T, class... Ids>
        struct get_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

        template<class T, class... Ids>
        struct get_pointer_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Ids>
        struct get_optional_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T>
        struct update_t {
            using type = T;

            type obj;
        };

        template<class T, class... Ids>
        struct remove_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

        template<class T>
        struct insert_t {
            using type = T;

            type obj;
        };

        template<class T, class... Cols>
        struct insert_explicit {
            using type = T;
            using columns_type = columns_t<Cols...>;

            type obj;
            columns_type columns;
        };

        template<class T>
        struct replace_t {
            using type = T;

            type obj;
        };

        template<class It>
        struct insert_range_t {
            using iterator_type = It;
            using object_type = typename std::iterator_traits<iterator_type>::value_type;

            std::pair<iterator_type, iterator_type> range;
        };

        template<class It>
        struct replace_range_t {
            using iterator_type = It;
            using object_type = typename std::iterator_traits<iterator_type>::value_type;

            std::pair<iterator_type, iterator_type> range;
        };
    }

    /**
     *  Create a replace range statement
     */
    template<class It>
    internal::replace_range_t<It> replace_range(It from, It to) {
        return {{std::move(from), std::move(to)}};
    }

    /**
     *  Create an insert range statement
     */
    template<class It>
    internal::insert_range_t<It> insert_range(It from, It to) {
        return {{std::move(from), std::move(to)}};
    }

    /**
     *  Create a replace statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.replace(myUserInstance);
     *  Parameter obj is accepted by value. Is you want to accept it by ref
     *  the use std::ref function: storage.replace(std::ref(myUserInstance));
     */
    template<class T>
    internal::replace_t<T> replace(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create an insert statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.insert(myUserInstance);
     *  Parameter obj is accepted by value. Is you want to accept it by ref
     *  the use std::ref function: storage.insert(std::ref(myUserInstance));
     */
    template<class T>
    internal::insert_t<T> insert(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create an explicit insert statement.
     *  T is an object type mapped to a storage.
     *  Cols is columns types aparameter pack. Must contain member pointers
     *  Usage: storage.insert(myUserInstance, columns(&User::id, &User::name));
     *  Parameter obj is accepted by value. Is you want to accept it by ref
     *  the use std::ref function: storage.insert(std::ref(myUserInstance), columns(&User::id, &User::name));
     */
    template<class T, class... Cols>
    internal::insert_explicit<T, Cols...> insert(T obj, internal::columns_t<Cols...> cols) {
        return {std::move(obj), std::move(cols)};
    }

    /**
     *  Create a remove statement
     *  T is an object type mapped to a storage.
     *  Usage: remove<User>(5);
     */
    template<class T, class... Ids>
    internal::remove_t<T, Ids...> remove(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

    /**
     *  Create an update statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.update(myUserInstance);
     *  Parameter obj is accepted by value. Is you want to accept it by ref
     *  the use std::ref function: storage.update(std::ref(myUserInstance));
     */
    template<class T>
    internal::update_t<T> update(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create a get statement.
     *  T is an object type mapped to a storage.
     *  Usage: get<User>(5);
     */
    template<class T, class... Ids>
    internal::get_t<T, Ids...> get(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

    /**
     *  Create a get pointer statement.
     *  T is an object type mapped to a storage.
     *  Usage: get_pointer<User>(5);
     */
    template<class T, class... Ids>
    internal::get_pointer_t<T, Ids...> get_pointer(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Create a get optional statement.
     *  T is an object type mapped to a storage.
     *  Usage: get_optional<User>(5);
     */
    template<class T, class... Ids>
    internal::get_optional_t<T, Ids...> get_optional(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

    /**
     *  Create a remove all statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.remove_all<User>(...);
     */
    template<class T, class... Args>
    internal::remove_all_t<T, Args...> remove_all(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create a get all statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_t<T, Args...> get_all(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create an update all statement.
     *  Usage: storage.update_all(set(...), ...);
     */
    template<class... Args, class... Wargs>
    internal::update_all_t<internal::set_t<Args...>, Wargs...> update_all(internal::set_t<Args...> set, Wargs... wh) {
        using args_tuple = std::tuple<Wargs...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Wargs>(wh)...};
        return {std::move(set), move(conditions)};
    }

    /**
     *  Create a get all pointer statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all_pointer<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_pointer_t<T, Args...> get_all_pointer(Args... args) {
        std::tuple<Args...> conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Create a get all optional statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all_optional<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_optional_t<T, Args...> get_all_optional(Args... args) {
        std::tuple<Args...> conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
}

namespace sqlite_orm {

    namespace internal {

        /**
         *  ast_iterator accepts any expression and a callable object
         *  which will be called for any node of provided expression.
         *  E.g. if we pass `where(is_equal(5, max(&User::id, 10))` then
         *  callable object will be called with 5, &User::id and 10.
         *  ast_iterator is used mostly in finding literals to be bound to
         *  a statement. To use it just call `iterate_ast(object, callable);`
         *  T is an ast element. E.g. where_t
         */
        template<class T, class SFINAE = void>
        struct ast_iterator {
            using node_type = T;

            /**
             *  L is a callable type. Mostly is a templated lambda
             */
            template<class L>
            void operator()(const T &t, const L &l) const {
                l(t);
            }
        };

        /**
         *  Simplified API
         */
        template<class T, class L>
        void iterate_ast(const T &t, const L &l) {
            ast_iterator<T> iterator;
            iterator(t, l);
        }

        template<class T>
        struct ast_iterator<std::reference_wrapper<T>, void> {
            using node_type = std::reference_wrapper<T>;

            template<class L>
            void operator()(const node_type &r, const L &l) const {
                iterate_ast(r.get(), l);
            }
        };

        template<class C>
        struct ast_iterator<conditions::where_t<C>, void> {
            using node_type = conditions::where_t<C>;

            template<class L>
            void operator()(const node_type &where, const L &l) const {
                iterate_ast(where.c, l);
            }
        };

        template<class T>
        struct ast_iterator<
            T,
            typename std::enable_if<is_base_of_template<T, conditions::binary_condition>::value>::type> {
            using node_type = T;

            template<class L>
            void operator()(const node_type &binaryCondition, const L &l) const {
                iterate_ast(binaryCondition.l, l);
                iterate_ast(binaryCondition.r, l);
            }
        };

        template<class L, class R, class... Ds>
        struct ast_iterator<binary_operator<L, R, Ds...>, void> {
            using node_type = binary_operator<L, R, Ds...>;

            template<class C>
            void operator()(const node_type &binaryOperator, const C &l) const {
                iterate_ast(binaryOperator.lhs, l);
                iterate_ast(binaryOperator.rhs, l);
            }
        };

        template<class... Args>
        struct ast_iterator<columns_t<Args...>, void> {
            using node_type = columns_t<Args...>;

            template<class L>
            void operator()(const node_type &cols, const L &l) const {
                iterate_ast(cols.columns, l);
            }
        };

        template<class L, class A>
        struct ast_iterator<conditions::in_t<L, A>, void> {
            using node_type = conditions::in_t<L, A>;

            template<class C>
            void operator()(const node_type &in, const C &l) const {
                iterate_ast(in.l, l);
                iterate_ast(in.arg, l);
            }
        };

        template<class T>
        struct ast_iterator<std::vector<T>, void> {
            using node_type = std::vector<T>;

            template<class L>
            void operator()(const node_type &vec, const L &l) const {
                for(auto &i: vec) {
                    iterate_ast(i, l);
                }
            }
        };

        template<>
        struct ast_iterator<std::vector<char>, void> {
            using node_type = std::vector<char>;

            template<class L>
            void operator()(const node_type &vec, const L &l) const {
                l(vec);
            }
        };

        template<class T>
        struct ast_iterator<T, typename std::enable_if<is_base_of_template<T, compound_operator>::value>::type> {
            using node_type = T;

            template<class L>
            void operator()(const node_type &c, const L &l) const {
                iterate_ast(c.left, l);
                iterate_ast(c.right, l);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<select_t<T, Args...>, void> {
            using node_type = select_t<T, Args...>;

            template<class L>
            void operator()(const node_type &sel, const L &l) const {
                iterate_ast(sel.col, l);
                iterate_ast(sel.conditions, l);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<get_all_t<T, Args...>, void> {
            using node_type = get_all_t<T, Args...>;

            template<class L>
            void operator()(const node_type &get, const L &l) const {
                iterate_ast(get.conditions, l);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<get_all_pointer_t<T, Args...>, void> {
            using node_type = get_all_pointer_t<T, Args...>;

            template<class L>
            void operator()(const node_type &get, const L &l) const {
                iterate_ast(get.conditions, l);
            }
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct ast_iterator<get_all_optional_t<T, Args...>, void> {
            using node_type = get_all_optional_t<T, Args...>;

            template<class L>
            void operator()(const node_type &get, const L &l) const {
                iterate_ast(get.conditions, l);
            }
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class... Args, class... Wargs>
        struct ast_iterator<update_all_t<set_t<Args...>, Wargs...>, void> {
            using node_type = update_all_t<set_t<Args...>, Wargs...>;

            template<class L>
            void operator()(const node_type &u, const L &l) const {
                iterate_ast(u.set, l);
                iterate_ast(u.conditions, l);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<remove_all_t<T, Args...>, void> {
            using node_type = remove_all_t<T, Args...>;

            template<class L>
            void operator()(const node_type &r, const L &l) const {
                iterate_ast(r.conditions, l);
            }
        };

        template<class... Args>
        struct ast_iterator<set_t<Args...>, void> {
            using node_type = set_t<Args...>;

            template<class L>
            void operator()(const node_type &s, const L &l) const {
                iterate_ast(s.assigns, l);
            }
        };

        template<class... Args>
        struct ast_iterator<std::tuple<Args...>, void> {
            using node_type = std::tuple<Args...>;

            template<class L>
            void operator()(const node_type &tuple, const L &l) const {
                iterate_tuple(tuple, [&l](auto &v) {
                    iterate_ast(v, l);
                });
            }
        };

        template<class T>
        struct ast_iterator<conditions::having_t<T>, void> {
            using node_type = conditions::having_t<T>;

            template<class L>
            void operator()(const node_type &hav, const L &l) const {
                iterate_ast(hav.t, l);
            }
        };

        template<class T, class E>
        struct ast_iterator<conditions::cast_t<T, E>, void> {
            using node_type = conditions::cast_t<T, E>;

            template<class L>
            void operator()(const node_type &c, const L &l) const {
                iterate_ast(c.expression, l);
            }
        };

        template<class T>
        struct ast_iterator<conditions::exists_t<T>, void> {
            using node_type = conditions::exists_t<T>;

            template<class L>
            void operator()(const node_type &e, const L &l) const {
                iterate_ast(e.t, l);
            }
        };

        template<class A, class T, class E>
        struct ast_iterator<conditions::like_t<A, T, E>, void> {
            using node_type = conditions::like_t<A, T, E>;

            template<class L>
            void operator()(const node_type &lk, const L &l) const {
                iterate_ast(lk.arg, l);
                iterate_ast(lk.pattern, l);
                lk.arg3.apply([&l](auto &value) {
                    iterate_ast(value, l);
                });
            }
        };

        template<class A, class T>
        struct ast_iterator<conditions::glob_t<A, T>, void> {
            using node_type = conditions::glob_t<A, T>;

            template<class L>
            void operator()(const node_type &lk, const L &l) const {
                iterate_ast(lk.arg, l);
                iterate_ast(lk.pattern, l);
            }
        };

        template<class A, class T>
        struct ast_iterator<conditions::between_t<A, T>, void> {
            using node_type = conditions::between_t<A, T>;

            template<class L>
            void operator()(const node_type &b, const L &l) const {
                iterate_ast(b.expr, l);
                iterate_ast(b.b1, l);
                iterate_ast(b.b2, l);
            }
        };

        template<class T>
        struct ast_iterator<conditions::named_collate<T>, void> {
            using node_type = conditions::named_collate<T>;

            template<class L>
            void operator()(const node_type &col, const L &l) const {
                iterate_ast(col.expr, l);
            }
        };

        template<class C>
        struct ast_iterator<conditions::negated_condition_t<C>, void> {
            using node_type = conditions::negated_condition_t<C>;

            template<class L>
            void operator()(const node_type &neg, const L &l) const {
                iterate_ast(neg.c, l);
            }
        };

        template<class T>
        struct ast_iterator<conditions::is_null_t<T>, void> {
            using node_type = conditions::is_null_t<T>;

            template<class L>
            void operator()(const node_type &i, const L &l) const {
                iterate_ast(i.t, l);
            }
        };

        template<class T>
        struct ast_iterator<conditions::is_not_null_t<T>, void> {
            using node_type = conditions::is_not_null_t<T>;

            template<class L>
            void operator()(const node_type &i, const L &l) const {
                iterate_ast(i.t, l);
            }
        };

        template<class R, class S, class... Args>
        struct ast_iterator<core_functions::core_function_t<R, S, Args...>, void> {
            using node_type = core_functions::core_function_t<R, S, Args...>;

            template<class L>
            void operator()(const node_type &f, const L &l) const {
                iterate_ast(f.args, l);
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::left_join_t<T, O>, void> {
            using node_type = conditions::left_join_t<T, O>;

            template<class L>
            void operator()(const node_type &j, const L &l) const {
                iterate_ast(j.constraint, l);
            }
        };

        template<class T>
        struct ast_iterator<conditions::on_t<T>, void> {
            using node_type = conditions::on_t<T>;

            template<class L>
            void operator()(const node_type &o, const L &l) const {
                iterate_ast(o.arg, l);
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::join_t<T, O>, void> {
            using node_type = conditions::join_t<T, O>;

            template<class L>
            void operator()(const node_type &j, const L &l) const {
                iterate_ast(j.constraint, l);
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::left_outer_join_t<T, O>, void> {
            using node_type = conditions::left_outer_join_t<T, O>;

            template<class L>
            void operator()(const node_type &j, const L &l) const {
                iterate_ast(j.constraint, l);
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::inner_join_t<T, O>, void> {
            using node_type = conditions::inner_join_t<T, O>;

            template<class L>
            void operator()(const node_type &j, const L &l) const {
                iterate_ast(j.constraint, l);
            }
        };

        template<class R, class T, class E, class... Args>
        struct ast_iterator<simple_case_t<R, T, E, Args...>, void> {
            using node_type = simple_case_t<R, T, E, Args...>;

            template<class L>
            void operator()(const node_type &c, const L &l) const {
                c.case_expression.apply([&l](auto &c) {
                    iterate_ast(c, l);
                });
                iterate_tuple(c.args, [&l](auto &pair) {
                    iterate_ast(pair.first, l);
                    iterate_ast(pair.second, l);
                });
                c.else_expression.apply([&l](auto &el) {
                    iterate_ast(el, l);
                });
            }
        };

        template<class T, class E>
        struct ast_iterator<as_t<T, E>, void> {
            using node_type = as_t<T, E>;

            template<class L>
            void operator()(const node_type &a, const L &l) const {
                iterate_ast(a.expression, l);
            }
        };

        template<class T, bool OI>
        struct ast_iterator<conditions::limit_t<T, false, OI, void>, void> {
            using node_type = conditions::limit_t<T, false, OI, void>;

            template<class L>
            void operator()(const node_type &a, const L &l) const {
                iterate_ast(a.lim, l);
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::limit_t<T, true, false, O>, void> {
            using node_type = conditions::limit_t<T, true, false, O>;

            template<class L>
            void operator()(const node_type &a, const L &l) const {
                iterate_ast(a.lim, l);
                a.off.apply([&l](auto &value) {
                    iterate_ast(value, l);
                });
            }
        };

        template<class T, class O>
        struct ast_iterator<conditions::limit_t<T, true, true, O>, void> {
            using node_type = conditions::limit_t<T, true, true, O>;

            template<class L>
            void operator()(const node_type &a, const L &l) const {
                a.off.apply([&l](auto &value) {
                    iterate_ast(value, l);
                });
                iterate_ast(a.lim, l);
            }
        };
    }
}

// #include "prepared_statement.h"

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class S, class... Args>
        struct view_t {
            using mapped_type = T;
            using storage_type = S;
            using self = view_t<T, S, Args...>;

            storage_type &storage;
            connection_ref connection;
            get_all_t<T, Args...> args;

            view_t(storage_type &stor, decltype(connection) conn, Args &&... args_) :
                storage(stor), connection(std::move(conn)), args{std::make_tuple(std::forward<Args>(args_)...)} {}

            size_t size() {
                return this->storage.template count<T>();
            }

            bool empty() {
                return !this->size();
            }

            iterator_t<self> end() {
                return {nullptr, *this};
            }

            iterator_t<self> begin() {
                sqlite3_stmt *stmt = nullptr;
                auto db = this->connection.get();
                auto query = this->storage.string_from_expression(this->args, false);
                auto ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
                if(ret == SQLITE_OK) {
                    auto index = 1;
                    iterate_ast(this->args.conditions, [&index, stmt, db](auto &node) {
                        using node_type = typename std::decay<decltype(node)>::type;
                        conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                        if(SQLITE_OK != binder(node)) {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    });
                    return {stmt, *this};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }
        };
    }
}

// #include "ast_iterator.h"

// #include "storage_base.h"

#include <functional>  //  std::function, std::bind
#include <sqlite3.h>
#include <string>  //  std::string
#include <sstream>  //  std::stringstream
#include <utility>  //  std::move
#include <system_error>  //  std::system_error, std::error_code, std::make_error_code
#include <vector>  //  std::vector
#include <memory>  //  std::make_shared, std::shared_ptr
#include <map>  //  std::map
#include <type_traits>  //  std::decay, std::is_same
#include <algorithm>  //  std::iter_swap

// #include "pragma.h"

#include <string>  //  std::string
#include <sqlite3.h>
#include <functional>  //  std::function
#include <memory>  // std::shared_ptr

// #include "error_code.h"

// #include "row_extractor.h"

// #include "journal_mode.h"

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {
        struct storage_base;
    }

    struct pragma_t {
        using get_connection_t = std::function<internal::connection_ref()>;

        pragma_t(get_connection_t get_connection_) : get_connection(std::move(get_connection_)) {}

        sqlite_orm::journal_mode journal_mode() {
            return this->get_pragma<sqlite_orm::journal_mode>("journal_mode");
        }

        void journal_mode(sqlite_orm::journal_mode value) {
            this->_journal_mode = -1;
            this->set_pragma("journal_mode", value);
            this->_journal_mode = static_cast<decltype(this->_journal_mode)>(value);
        }

        int synchronous() {
            return this->get_pragma<int>("synchronous");
        }

        void synchronous(int value) {
            this->_synchronous = -1;
            this->set_pragma("synchronous", value);
            this->_synchronous = value;
        }

        int user_version() {
            return this->get_pragma<int>("user_version");
        }

        void user_version(int value) {
            this->set_pragma("user_version", value);
        }

        int auto_vacuum() {
            return this->get_pragma<int>("auto_vacuum");
        }

        void auto_vacuum(int value) {
            this->set_pragma("auto_vacuum", value);
        }

      protected:
        friend struct storage_base;

      public:
        int _synchronous = -1;
        signed char _journal_mode = -1;  //  if != -1 stores static_cast<sqlite_orm::journal_mode>(journal_mode)
        get_connection_t get_connection;

        template<class T>
        T get_pragma(const std::string &name) {
            auto connection = this->get_connection();
            auto query = "PRAGMA " + name;
            T res;
            auto db = connection.get();
            auto rc = sqlite3_exec(
                db,
                query.c_str(),
                [](void *data, int argc, char **argv, char **) -> int {
                    auto &res = *(T *)data;
                    if(argc) {
                        res = row_extractor<T>().extract(argv[0]);
                    }
                    return 0;
                },
                &res,
                nullptr);
            if(rc == SQLITE_OK) {
                return res;
            } else {
                throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
            }
        }

        /**
         *  Yevgeniy Zakharov: I wanted to refactore this function with statements and value bindings
         *  but it turns out that bindings in pragma statements are not supported.
         */
        template<class T>
        void set_pragma(const std::string &name, const T &value, sqlite3 *db = nullptr) {
            auto con = this->get_connection();
            if(!db) {
                db = con.get();
            }
            std::stringstream ss;
            ss << "PRAGMA " << name << " = " << value;
            auto query = ss.str();
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
            if(rc != SQLITE_OK) {
                throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
            }
        }

        void set_pragma(const std::string &name, const sqlite_orm::journal_mode &value, sqlite3 *db = nullptr) {
            auto con = this->get_connection();
            if(!db) {
                db = con.get();
            }
            std::stringstream ss;
            ss << "PRAGMA " << name << " = " << internal::to_string(value);
            auto query = ss.str();
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
            if(rc != SQLITE_OK) {
                throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
            }
        }
    };
}

// #include "limit_accesor.h"

#include <sqlite3.h>
#include <map>  //  std::map
#include <functional>  //  std::function
#include <memory>  //  std::shared_ptr

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        struct limit_accesor {
            using get_connection_t = std::function<connection_ref()>;

            limit_accesor(get_connection_t get_connection_) : get_connection(std::move(get_connection_)) {}

            int length() {
                return this->get(SQLITE_LIMIT_LENGTH);
            }

            void length(int newValue) {
                this->set(SQLITE_LIMIT_LENGTH, newValue);
            }

            int sql_length() {
                return this->get(SQLITE_LIMIT_SQL_LENGTH);
            }

            void sql_length(int newValue) {
                this->set(SQLITE_LIMIT_SQL_LENGTH, newValue);
            }

            int column() {
                return this->get(SQLITE_LIMIT_COLUMN);
            }

            void column(int newValue) {
                this->set(SQLITE_LIMIT_COLUMN, newValue);
            }

            int expr_depth() {
                return this->get(SQLITE_LIMIT_EXPR_DEPTH);
            }

            void expr_depth(int newValue) {
                this->set(SQLITE_LIMIT_EXPR_DEPTH, newValue);
            }

            int compound_select() {
                return this->get(SQLITE_LIMIT_COMPOUND_SELECT);
            }

            void compound_select(int newValue) {
                this->set(SQLITE_LIMIT_COMPOUND_SELECT, newValue);
            }

            int vdbe_op() {
                return this->get(SQLITE_LIMIT_VDBE_OP);
            }

            void vdbe_op(int newValue) {
                this->set(SQLITE_LIMIT_VDBE_OP, newValue);
            }

            int function_arg() {
                return this->get(SQLITE_LIMIT_FUNCTION_ARG);
            }

            void function_arg(int newValue) {
                this->set(SQLITE_LIMIT_FUNCTION_ARG, newValue);
            }

            int attached() {
                return this->get(SQLITE_LIMIT_ATTACHED);
            }

            void attached(int newValue) {
                this->set(SQLITE_LIMIT_ATTACHED, newValue);
            }

            int like_pattern_length() {
                return this->get(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
            }

            void like_pattern_length(int newValue) {
                this->set(SQLITE_LIMIT_LIKE_PATTERN_LENGTH, newValue);
            }

            int variable_number() {
                return this->get(SQLITE_LIMIT_VARIABLE_NUMBER);
            }

            void variable_number(int newValue) {
                this->set(SQLITE_LIMIT_VARIABLE_NUMBER, newValue);
            }

            int trigger_depth() {
                return this->get(SQLITE_LIMIT_TRIGGER_DEPTH);
            }

            void trigger_depth(int newValue) {
                this->set(SQLITE_LIMIT_TRIGGER_DEPTH, newValue);
            }

#if SQLITE_VERSION_NUMBER >= 3008007
            int worker_threads() {
                return this->get(SQLITE_LIMIT_WORKER_THREADS);
            }

            void worker_threads(int newValue) {
                this->set(SQLITE_LIMIT_WORKER_THREADS, newValue);
            }
#endif

          protected:
            get_connection_t get_connection;

            friend struct storage_base;

            /**
             *  Stores limit set between connections.
             */
            std::map<int, int> limits;

            int get(int id) {
                auto connection = this->get_connection();
                return sqlite3_limit(connection.get(), id, -1);
            }

            void set(int id, int newValue) {
                this->limits[id] = newValue;
                auto connection = this->get_connection();
                sqlite3_limit(connection.get(), id, newValue);
            }
        };
    }
}

// #include "transaction_guard.h"

#include <functional>  //  std::function

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  Class used as a guard for a transaction. Calls `ROLLBACK` in destructor.
         *  Has explicit `commit()` and `rollback()` functions. After explicit function is fired
         *  guard won't do anything in d-tor. Also you can set `commit_on_destroy` to true to
         *  make it call `COMMIT` on destroy.
         */
        struct transaction_guard_t {
            /**
             *  This is a public lever to tell a guard what it must do in its destructor
             *  if `gotta_fire` is true
             */
            bool commit_on_destroy = false;

            transaction_guard_t(connection_ref connection_,
                                std::function<void()> commit_func_,
                                std::function<void()> rollback_func_) :
                connection(std::move(connection_)),
                commit_func(std::move(commit_func_)), rollback_func(std::move(rollback_func_)) {}

            ~transaction_guard_t() {
                if(this->gotta_fire) {
                    if(!this->commit_on_destroy) {
                        this->rollback_func();
                    } else {
                        this->commit_func();
                    }
                }
            }

            /**
             *  Call `COMMIT` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
            void commit() {
                this->commit_func();
                this->gotta_fire = false;
            }

            /**
             *  Call `ROLLBACK` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
            void rollback() {
                this->rollback_func();
                this->gotta_fire = false;
            }

          protected:
            connection_ref connection;
            std::function<void()> commit_func;
            std::function<void()> rollback_func;
            bool gotta_fire = true;
        };
    }
}

// #include "statement_finalizer.h"

// #include "type_printer.h"

// #include "tuple_helper.h"

// #include "row_extractor.h"

// #include "connection_holder.h"

// #include "backup.h"

#include <sqlite3.h>
#include <string>  //  std::string
#include <memory>

// #include "error_code.h"

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  A backup class. Don't construct it as is, call storage.make_backup_from or storage.make_backup_to instead.
         *  An instance of this class represents a wrapper around sqlite3_backup pointer. Use this class
         *  to have maximum control on a backup operation. In case you need a single backup in one line you
         *  can skip creating a backup_t instance and just call storage.backup_from or storage.backup_to function.
         */
        struct backup_t {
            backup_t(connection_ref to_,
                     const std::string &zDestName,
                     connection_ref from_,
                     const std::string &zSourceName,
                     std::unique_ptr<connection_holder> holder_) :
                handle(sqlite3_backup_init(to_.get(), zDestName.c_str(), from_.get(), zSourceName.c_str())),
                holder(move(holder_)), to(to_), from(from_) {
                if(!this->handle) {
                    throw std::system_error(std::make_error_code(orm_error_code::failed_to_init_a_backup));
                }
            }

            backup_t(backup_t &&other) :
                handle(other.handle), holder(move(other.holder)), to(other.to), from(other.from) {
                other.handle = nullptr;
            }

            ~backup_t() {
                if(this->handle) {
                    (void)sqlite3_backup_finish(this->handle);
                    this->handle = nullptr;
                }
            }

            /**
             *  Calls sqlite3_backup_step with pages argument
             */
            int step(int pages) {
                return sqlite3_backup_step(this->handle, pages);
            }

            /**
             *  Returns sqlite3_backup_remaining result
             */
            int remaining() const {
                return sqlite3_backup_remaining(this->handle);
            }

            /**
             *  Returns sqlite3_backup_pagecount result
             */
            int pagecount() const {
                return sqlite3_backup_pagecount(this->handle);
            }

          protected:
            sqlite3_backup *handle = nullptr;
            std::unique_ptr<connection_holder> holder;
            connection_ref to;
            connection_ref from;
        };
    }
}

namespace sqlite_orm {

    namespace internal {

        struct storage_base {
            using collating_function = std::function<int(int, const void *, int, const void *)>;

            std::function<void(sqlite3 *)> on_open;
            pragma_t pragma;
            limit_accesor limit;

            transaction_guard_t transaction_guard() {
                this->begin_transaction();
                auto commitFunc = std::bind(static_cast<void (storage_base::*)()>(&storage_base::commit), this);
                auto rollbackFunc = std::bind(static_cast<void (storage_base::*)()>(&storage_base::rollback), this);
                return {this->get_connection(), move(commitFunc), move(rollbackFunc)};
            }

            void drop_index(const std::string &indexName) {
                auto con = this->get_connection();
                auto db = con.get();
                std::stringstream ss;
                ss << "DROP INDEX '" << indexName + "'";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            void vacuum() {
                auto con = this->get_connection();
                auto db = con.get();
                std::string query = "VACUUM";
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            /**
             *  Drops table with given name.
             */
            void drop_table(const std::string &tableName) {
                auto con = this->get_connection();
                this->drop_table_internal(tableName, con.get());
            }

            /**
             *  sqlite3_changes function.
             */
            int changes() {
                auto con = this->get_connection();
                return sqlite3_changes(con.get());
            }

            /**
             *  sqlite3_total_changes function.
             */
            int total_changes() {
                auto con = this->get_connection();
                return sqlite3_total_changes(con.get());
            }

            int64 last_insert_rowid() {
                auto con = this->get_connection();
                return sqlite3_last_insert_rowid(con.get());
            }

            int busy_timeout(int ms) {
                auto con = this->get_connection();
                return sqlite3_busy_timeout(con.get(), ms);
            }

            /**
             *  Returns libsqltie3 lib version, not sqlite_orm
             */
            std::string libversion() {
                return sqlite3_libversion();
            }

            bool transaction(std::function<bool()> f) {
                this->begin_transaction();
                auto con = this->get_connection();
                auto db = con.get();
                auto shouldCommit = f();
                if(shouldCommit) {
                    this->commit(db);
                } else {
                    this->rollback(db);
                }
                return shouldCommit;
            }

            std::string current_timestamp() {
                auto con = this->get_connection();
                return this->current_timestamp(con.get());
            }

#if SQLITE_VERSION_NUMBER >= 3007010
            /**
             * \fn db_release_memory
             * \brief Releases freeable memory of database. It is function can/should be called periodically by
             * application, if application has less memory usage constraint. \note sqlite3_db_release_memory added
             * in 3.7.10 https://sqlite.org/changes.html
             */
            int db_release_memory() {
                auto con = this->get_connection();
                return sqlite3_db_release_memory(con.get());
            }
#endif

            /**
             *  Returns existing permanent table names in database. Doesn't check storage itself - works only with
             * actual database.
             *  @return Returns list of tables in database.
             */
            std::vector<std::string> table_names() {
                auto con = this->get_connection();
                std::vector<std::string> tableNames;
                std::string sql = "SELECT name FROM sqlite_master WHERE type='table'";
                using data_t = std::vector<std::string>;
                auto db = con.get();
                int res = sqlite3_exec(
                    db,
                    sql.c_str(),
                    [](void *data, int argc, char **argv, char * * /*columnName*/) -> int {
                        auto &tableNames = *(data_t *)data;
                        for(int i = 0; i < argc; i++) {
                            if(argv[i]) {
                                tableNames.push_back(argv[i]);
                            }
                        }
                        return 0;
                    },
                    &tableNames,
                    nullptr);

                if(res != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return tableNames;
            }

            void open_forever() {
                this->isOpenedForever = true;
                this->connection->retain();
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
            }

            void create_collation(const std::string &name, collating_function f) {
                collating_function *functionPointer = nullptr;
                if(f) {
                    functionPointer = &(collatingFunctions[name] = std::move(f));
                } else {
                    collatingFunctions.erase(name);
                }

                //  create collations if db is open
                if(this->connection->retain_count() > 0) {
                    auto db = this->connection->get();
                    if(sqlite3_create_collation(db,
                                                name.c_str(),
                                                SQLITE_UTF8,
                                                functionPointer,
                                                f ? collate_callback : nullptr) != SQLITE_OK) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                }
            }

            void begin_transaction() {
                this->connection->retain();
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
                auto db = this->connection->get();
                this->begin_transaction(db);
            }

            void commit() {
                auto db = this->connection->get();
                this->commit(db);
                this->connection->release();
                if(this->connection->retain_count() < 0) {
                    throw std::system_error(std::make_error_code(orm_error_code::no_active_transaction));
                }
            }

            void rollback() {
                auto db = this->connection->get();
                this->rollback(db);
                this->connection->release();
                if(this->connection->retain_count() < 0) {
                    throw std::system_error(std::make_error_code(orm_error_code::no_active_transaction));
                }
            }

            void backup_to(const std::string &filename) {
                auto backup = this->make_backup_to(filename);
                backup.step(-1);
            }

            void backup_to(storage_base &other) {
                auto backup = this->make_backup_to(other);
                backup.step(-1);
            }

            void backup_from(const std::string &filename) {
                auto backup = this->make_backup_from(filename);
                backup.step(-1);
            }

            void backup_from(storage_base &other) {
                auto backup = this->make_backup_from(other);
                backup.step(-1);
            }

            backup_t make_backup_to(const std::string &filename) {
                auto holder = std::make_unique<connection_holder>(filename);
                return {connection_ref{*holder}, "main", this->get_connection(), "main", move(holder)};
            }

            backup_t make_backup_to(storage_base &other) {
                return {other.get_connection(), "main", this->get_connection(), "main", {}};
            }

            backup_t make_backup_from(const std::string &filename) {
                auto holder = std::make_unique<connection_holder>(filename);
                return {this->get_connection(), "main", connection_ref{*holder}, "main", move(holder)};
            }

            backup_t make_backup_from(storage_base &other) {
                return {this->get_connection(), "main", other.get_connection(), "main", {}};
            }

            const std::string &filename() const {
                return this->connection->filename;
            }

          protected:
            storage_base(const std::string &filename_, int foreignKeysCount) :
                pragma(std::bind(&storage_base::get_connection, this)),
                limit(std::bind(&storage_base::get_connection, this)),
                inMemory(filename_.empty() || filename_ == ":memory:"),
                connection(std::make_unique<connection_holder>(filename_)), cachedForeignKeysCount(foreignKeysCount) {
                if(this->inMemory) {
                    this->connection->retain();
                    this->on_open_internal(this->connection->get());
                }
            }

            storage_base(const storage_base &other) :
                on_open(other.on_open), pragma(std::bind(&storage_base::get_connection, this)),
                limit(std::bind(&storage_base::get_connection, this)), inMemory(other.inMemory),
                connection(std::make_unique<connection_holder>(other.connection->filename)),
                cachedForeignKeysCount(other.cachedForeignKeysCount) {
                if(this->inMemory) {
                    this->connection->retain();
                    this->on_open_internal(this->connection->get());
                }
            }

            ~storage_base() {
                if(this->isOpenedForever) {
                    this->connection->release();
                }
                if(this->inMemory) {
                    this->connection->release();
                }
            }

            const bool inMemory;
            bool isOpenedForever = false;
            std::unique_ptr<connection_holder> connection;
            std::map<std::string, collating_function> collatingFunctions;
            const int cachedForeignKeysCount;

            connection_ref get_connection() {
                connection_ref res{*this->connection};
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
                return res;
            }

#if SQLITE_VERSION_NUMBER >= 3006019

            void foreign_keys(sqlite3 *db, bool value) {
                std::stringstream ss;
                ss << "PRAGMA foreign_keys = " << value;
                auto query = ss.str();
                auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            bool foreign_keys(sqlite3 *db) {
                std::string query = "PRAGMA foreign_keys";
                auto res = false;
                auto rc = sqlite3_exec(
                    db,
                    query.c_str(),
                    [](void *data, int argc, char **argv, char **) -> int {
                        auto &res = *(bool *)data;
                        if(argc) {
                            res = row_extractor<bool>().extract(argv[0]);
                        }
                        return 0;
                    },
                    &res,
                    nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }

#endif
            void on_open_internal(sqlite3 *db) {

#if SQLITE_VERSION_NUMBER >= 3006019
                if(this->cachedForeignKeysCount) {
                    this->foreign_keys(db, true);
                }
#endif
                if(this->pragma._synchronous != -1) {
                    this->pragma.synchronous(this->pragma._synchronous);
                }

                if(this->pragma._journal_mode != -1) {
                    this->pragma.set_pragma("journal_mode", static_cast<journal_mode>(this->pragma._journal_mode), db);
                }

                for(auto &p: this->collatingFunctions) {
                    if(sqlite3_create_collation(db, p.first.c_str(), SQLITE_UTF8, &p.second, collate_callback) !=
                       SQLITE_OK) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                }

                for(auto &p: this->limit.limits) {
                    sqlite3_limit(db, p.first, p.second);
                }

                if(this->on_open) {
                    this->on_open(db);
                }
            }

            void begin_transaction(sqlite3 *db) {
                std::stringstream ss;
                ss << "BEGIN TRANSACTION";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            void commit(sqlite3 *db) {
                std::stringstream ss;
                ss << "COMMIT";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            void rollback(sqlite3 *db) {
                std::stringstream ss;
                ss << "ROLLBACK";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            std::string current_timestamp(sqlite3 *db) {
                std::string res;
                std::stringstream ss;
                ss << "SELECT CURRENT_TIMESTAMP";
                auto query = ss.str();
                auto rc = sqlite3_exec(
                    db,
                    query.c_str(),
                    [](void *data, int argc, char **argv, char **) -> int {
                        auto &res = *(std::string *)data;
                        if(argc) {
                            if(argv[0]) {
                                res = row_extractor<std::string>().extract(argv[0]);
                            }
                        }
                        return 0;
                    },
                    &res,
                    nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }

            void drop_table_internal(const std::string &tableName, sqlite3 *db) {
                std::stringstream ss;
                ss << "DROP TABLE '" << tableName + "'";
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class S>
            std::string process_order_by(const conditions::dynamic_order_by_t<S> &orderBy) const {
                std::vector<std::string> expressions;
                for(auto &entry: orderBy) {
                    std::string entryString;
                    {
                        std::stringstream ss;
                        ss << entry.name << " ";
                        if(!entry._collate_argument.empty()) {
                            ss << "COLLATE " << entry._collate_argument << " ";
                        }
                        switch(entry.asc_desc) {
                            case 1:
                                ss << "ASC";
                                break;
                            case -1:
                                ss << "DESC";
                                break;
                        }
                        entryString = ss.str();
                    }
                    expressions.push_back(move(entryString));
                };
                std::stringstream ss;
                ss << static_cast<std::string>(orderBy) << " ";
                for(size_t i = 0; i < expressions.size(); ++i) {
                    ss << expressions[i];
                    if(i < expressions.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << " ";
                return ss.str();
            }

            static int collate_callback(void *arg, int leftLen, const void *lhs, int rightLen, const void *rhs) {
                auto &f = *(collating_function *)arg;
                return f(leftLen, lhs, rightLen, rhs);
            }

            //  returns foreign keys count in storage definition
            template<class T>
            static int foreign_keys_count(T &storageImpl) {
                auto res = 0;
                storageImpl.for_each([&res](auto impl) {
                    res += impl->foreign_keys_count();
                });
                return res;
            }
        };
    }
}

// #include "prepared_statement.h"

// #include "expression_object_type.h"

#include <type_traits>  //  std::decay
#include <functional>  //  std::reference_wrapper

// #include "prepared_statement.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct expression_object_type;

        template<class T>
        struct expression_object_type<update_t<T>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct expression_object_type<update_t<std::reference_wrapper<T>>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct expression_object_type<replace_t<T>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct expression_object_type<replace_t<std::reference_wrapper<T>>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct expression_object_type<insert_t<T>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct expression_object_type<insert_t<std::reference_wrapper<T>>> {
            using type = typename std::decay<T>::type;
        };

        template<class T, class... Cols>
        struct expression_object_type<insert_explicit<T, Cols...>> {
            using type = typename std::decay<T>::type;
        };

        template<class T, class... Cols>
        struct expression_object_type<insert_explicit<std::reference_wrapper<T>, Cols...>> {
            using type = typename std::decay<T>::type;
        };

        template<class T>
        struct get_ref_t {

            template<class O>
            auto &operator()(O &t) const {
                return t;
            }
        };

        template<class T>
        struct get_ref_t<std::reference_wrapper<T>> {

            template<class O>
            auto &operator()(O &t) const {
                return t.get();
            }
        };

        template<class T>
        auto &get_ref(T &t) {
            using arg_type = typename std::decay<T>::type;
            get_ref_t<arg_type> g;
            return g(t);
        }

        template<class T>
        struct get_object_t;

        template<class T>
        struct get_object_t<const T> : get_object_t<T> {};

        template<class T>
        auto &get_object(T &t) {
            using expression_type = typename std::decay<T>::type;
            get_object_t<expression_type> obj;
            return obj(t);
        }

        template<class T>
        struct get_object_t<replace_t<T>> {
            using expression_type = replace_t<T>;

            template<class O>
            auto &operator()(O &e) const {
                return get_ref(e.obj);
            }
        };

        template<class T>
        struct get_object_t<insert_t<T>> {
            using expression_type = insert_t<T>;

            template<class O>
            auto &operator()(O &e) const {
                return get_ref(e.obj);
            }
        };

        template<class T>
        struct get_object_t<update_t<T>> {
            using expression_type = update_t<T>;

            template<class O>
            auto &operator()(O &e) const {
                return get_ref(e.obj);
            }
        };
    }
}

namespace sqlite_orm {

    namespace conditions {

        template<class S>
        struct dynamic_order_by_t;
    }

    namespace internal {

        /**
         *  Storage class itself. Create an instanse to use it as an interfacto to sqlite db by calling `make_storage`
         * function.
         */
        template<class... Ts>
        struct storage_t : storage_base {
            using self = storage_t<Ts...>;
            using impl_type = storage_impl<Ts...>;

            /**
             *  @param filename database filename.
             *  @param impl_ storage_impl head
             */
            storage_t(const std::string &filename, impl_type impl_) :
                storage_base{filename, foreign_keys_count(impl_)}, impl(std::move(impl_)) {}

            storage_t(const storage_t &other) : storage_base(other), impl(other.impl) {}

          protected:
            impl_type impl;

            template<class T, class S, class... Args>
            friend struct view_t;

            template<class S>
            friend struct conditions::dynamic_order_by_t;

            template<class V>
            friend struct iterator_t;

            template<class O, class T, class G, class S, class... Op>
            std::string serialize_column_schema(const internal::column_t<O, T, G, S, Op...> &c) {
                std::stringstream ss;
                ss << "'" << c.name << "' ";
                using column_type = typename std::decay<decltype(c)>::type;
                using field_type = typename column_type::field_type;
                using constraints_type = typename column_type::constraints_type;
                ss << type_printer<field_type>().print() << " ";
                {
                    std::vector<std::string> constraintsStrings;
                    constexpr const size_t constraintsCount = std::tuple_size<constraints_type>::value;
                    constraintsStrings.reserve(constraintsCount);
                    int primaryKeyIndex = -1;
                    int autoincrementIndex = -1;
                    int tupleIndex = 0;
                    iterate_tuple(c.constraints,
                                  [&constraintsStrings, &primaryKeyIndex, &autoincrementIndex, &tupleIndex](auto &v) {
                                      using constraint_type = typename std::decay<decltype(v)>::type;
                                      constraintsStrings.push_back(serialize(v));
                                      if(is_primary_key<constraint_type>::value) {
                                          primaryKeyIndex = tupleIndex;
                                      } else if(std::is_same<constraints::autoincrement_t, constraint_type>::value) {
                                          autoincrementIndex = tupleIndex;
                                      }
                                      ++tupleIndex;
                                  });
                    if(primaryKeyIndex != -1 && autoincrementIndex != -1 && autoincrementIndex < primaryKeyIndex) {
                        iter_swap(constraintsStrings.begin() + primaryKeyIndex,
                                  constraintsStrings.begin() + autoincrementIndex);
                    }
                    for(auto &str: constraintsStrings) {
                        ss << str << ' ';
                    }
                }
                if(c.not_null()) {
                    ss << "NOT NULL ";
                }
                return ss.str();
            }

            template<class... Cs>
            std::string serialize_column_schema(const constraints::primary_key_t<Cs...> &fk) {
                std::stringstream ss;
                ss << static_cast<std::string>(fk) << " (";
                std::vector<std::string> columnNames;
                columnNames.reserve(std::tuple_size<decltype(fk.columns)>::value);
                iterate_tuple(fk.columns, [&columnNames, this](auto &c) {
                    columnNames.push_back(this->impl.column_name(c));
                });
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << columnNames[i];
                    if(i < columnNames.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << ") ";
                return ss.str();
            }

#if SQLITE_VERSION_NUMBER >= 3006019

            template<class... Cs, class... Rs>
            std::string
            serialize_column_schema(const constraints::foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> &fk) {
                std::stringstream ss;
                std::vector<std::string> columnNames;
                using columns_type_t = typename std::decay<decltype(fk)>::type::columns_type;
                constexpr const size_t columnsCount = std::tuple_size<columns_type_t>::value;
                columnNames.reserve(columnsCount);
                iterate_tuple(fk.columns, [&columnNames, this](auto &v) {
                    columnNames.push_back(this->impl.column_name(v));
                });
                ss << "FOREIGN KEY( ";
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << columnNames[i];
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << ") REFERENCES ";
                std::vector<std::string> referencesNames;
                using references_type_t = typename std::decay<decltype(fk)>::type::references_type;
                constexpr const size_t referencesCount = std::tuple_size<references_type_t>::value;
                referencesNames.reserve(referencesCount);
                {
                    using first_reference_t = typename std::tuple_element<0, references_type_t>::type;
                    using first_reference_mapped_type = typename internal::table_type<first_reference_t>::type;
                    auto refTableName = this->impl.template find_table_name<first_reference_mapped_type>();
                    ss << refTableName << " ";
                }
                iterate_tuple(fk.references, [&referencesNames, this](auto &v) {
                    referencesNames.push_back(this->impl.column_name(v));
                });
                ss << "( ";
                for(size_t i = 0; i < referencesNames.size(); ++i) {
                    ss << referencesNames[i];
                    if(i < referencesNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << ") ";
                if(fk.on_update) {
                    ss << static_cast<std::string>(fk.on_update) << " " << fk.on_update._action << " ";
                }
                if(fk.on_delete) {
                    ss << static_cast<std::string>(fk.on_delete) << " " << fk.on_delete._action << " ";
                }
                return ss.str();
            }
#endif

            template<class I>
            void create_table(sqlite3 *db, const std::string &tableName, I *impl) {
                std::stringstream ss;
                ss << "CREATE TABLE '" << tableName << "' ( ";
                auto columnsCount = impl->table.columns_count;
                auto index = 0;
                impl->table.for_each_column_with_constraints([columnsCount, &index, &ss, this](auto &c) {
                    ss << this->serialize_column_schema(c);
                    if(index < columnsCount - 1) {
                        ss << ", ";
                    }
                    index++;
                });
                ss << ") ";
                if(impl->table._without_rowid) {
                    ss << "WITHOUT ROWID ";
                }
                auto query = ss.str();
                sqlite3_stmt *stmt;
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    statement_finalizer finalizer{stmt};
                    if(sqlite3_step(stmt) == SQLITE_DONE) {
                        //  done..
                    } else {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class I>
            void backup_table(sqlite3 *db, I *impl) {

                //  here we copy source table to another with a name with '_backup' suffix, but in case table with such
                //  a name already exists we append suffix 1, then 2, etc until we find a free name..
                auto backupTableName = impl->table.name + "_backup";
                if(impl->table_exists(backupTableName, db)) {
                    int suffix = 1;
                    do {
                        std::stringstream stream;
                        stream << suffix;
                        auto anotherBackupTableName = backupTableName + stream.str();
                        if(!impl->table_exists(anotherBackupTableName, db)) {
                            backupTableName = anotherBackupTableName;
                            break;
                        }
                        ++suffix;
                    } while(true);
                }

                this->create_table(db, backupTableName, impl);

                impl->copy_table(db, backupTableName);

                this->drop_table_internal(impl->table.name, db);

                impl->rename_table(db, backupTableName, impl->table.name);
            }

            template<class O>
            void assert_mapped_type() const {
                using mapped_types_tuples = std::tuple<typename Ts::object_type...>;
                static_assert(tuple_helper::has_type<O, mapped_types_tuples>::value, "type is not mapped to a storage");
            }

            template<class O>
            auto &get_impl() const {
                return this->impl.template get_impl<O>();
            }

            template<class T>
            typename std::enable_if<is_bindable<T>::value, std::string>::type
            string_from_expression(const T &, bool /*noTableName*/) const {
                return "?";
            }

            template<class T>
            std::string string_from_expression(std::reference_wrapper<T> ref, bool noTableName) const {
                return this->string_from_expression(ref.get(), noTableName);
            }

            std::string string_from_expression(std::nullptr_t, bool /*noTableName*/) const {
                return "?";
            }

            template<class T>
            std::string string_from_expression(const alias_holder<T> &, bool /*noTableName*/) const {
                return T::get();
            }

            template<class R, class S, class... Args>
            std::string string_from_expression(const core_functions::core_function_t<R, S, Args...> &c,
                                               bool noTableName) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << "(";
                std::vector<std::string> args;
                using args_type = typename std::decay<decltype(c)>::type::args_type;
                args.reserve(std::tuple_size<args_type>::value);
                iterate_tuple(c.args, [&args, this, noTableName](auto &v) {
                    args.push_back(this->string_from_expression(v, noTableName));
                });
                for(size_t i = 0; i < args.size(); ++i) {
                    ss << args[i];
                    if(i < args.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << ")";
                return ss.str();
            }

            template<class T, class E>
            std::string string_from_expression(const as_t<T, E> &als, bool noTableName) const {
                auto tableAliasString = alias_extractor<T>::get();
                return this->string_from_expression(als.expression, noTableName) + " AS " + tableAliasString;
            }

            template<class T, class C>
            std::string string_from_expression(const alias_column_t<T, C> &als, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << T::get() << "'.";
                }
                ss << this->string_from_expression(als.column, true);
                return ss.str();
            }

            std::string string_from_expression(const std::string &, bool /*noTableName*/) const {
                return "?";
            }

            std::string string_from_expression(const char *, bool /*noTableName*/) const {
                return "?";
            }

            template<class F, class O>
            std::string string_from_expression(F O::*m, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << this->impl.template find_table_name<O>() << "'.";
                }
                ss << "\"" << this->impl.column_name(m) << "\"";
                return ss.str();
            }

            std::string string_from_expression(const rowid_t &rid, bool /*noTableName*/) const {
                return static_cast<std::string>(rid);
            }

            std::string string_from_expression(const oid_t &rid, bool /*noTableName*/) const {
                return static_cast<std::string>(rid);
            }

            std::string string_from_expression(const _rowid_t &rid, bool /*noTableName*/) const {
                return static_cast<std::string>(rid);
            }

            template<class O>
            std::string string_from_expression(const table_rowid_t<O> &rid, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << this->impl.template find_table_name<O>() << "'.";
                }
                ss << static_cast<std::string>(rid);
                return ss.str();
            }

            template<class O>
            std::string string_from_expression(const table_oid_t<O> &rid, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << this->impl.template find_table_name<O>() << "'.";
                }
                ss << static_cast<std::string>(rid);
                return ss.str();
            }

            template<class O>
            std::string string_from_expression(const table__rowid_t<O> &rid, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << this->impl.template find_table_name<O>() << "'.";
                }
                ss << static_cast<std::string>(rid);
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::group_concat_double_t<T> &f,
                                               bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                auto expr2 = this->string_from_expression(f.y, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ", " << expr2 << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::group_concat_single_t<T> &f,
                                               bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class L, class R, class... Ds>
            std::string string_from_expression(const binary_operator<L, R, Ds...> &f, bool noTableName) const {
                std::stringstream ss;
                auto lhs = this->string_from_expression(f.lhs, noTableName);
                auto rhs = this->string_from_expression(f.rhs, noTableName);
                ss << "(" << lhs << " " << static_cast<std::string>(f) << " " << rhs << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::min_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::max_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::total_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::sum_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::count_asterisk_t<T> &,
                                               bool noTableName) const {
                return this->string_from_expression(aggregate_functions::count_asterisk_without_type{}, noTableName);
            }

            std::string string_from_expression(const aggregate_functions::count_asterisk_without_type &f,
                                               bool /*noTableName*/) const {
                std::stringstream ss;
                ss << static_cast<std::string>(f) << "(*)";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::count_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.arg, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const aggregate_functions::avg_t<T> &a, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(a.arg, noTableName);
                ss << static_cast<std::string>(a) << "(" << expr << ") ";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const distinct_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.t, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ") ";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const all_t<T> &f, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(f.t, noTableName);
                ss << static_cast<std::string>(f) << "(" << expr << ") ";
                return ss.str();
            }

            template<class T, class F>
            std::string string_from_expression(const column_pointer<T, F> &c, bool noTableName) const {
                std::stringstream ss;
                if(!noTableName) {
                    ss << "'" << this->impl.template find_table_name<T>() << "'.";
                }
                auto &impl = this->get_impl<T>();
                ss << "\"" << impl.column_name_simple(c.field) << "\"";
                return ss.str();
            }

            template<class T>
            std::vector<std::string> get_column_names(const T &t) const {
                auto columnName = this->string_from_expression(t, false);
                if(columnName.length()) {
                    return {columnName};
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
                }
            }

            template<class T>
            std::vector<std::string> get_column_names(std::reference_wrapper<T> r) const {
                return this->get_column_names(r.get());
            }

            template<class T>
            std::vector<std::string> get_column_names(const internal::asterisk_t<T> &) const {
                std::vector<std::string> res;
                res.push_back("*");
                return res;
            }

            template<class... Args>
            std::vector<std::string> get_column_names(const internal::columns_t<Args...> &cols) const {
                std::vector<std::string> columnNames;
                columnNames.reserve(static_cast<size_t>(cols.count));
                iterate_tuple(cols.columns, [&columnNames, this](auto &m) {
                    auto columnName = this->string_from_expression(m, false);
                    if(columnName.length()) {
                        columnNames.push_back(columnName);
                    } else {
                        throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
                    }
                });
                return columnNames;
            }

            /**
             *  Takes select_t object and returns SELECT query string
             */
            template<class T, class... Args>
            std::string string_from_expression(const internal::select_t<T, Args...> &sel, bool /*noTableName*/) const {
                std::stringstream ss;
                if(!is_base_of_template<T, compound_operator>::value) {
                    if(!sel.highest_level) {
                        ss << "( ";
                    }
                    ss << "SELECT ";
                }
                if(get_distinct(sel.col)) {
                    ss << static_cast<std::string>(distinct(0)) << " ";
                }
                auto columnNames = this->get_column_names(sel.col);
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << columnNames[i];
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                auto tableNamesSet = this->parse_table_name(sel.col);
                internal::join_iterator<Args...>()([&tableNamesSet, this](const auto &c) {
                    using original_join_type = typename std::decay<decltype(c)>::type::join_type::type;
                    using cross_join_type = typename internal::mapped_type_proxy<original_join_type>::type;
                    auto crossJoinedTableName = this->impl.template find_table_name<cross_join_type>();
                    auto tableAliasString = alias_extractor<original_join_type>::get();
                    std::pair<std::string, std::string> tableNameWithAlias(std::move(crossJoinedTableName),
                                                                           std::move(tableAliasString));
                    tableNamesSet.erase(tableNameWithAlias);
                });
                if(!tableNamesSet.empty()) {
                    ss << "FROM ";
                    std::vector<std::pair<std::string, std::string>> tableNames(tableNamesSet.begin(),
                                                                                tableNamesSet.end());
                    for(size_t i = 0; i < tableNames.size(); ++i) {
                        auto &tableNamePair = tableNames[i];
                        ss << "'" << tableNamePair.first << "' ";
                        if(!tableNamePair.second.empty()) {
                            ss << tableNamePair.second << " ";
                        }
                        if(int(i) < int(tableNames.size()) - 1) {
                            ss << ",";
                        }
                        ss << " ";
                    }
                }
                iterate_tuple(sel.conditions, [&ss, this](auto &v) {
                    this->process_single_condition(ss, v);
                });
                if(!is_base_of_template<T, compound_operator>::value) {
                    if(!sel.highest_level) {
                        ss << ") ";
                    }
                }
                return ss.str();
            }

            // Common code for statements returning the whole content of a table: get_all_t, get_all_pointer_t,
            // get_all_optional_t.
            template<class T, class... Args>
            std::stringstream string_from_expression_impl_get_all(bool /*noTableName*/) const {
                std::stringstream ss;
                ss << "SELECT ";
                auto &impl = this->get_impl<T>();
                auto columnNames = impl.table.column_names();
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << "\"" << impl.table.name << "\"."
                       << "\"" << columnNames[i] << "\"";
                    if(i < columnNames.size() - 1) {
                        ss << ", ";
                    } else {
                        ss << " ";
                    }
                }
                ss << "FROM '" << impl.table.name << "' ";
                return ss;
            }

            template<class T, class... Args>
            std::string string_from_expression(const get_all_t<T, Args...> &get, bool noTableName) const {
                std::stringstream ss = this->string_from_expression_impl_get_all<T>(noTableName);
                this->process_conditions(ss, get.conditions);
                return ss.str();
            }

            template<class T, class... Args>
            std::string string_from_expression(const get_all_pointer_t<T, Args...> &get, bool noTableName) const {
                std::stringstream ss = this->string_from_expression_impl_get_all<T>(noTableName);
                this->process_conditions(ss, get.conditions);
                return ss.str();
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Args>
            std::string string_from_expression(const get_all_optional_t<T, Args...> &get, bool noTableName) const {
                std::stringstream ss = this->string_from_expression_impl_get_all<T>(noTableName);
                this->process_conditions(ss, get.conditions);
                return ss.str();
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class... Args, class... Wargs>
            std::string string_from_expression(const update_all_t<set_t<Args...>, Wargs...> &upd,
                                               bool /*noTableName*/) const {
                std::stringstream ss;
                ss << "UPDATE ";
                std::set<std::pair<std::string, std::string>> tableNamesSet;
                iterate_tuple(upd.set.assigns, [this, &tableNamesSet](auto &asgn) {
                    auto tableName = this->parse_table_name(asgn.lhs);
                    tableNamesSet.insert(tableName.begin(), tableName.end());
                });
                if(!tableNamesSet.empty()) {
                    if(tableNamesSet.size() == 1) {
                        ss << " '" << tableNamesSet.begin()->first << "' ";
                        ss << static_cast<std::string>(upd.set) << " ";
                        std::vector<std::string> setPairs;
                        iterate_tuple(upd.set.assigns, [this, &setPairs](auto &asgn) {
                            std::stringstream sss;
                            sss << this->string_from_expression(asgn.lhs, true);
                            sss << " " << static_cast<std::string>(asgn) << " ";
                            sss << this->string_from_expression(asgn.rhs, false) << " ";
                            setPairs.push_back(sss.str());
                        });
                        auto setPairsCount = setPairs.size();
                        for(size_t i = 0; i < setPairsCount; ++i) {
                            ss << setPairs[i] << " ";
                            if(i < setPairsCount - 1) {
                                ss << ", ";
                            }
                        }
                        this->process_conditions(ss, upd.conditions);
                        return ss.str();
                    } else {
                        throw std::system_error(std::make_error_code(orm_error_code::too_many_tables_specified));
                    }
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::incorrect_set_fields_specified));
                }
            }

            template<class T, class... Args>
            std::string string_from_expression(const remove_all_t<T, Args...> &rem, bool /*noTableName*/) const {
                auto &impl = this->get_impl<T>();
                std::stringstream ss;
                ss << "DELETE FROM '" << impl.table.name << "' ";
                this->process_conditions(ss, rem.conditions);
                return ss.str();
            }

            // Common code for statements with conditions: get_t, get_pointer_t, get_optional_t.
            template<class T>
            std::string string_from_expression_impl_get(bool /*noTableName*/) const {
                auto &impl = this->get_impl<T>();
                std::stringstream ss;
                ss << "SELECT ";
                auto columnNames = impl.table.column_names();
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << "\"" << columnNames[i] << "\"";
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << "FROM '" << impl.table.name << "' WHERE ";
                auto primaryKeyColumnNames = impl.table.primary_key_column_names();
                if(!primaryKeyColumnNames.empty()) {
                    for(size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                        ss << "\"" << primaryKeyColumnNames[i] << "\""
                           << " = ? ";
                        if(i < primaryKeyColumnNames.size() - 1) {
                            ss << "AND ";
                        }
                        ss << ' ';
                    }
                    return ss.str();
                } else {
                    throw std::system_error(std::make_error_code(orm_error_code::table_has_no_primary_key_column));
                }
            }

            template<class T, class... Ids>
            std::string string_from_expression(const get_t<T, Ids...> &, bool noTableName) const {
                return this->string_from_expression_impl_get<T>(noTableName);
            }

            template<class T, class... Ids>
            std::string string_from_expression(const get_pointer_t<T, Ids...> &, bool noTableName) const {
                return this->string_from_expression_impl_get<T>(noTableName);
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Ids>
            std::string string_from_expression(const get_optional_t<T, Ids...> &, bool noTableName) const {
                return this->string_from_expression_impl_get<T>(noTableName);
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class T>
            std::string string_from_expression(const update_t<T> &upd, bool /*noTableName*/) const {
                using expression_type = typename std::decay<decltype(upd)>::type;
                using object_type = typename expression_object_type<expression_type>::type;
                auto &impl = this->get_impl<object_type>();

                std::stringstream ss;
                ss << "UPDATE '" << impl.table.name << "' SET ";
                std::vector<std::string> setColumnNames;
                impl.table.for_each_column([&setColumnNames](auto &c) {
                    if(!c.template has<constraints::primary_key_t<>>()) {
                        setColumnNames.emplace_back(c.name);
                    }
                });
                for(size_t i = 0; i < setColumnNames.size(); ++i) {
                    ss << "\"" << setColumnNames[i] << "\""
                       << " = ?";
                    if(i < setColumnNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << "WHERE ";
                auto primaryKeyColumnNames = impl.table.primary_key_column_names();
                for(size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                    ss << "\"" << primaryKeyColumnNames[i] << "\""
                       << " = ?";
                    if(i < primaryKeyColumnNames.size() - 1) {
                        ss << " AND";
                    }
                    ss << " ";
                }
                return ss.str();
            }

            template<class T, class... Ids>
            std::string string_from_expression(const remove_t<T, Ids...> &, bool /*noTableName*/) const {
                auto &impl = this->get_impl<T>();
                std::stringstream ss;
                ss << "DELETE FROM '" << impl.table.name << "' ";
                ss << "WHERE ";
                auto primaryKeyColumnNames = impl.table.primary_key_column_names();
                for(size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                    ss << "\"" << primaryKeyColumnNames[i] << "\""
                       << " = ? ";
                    if(i < primaryKeyColumnNames.size() - 1) {
                        ss << "AND ";
                    }
                }
                return ss.str();
            }

            template<class T, class... Cols>
            std::string string_from_expression(const insert_explicit<T, Cols...> &ins, bool /*noTableName*/) const {
                constexpr const size_t colsCount = std::tuple_size<std::tuple<Cols...>>::value;
                static_assert(colsCount > 0, "Use insert or replace with 1 argument instead");
                using expression_type = typename std::decay<decltype(ins)>::type;
                using object_type = typename expression_object_type<expression_type>::type;
                this->assert_mapped_type<object_type>();
                auto &impl = this->get_impl<object_type>();
                std::stringstream ss;
                ss << "INSERT INTO '" << impl.table.name << "' ";
                std::vector<std::string> columnNames;
                columnNames.reserve(colsCount);
                iterate_tuple(ins.columns.columns, [&columnNames, this](auto &m) {
                    auto columnName = this->string_from_expression(m, true);
                    if(!columnName.empty()) {
                        columnNames.push_back(columnName);
                    } else {
                        throw std::system_error(std::make_error_code(orm_error_code::column_not_found));
                    }
                });
                ss << "(";
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << columnNames[i];
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    } else {
                        ss << ")";
                    }
                    ss << " ";
                }
                ss << "VALUES (";
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << "?";
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    } else {
                        ss << ")";
                    }
                    ss << " ";
                }
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const insert_t<T> &ins, bool /*noTableName*/) const {
                using expression_type = typename std::decay<decltype(ins)>::type;
                using object_type = typename expression_object_type<expression_type>::type;
                this->assert_mapped_type<object_type>();
                auto &impl = this->get_impl<object_type>();
                std::stringstream ss;
                ss << "INSERT INTO '" << impl.table.name << "' ";
                std::vector<std::string> columnNames;
                auto compositeKeyColumnNames = impl.table.composite_key_columns_names();

                impl.table.for_each_column([&impl, &columnNames, &compositeKeyColumnNames](auto &c) {
                    if(impl.table._without_rowid || !c.template has<constraints::primary_key_t<>>()) {
                        auto it = std::find(compositeKeyColumnNames.begin(), compositeKeyColumnNames.end(), c.name);
                        if(it == compositeKeyColumnNames.end()) {
                            columnNames.emplace_back(c.name);
                        }
                    }
                });

                auto columnNamesCount = columnNames.size();
                if(columnNamesCount) {
                    ss << "(";
                    for(size_t i = 0; i < columnNamesCount; ++i) {
                        ss << "\"" << columnNames[i] << "\"";
                        if(i < columnNamesCount - 1) {
                            ss << ",";
                        } else {
                            ss << ")";
                        }
                        ss << " ";
                    }
                } else {
                    ss << "DEFAULT ";
                }
                ss << "VALUES ";
                if(columnNamesCount) {
                    ss << "(";
                    for(size_t i = 0; i < columnNamesCount; ++i) {
                        ss << "?";
                        if(i < columnNamesCount - 1) {
                            ss << ", ";
                        } else {
                            ss << ")";
                        }
                    }
                }
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const replace_t<T> &rep, bool /*noTableName*/) const {
                using expression_type = typename std::decay<decltype(rep)>::type;
                using object_type = typename expression_object_type<expression_type>::type;
                this->assert_mapped_type<object_type>();
                auto &impl = this->get_impl<object_type>();
                std::stringstream ss;
                ss << "REPLACE INTO '" << impl.table.name << "' (";
                auto columnNames = impl.table.column_names();
                auto columnNamesCount = columnNames.size();
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "\"" << columnNames[i] << "\"";
                    if(i < columnNamesCount - 1) {
                        ss << ",";
                    } else {
                        ss << ")";
                    }
                    ss << " ";
                }
                ss << "VALUES(";
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "?";
                    if(i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ")";
                    }
                }
                return ss.str();
            }

            template<class It>
            std::string string_from_expression(const replace_range_t<It> &rep, bool /*noTableName*/) const {
                using expression_type = typename std::decay<decltype(rep)>::type;
                using object_type = typename expression_type::object_type;
                auto &impl = this->get_impl<object_type>();
                std::stringstream ss;
                ss << "REPLACE INTO '" << impl.table.name << "' (";
                auto columnNames = impl.table.column_names();
                auto columnNamesCount = columnNames.size();
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "\"" << columnNames[i] << "\"";
                    if(i < columnNamesCount - 1) {
                        ss << ", ";
                    } else {
                        ss << ") ";
                    }
                }
                ss << "VALUES ";
                auto valuesString = [columnNamesCount] {
                    std::stringstream ss;
                    ss << "(";
                    for(size_t i = 0; i < columnNamesCount; ++i) {
                        ss << "?";
                        if(i < columnNamesCount - 1) {
                            ss << ", ";
                        } else {
                            ss << ")";
                        }
                    }
                    return ss.str();
                }();
                auto valuesCount = static_cast<int>(std::distance(rep.range.first, rep.range.second));
                for(auto i = 0; i < valuesCount; ++i) {
                    ss << valuesString;
                    if(i < valuesCount - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                return ss.str();
            }

            template<class It>
            std::string string_from_expression(const insert_range_t<It> &ins, bool /*noTableName*/) const {
                using expression_type = typename std::decay<decltype(ins)>::type;
                using object_type = typename expression_type::object_type;
                auto &impl = this->get_impl<object_type>();

                std::stringstream ss;
                ss << "INSERT INTO '" << impl.table.name << "' (";
                std::vector<std::string> columnNames;
                impl.table.for_each_column([&columnNames](auto &c) {
                    if(!c.template has<constraints::primary_key_t<>>()) {
                        columnNames.emplace_back(c.name);
                    }
                });

                auto columnNamesCount = columnNames.size();
                for(size_t i = 0; i < columnNamesCount; ++i) {
                    ss << "\"" << columnNames[i] << "\"";
                    if(i < columnNamesCount - 1) {
                        ss << ",";
                    } else {
                        ss << ")";
                    }
                    ss << " ";
                }
                ss << "VALUES ";
                auto valuesString = [columnNamesCount] {
                    std::stringstream ss;
                    ss << "(";
                    for(size_t i = 0; i < columnNamesCount; ++i) {
                        ss << "?";
                        if(i < columnNamesCount - 1) {
                            ss << ", ";
                        } else {
                            ss << ")";
                        }
                    }
                    return ss.str();
                }();
                auto valuesCount = static_cast<int>(std::distance(ins.range.first, ins.range.second));
                for(auto i = 0; i < valuesCount; ++i) {
                    ss << valuesString;
                    if(i < valuesCount - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                return ss.str();
            }

            template<class T, class E>
            std::string string_from_expression(const conditions::cast_t<T, E> &c, bool noTableName) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " (";
                ss << this->string_from_expression(c.expression, noTableName) << " AS " << type_printer<T>().print()
                   << ")";
                return ss.str();
            }

            template<class T>
            typename std::enable_if<is_base_of_template<T, compound_operator>::value, std::string>::type
            string_from_expression(const T &op, bool noTableName) const {
                std::stringstream ss;
                ss << this->string_from_expression(op.left, noTableName) << " ";
                ss << static_cast<std::string>(op) << " ";
                ss << this->string_from_expression(op.right, noTableName);
                return ss.str();
            }

            template<class R, class T, class E, class... Args>
            std::string string_from_expression(const internal::simple_case_t<R, T, E, Args...> &c,
                                               bool noTableName) const {
                std::stringstream ss;
                ss << "CASE ";
                c.case_expression.apply([&ss, this, noTableName](auto &c) {
                    ss << this->string_from_expression(c, noTableName) << " ";
                });
                iterate_tuple(c.args, [&ss, this, noTableName](auto &pair) {
                    ss << "WHEN " << this->string_from_expression(pair.first, noTableName) << " ";
                    ss << "THEN " << this->string_from_expression(pair.second, noTableName) << " ";
                });
                c.else_expression.apply([&ss, this, noTableName](auto &el) {
                    ss << "ELSE " << this->string_from_expression(el, noTableName) << " ";
                });
                ss << "END";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const conditions::is_null_t<T> &c, bool noTableName) const {
                std::stringstream ss;
                ss << this->string_from_expression(c.t, noTableName) << " " << static_cast<std::string>(c) << " ";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const conditions::is_not_null_t<T> &c, bool noTableName) const {
                std::stringstream ss;
                ss << this->string_from_expression(c.t, noTableName) << " " << static_cast<std::string>(c) << " ";
                return ss.str();
            }

            template<class C>
            std::string string_from_expression(const conditions::negated_condition_t<C> &c, bool noTableName) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " ";
                auto cString = this->string_from_expression(c.c, noTableName);
                ss << " (" << cString << " ) ";
                return ss.str();
            }

            template<class L, class R>
            std::string string_from_expression(const conditions::is_equal_t<L, R> &c, bool noTableName) const {
                auto leftString = this->string_from_expression(c.l, noTableName);
                auto rightString = this->string_from_expression(c.r, noTableName);
                std::stringstream ss;
                ss << leftString << " " << static_cast<std::string>(c) << " " << rightString;
                return ss.str();
            }

            template<class C>
            typename std::enable_if<is_base_of_template<C, conditions::binary_condition>::value, std::string>::type
            string_from_expression(const C &c, bool noTableName) const {
                auto leftString = this->string_from_expression(c.l, noTableName);
                auto rightString = this->string_from_expression(c.r, noTableName);
                std::stringstream ss;
                ss << "(" << leftString << " " << static_cast<std::string>(c) << " " << rightString << ")";
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const conditions::named_collate<T> &col, bool noTableName) const {
                auto res = this->string_from_expression(col.expr, noTableName);
                return res + " " + static_cast<std::string>(col);
            }

            template<class T>
            std::string string_from_expression(const conditions::collate_t<T> &col, bool noTableName) const {
                auto res = this->string_from_expression(col.expr, noTableName);
                return res + " " + static_cast<std::string>(col);
            }

            template<class L, class A>
            std::string string_from_expression(const conditions::in_t<L, A> &inCondition, bool noTableName) const {
                std::stringstream ss;
                auto leftString = this->string_from_expression(inCondition.l, noTableName);
                ss << leftString << " " << static_cast<std::string>(inCondition) << " ";
                ss << this->string_from_expression(inCondition.arg, noTableName);
                return ss.str();
            }

            template<class L, class E>
            std::string string_from_expression(const conditions::in_t<L, std::vector<E>> &inCondition,
                                               bool noTableName) const {
                std::stringstream ss;
                auto leftString = this->string_from_expression(inCondition.l, noTableName);
                ss << leftString << " " << static_cast<std::string>(inCondition) << " ( ";
                for(size_t index = 0; index < inCondition.arg.size(); ++index) {
                    auto &value = inCondition.arg[index];
                    ss << " " << this->string_from_expression(value, noTableName);
                    if(index < inCondition.arg.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << " )";
                return ss.str();
            }

            template<class A, class T, class E>
            std::string string_from_expression(const conditions::like_t<A, T, E> &l, bool noTableName) const {
                std::stringstream ss;
                ss << this->string_from_expression(l.arg, noTableName) << " ";
                ss << static_cast<std::string>(l) << " ";
                ss << this->string_from_expression(l.pattern, noTableName);
                l.arg3.apply([&ss, this, noTableName](auto &value) {
                    ss << " ESCAPE " << this->string_from_expression(value, noTableName);
                });
                return ss.str();
            }

            template<class A, class T>
            std::string string_from_expression(const conditions::glob_t<A, T> &l, bool noTableName) const {
                std::stringstream ss;
                ss << this->string_from_expression(l.arg, noTableName) << " ";
                ss << static_cast<std::string>(l) << " ";
                ss << this->string_from_expression(l.pattern, noTableName);
                return ss.str();
            }

            template<class A, class T>
            std::string string_from_expression(const conditions::between_t<A, T> &bw, bool noTableName) const {
                std::stringstream ss;
                auto expr = this->string_from_expression(bw.expr, noTableName);
                ss << expr << " " << static_cast<std::string>(bw) << " ";
                ss << this->string_from_expression(bw.b1, noTableName);
                ss << " AND ";
                ss << this->string_from_expression(bw.b2, noTableName);
                return ss.str();
            }

            template<class T>
            std::string string_from_expression(const conditions::exists_t<T> &e, bool noTableName) const {
                std::stringstream ss;
                ss << static_cast<std::string>(e) << " ";
                ss << this->string_from_expression(e.t, noTableName);
                return ss.str();
            }

            template<class O>
            std::string process_order_by(const conditions::order_by_t<O> &orderBy) const {
                std::stringstream ss;
                auto columnName = this->string_from_expression(orderBy.o, false);
                ss << columnName << " ";
                if(orderBy._collate_argument.length()) {
                    ss << "COLLATE " << orderBy._collate_argument << " ";
                }
                switch(orderBy.asc_desc) {
                    case 1:
                        ss << "ASC";
                        break;
                    case -1:
                        ss << "DESC";
                        break;
                }
                return ss.str();
            }

            template<class T>
            void process_join_constraint(std::stringstream &ss, const conditions::on_t<T> &t) const {
                ss << static_cast<std::string>(t) << " " << this->string_from_expression(t.arg, false);
            }

            template<class F, class O>
            void process_join_constraint(std::stringstream &ss, const conditions::using_t<F, O> &u) const {
                ss << static_cast<std::string>(u) << " (" << this->string_from_expression(u.column, true) << " )";
            }

            /**
             *  HO - has offset
             *  OI - offset is implicit
             */
            template<class T, bool HO, bool OI, class O>
            void process_single_condition(std::stringstream &ss, const conditions::limit_t<T, HO, OI, O> &limt) const {
                ss << static_cast<std::string>(limt) << " ";
                if(HO) {
                    if(OI) {
                        limt.off.apply([this, &ss](auto &value) {
                            ss << this->string_from_expression(value, false);
                        });
                        ss << ", ";
                        ss << this->string_from_expression(limt.lim, false);
                    } else {
                        ss << this->string_from_expression(limt.lim, false) << " OFFSET ";
                        limt.off.apply([this, &ss](auto &value) {
                            ss << this->string_from_expression(value, false);
                        });
                    }
                } else {
                    ss << this->string_from_expression(limt.lim, false);
                }
            }

            template<class O>
            void process_single_condition(std::stringstream &ss, const conditions::cross_join_t<O> &c) const {
                ss << static_cast<std::string>(c) << " ";
                ss << " '" << this->impl.template find_table_name<O>() << "'";
            }

            template<class O>
            void process_single_condition(std::stringstream &ss, const conditions::natural_join_t<O> &c) const {
                ss << static_cast<std::string>(c) << " ";
                ss << " '" << this->impl.template find_table_name<O>() << "'";
            }

            template<class T, class O>
            void process_single_condition(std::stringstream &ss, const conditions::inner_join_t<T, O> &l) const {
                ss << static_cast<std::string>(l) << " ";
                auto aliasString = alias_extractor<T>::get();
                ss << " '" << this->impl.template find_table_name<typename mapped_type_proxy<T>::type>() << "' ";
                if(aliasString.length()) {
                    ss << "'" << aliasString << "' ";
                }
                this->process_join_constraint(ss, l.constraint);
            }

            template<class T, class O>
            void process_single_condition(std::stringstream &ss, const conditions::left_outer_join_t<T, O> &l) const {
                ss << static_cast<std::string>(l) << " ";
                ss << " '" << this->impl.template find_table_name<T>() << "' ";
                this->process_join_constraint(ss, l.constraint);
            }

            template<class T, class O>
            void process_single_condition(std::stringstream &ss, const conditions::left_join_t<T, O> &l) const {
                ss << static_cast<std::string>(l) << " ";
                ss << " '" << this->impl.template find_table_name<T>() << "' ";
                this->process_join_constraint(ss, l.constraint);
            }

            template<class T, class O>
            void process_single_condition(std::stringstream &ss, const conditions::join_t<T, O> &l) const {
                ss << static_cast<std::string>(l) << " ";
                ss << " '" << this->impl.template find_table_name<T>() << "' ";
                this->process_join_constraint(ss, l.constraint);
            }

            template<class C>
            void process_single_condition(std::stringstream &ss, const conditions::where_t<C> &w) const {
                ss << static_cast<std::string>(w) << " ";
                auto whereString = this->string_from_expression(w.c, false);
                ss << "( " << whereString << ") ";
            }

            template<class O>
            void process_single_condition(std::stringstream &ss, const conditions::order_by_t<O> &orderBy) const {
                ss << static_cast<std::string>(orderBy) << " ";
                auto orderByString = this->process_order_by(orderBy);
                ss << orderByString << " ";
            }

            template<class... Args>
            void process_single_condition(std::stringstream &ss,
                                          const conditions::multi_order_by_t<Args...> &orderBy) const {
                std::vector<std::string> expressions;
                iterate_tuple(orderBy.args, [&expressions, this](auto &v) {
                    auto expression = this->process_order_by(v);
                    expressions.push_back(std::move(expression));
                });
                ss << static_cast<std::string>(orderBy) << " ";
                for(size_t i = 0; i < expressions.size(); ++i) {
                    ss << expressions[i];
                    if(i < expressions.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << " ";
            }

            template<class S>
            void process_single_condition(std::stringstream &ss,
                                          const conditions::dynamic_order_by_t<S> &orderBy) const {
                ss << this->storage_base::process_order_by(orderBy) << " ";
            }

            template<class... Args>
            void process_single_condition(std::stringstream &ss, const conditions::group_by_t<Args...> &groupBy) const {
                std::vector<std::string> expressions;
                iterate_tuple(groupBy.args, [&expressions, this](auto &v) {
                    auto expression = this->string_from_expression(v, false);
                    expressions.push_back(expression);
                });
                ss << static_cast<std::string>(groupBy) << " ";
                for(size_t i = 0; i < expressions.size(); ++i) {
                    ss << expressions[i];
                    if(i < expressions.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << " ";
            }

            template<class T>
            void process_single_condition(std::stringstream &ss, const conditions::having_t<T> &hav) const {
                ss << static_cast<std::string>(hav) << " ";
                ss << this->string_from_expression(hav.t, false) << " ";
            }

            template<class... Args>
            void process_conditions(std::stringstream &ss, const std::tuple<Args...> &args) const {
                iterate_tuple(args, [this, &ss](auto &v) {
                    this->process_single_condition(ss, v);
                });
            }

          public:
            template<class T, class... Args>
            view_t<T, self, Args...> iterate(Args &&... args) {
                this->assert_mapped_type<T>();

                auto con = this->get_connection();
                return {*this, std::move(con), std::forward<Args>(args)...};
            }

            template<class O, class... Args>
            void remove_all(Args &&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::remove_all<O>(std::forward<Args>(args)...));
                this->execute(statement);
            }

            /**
             *  Delete routine.
             *  O is an object's type. Must be specified explicitly.
             *  @param ids ids of object to be removed.
             */
            template<class O, class... Ids>
            void remove(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::remove<O>(std::forward<Ids>(ids)...));
                this->execute(statement);
            }

            /**
             *  Update routine. Sets all non primary key fields where primary key is equal.
             *  O is an object type. May be not specified explicitly cause it can be deduced by
             *      compiler from first parameter.
             *  @param o object to be updated.
             */
            template<class O>
            void update(const O &o) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::update(std::ref(o)));
                this->execute(statement);
            }

            template<class... Args, class... Wargs>
            void update_all(internal::set_t<Args...> set, Wargs... wh) {
                auto statement = this->prepare(sqlite_orm::update_all(std::move(set), std::forward<Wargs>(wh)...));
                this->execute(statement);
            }

          protected:
            template<class T>
            std::set<std::pair<std::string, std::string>> parse_table_name(const T &) const {
                return {};
            }

            template<class F, class O>
            std::set<std::pair<std::string, std::string>> parse_table_name(F O::*, std::string alias = {}) const {
                return {std::make_pair(this->impl.template find_table_name<O>(), std::move(alias))};
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::min_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::max_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::sum_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::total_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::group_concat_double_t<T> &f) const {
                auto res = this->parse_table_name(f.arg);
                auto secondSet = this->parse_table_name(f.y);
                res.insert(secondSet.begin(), secondSet.end());
                return res;
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::group_concat_single_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::count_t<T> &f) const {
                return this->parse_table_name(f.arg);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::avg_t<T> &a) const {
                return this->parse_table_name(a.arg);
            }

            template<class R, class S, class... Args>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const core_functions::core_function_t<R, S, Args...> &f) const {
                std::set<std::pair<std::string, std::string>> res;
                iterate_tuple(f.args, [&res, this](auto &v) {
                    auto tableNames = this->parse_table_name(v);
                    res.insert(tableNames.begin(), tableNames.end());
                });
                return res;
            }

            template<class T>
            std::set<std::pair<std::string, std::string>> parse_table_name(const distinct_t<T> &f) const {
                return this->parse_table_name(f.t);
            }

            template<class T>
            std::set<std::pair<std::string, std::string>> parse_table_name(const all_t<T> &f) const {
                return this->parse_table_name(f.t);
            }

            template<class L, class R, class... Ds>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const binary_operator<L, R, Ds...> &f) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftSet = this->parse_table_name(f.lhs);
                res.insert(leftSet.begin(), leftSet.end());
                auto rightSet = this->parse_table_name(f.rhs);
                res.insert(rightSet.begin(), rightSet.end());
                return res;
            }

            template<class T, class F>
            std::set<std::pair<std::string, std::string>> parse_table_name(const column_pointer<T, F> &) const {
                std::set<std::pair<std::string, std::string>> res;
                res.insert({this->impl.template find_table_name<T>(), ""});
                return res;
            }

            template<class T, class C>
            std::set<std::pair<std::string, std::string>> parse_table_name(const alias_column_t<T, C> &a) const {
                return this->parse_table_name(a.column, alias_extractor<T>::get());
            }

            template<class T>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::count_asterisk_t<T> &) const {
                auto tableName = this->impl.template find_table_name<T>();
                if(!tableName.empty()) {
                    return {std::make_pair(std::move(tableName), "")};
                } else {
                    return {};
                }
            }

            std::set<std::pair<std::string, std::string>>
            parse_table_name(const aggregate_functions::count_asterisk_without_type &) const {
                return {};
            }

            template<class T>
            std::set<std::pair<std::string, std::string>> parse_table_name(const asterisk_t<T> &) const {
                auto tableName = this->impl.template find_table_name<T>();
                return {std::make_pair(std::move(tableName), "")};
            }

            template<class T, class E>
            std::set<std::pair<std::string, std::string>> parse_table_name(const conditions::cast_t<T, E> &c) const {
                return this->parse_table_name(c.expression);
            }

            template<class R, class T, class E, class... Args>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const simple_case_t<R, T, E, Args...> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                c.case_expression.apply([this, &res](auto &c) {
                    auto caseExpressionSet = this->parse_table_name(c);
                    res.insert(caseExpressionSet.begin(), caseExpressionSet.end());
                });
                iterate_tuple(c.args, [this, &res](auto &pair) {
                    auto leftSet = this->parse_table_name(pair.first);
                    res.insert(leftSet.begin(), leftSet.end());
                    auto rightSet = this->parse_table_name(pair.second);
                    res.insert(rightSet.begin(), rightSet.end());
                });
                c.else_expression.apply([this, &res](auto &el) {
                    auto tableNames = this->parse_table_name(el);
                    res.insert(tableNames.begin(), tableNames.end());
                });
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::and_condition_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::or_condition_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::is_equal_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::is_not_equal_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::greater_than_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::greater_or_equal_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::lesser_than_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class L, class R>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::lesser_or_equal_t<L, R> &c) const {
                std::set<std::pair<std::string, std::string>> res;
                auto leftTableNames = this->parse_table_name(c.l);
                res.insert(leftTableNames.begin(), leftTableNames.end());
                auto rightTableNames = this->parse_table_name(c.r);
                res.insert(rightTableNames.begin(), rightTableNames.end());
                return res;
            }

            template<class A, class T, class E>
            std::set<std::pair<std::string, std::string>> parse_table_name(const conditions::like_t<A, T, E> &l) const {
                std::set<std::pair<std::string, std::string>> res;
                auto argTableNames = this->parse_table_name(l.arg);
                res.insert(argTableNames.begin(), argTableNames.end());
                auto patternTableNames = this->parse_table_name(l.pattern);
                res.insert(patternTableNames.begin(), patternTableNames.end());
                l.arg3.apply([&res, this](auto &value) {
                    auto escapeTableNames = this->parse_table_name(value);
                    res.insert(escapeTableNames.begin(), escapeTableNames.end());
                });
                return res;
            }

            template<class A, class T>
            std::set<std::pair<std::string, std::string>> parse_table_name(const conditions::glob_t<A, T> &l) const {
                std::set<std::pair<std::string, std::string>> res;
                auto argTableNames = this->parse_table_name(l.arg);
                res.insert(argTableNames.begin(), argTableNames.end());
                auto patternTableNames = this->parse_table_name(l.pattern);
                res.insert(patternTableNames.begin(), patternTableNames.end());
                return res;
            }

            template<class C>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const conditions::negated_condition_t<C> &c) const {
                return this->parse_table_name(c.c);
            }

            template<class T, class E>
            std::set<std::pair<std::string, std::string>> parse_table_name(const as_t<T, E> &a) const {
                return this->parse_table_name(a.expression);
            }

            template<class... Args>
            std::set<std::pair<std::string, std::string>>
            parse_table_name(const internal::columns_t<Args...> &cols) const {
                std::set<std::pair<std::string, std::string>> res;
                iterate_tuple(cols.columns, [&res, this](auto &m) {
                    auto tableName = this->parse_table_name(m);
                    res.insert(tableName.begin(), tableName.end());
                });
                return res;
            }

            template<class F, class O, class... Args>
            std::string group_concat_internal(F O::*m, std::unique_ptr<std::string> y, Args &&... args) {
                this->assert_mapped_type<O>();
                std::vector<std::string> rows;
                if(y) {
                    rows = this->select(sqlite_orm::group_concat(m, move(*y)), std::forward<Args>(args)...);
                } else {
                    rows = this->select(sqlite_orm::group_concat(m), std::forward<Args>(args)...);
                }
                if(!rows.empty()) {
                    return move(rows.front());
                } else {
                    return {};
                }
            }

          public:
            /**
             *  Select * with no conditions routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return All objects of type O stored in database at the moment.
             */
            template<class O, class C = std::vector<O>, class... Args>
            C get_all(Args &&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all<O>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  Select * with no conditions routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return All objects of type O as std::unique_ptr<O> stored in database at the moment.
             */
            template<class O, class C = std::vector<std::unique_ptr<O>>, class... Args>
            C get_all_pointer(Args &&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all_pointer<O>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  Select * by id routine.
             *  throws std::system_error(orm_error_code::not_found, orm_error_category) if object not found with given
             * id. throws std::system_error with orm_error_category in case of db error. O is an object type to be
             * extracted. Must be specified explicitly.
             *  @return Object of type O where id is equal parameter passed or throws
             * `std::system_error(orm_error_code::not_found, orm_error_category)` if there is no object with such id.
             */
            template<class O, class... Ids>
            O get(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }

            /**
             *  The same as `get` function but doesn't throw an exception if noting found but returns std::unique_ptr
             * with null value. throws std::system_error in case of db error.
             */
            template<class O, class... Ids>
            std::unique_ptr<O> get_pointer(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_pointer<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }

            /**
             * A previous version of get_pointer() that returns a shared_ptr
             * instead of a unique_ptr. New code should prefer get_pointer()
             * unless the data needs to be shared.
             *
             * @note
             * Most scenarios don't need shared ownership of data, so we should prefer
             * unique_ptr when possible. It's more efficient, doesn't require atomic
             * ops for a reference count (which can cause major slowdowns on
             * weakly-ordered platforms like ARM), and can be easily promoted to a
             * shared_ptr, exactly like we're doing here.
             * (Conversely, you _can't_ go from shared back to unique.)
             */
            template<class O, class... Ids>
            std::shared_ptr<O> get_no_throw(Ids... ids) {
                return std::shared_ptr<O>(get_pointer<O>(std::forward<Ids>(ids)...));
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            /**
             *  The same as `get` function but doesn't throw an exception if noting found but
             * returns an empty std::optional. throws std::system_error in case of db error.
             */
            template<class O, class... Ids>
            std::optional<O> get_optional(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_optional<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            /**
             *  SELECT COUNT(*) https://www.sqlite.org/lang_aggfunc.html#count
             *  @return Number of O object in table.
             */
            template<class O, class... Args, class R = typename mapped_type_proxy<O>::type>
            int count(Args &&... args) {
                this->assert_mapped_type<R>();
                auto rows = this->select(sqlite_orm::count<R>(), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            /**
             *  SELECT COUNT(X) https://www.sqlite.org/lang_aggfunc.html#count
             *  @param m member pointer to class mapped to the storage.
             */
            template<class F, class O, class... Args>
            int count(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::count(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            /**
             *  AVG(X) query.   https://www.sqlite.org/lang_aggfunc.html#avg
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return average value from db.
             */
            template<class F, class O, class... Args>
            double avg(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::avg(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            template<class F, class O>
            std::string group_concat(F O::*m) {
                return this->group_concat_internal(m, {});
            }

            /**
             *  GROUP_CONCAT(X) query.  https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
            template<class F,
                     class O,
                     class... Args,
                     class Tuple = std::tuple<Args...>,
                     typename sfinae = typename std::enable_if<std::tuple_size<Tuple>::value >= 1>::type>
            std::string group_concat(F O::*m, Args &&... args) {
                return this->group_concat_internal(m, {}, std::forward<Args>(args)...);
            }

            /**
             *  GROUP_CONCAT(X, Y) query.   https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
            template<class F, class O, class... Args>
            std::string group_concat(F O::*m, std::string y, Args &&... args) {
                return this->group_concat_internal(m,
                                                   std::make_unique<std::string>(move(y)),
                                                   std::forward<Args>(args)...);
            }

            template<class F, class O, class... Args>
            std::string group_concat(F O::*m, const char *y, Args &&... args) {
                std::unique_ptr<std::string> str;
                if(y) {
                    str = std::make_unique<std::string>(y);
                } else {
                    str = std::make_unique<std::string>();
                }
                return this->group_concat_internal(m, move(str), std::forward<Args>(args)...);
            }

            /**
             *  MAX(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with max value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
            std::unique_ptr<Ret> max(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::max(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  MIN(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with min value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
            std::unique_ptr<Ret> min(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::min(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  SUM(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with sum value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = typename column_result_t<self, F O::*>::type>
            std::unique_ptr<Ret> sum(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                std::vector<std::unique_ptr<double>> rows =
                    this->select(sqlite_orm::sum(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    if(rows.front()) {
                        return std::make_unique<Ret>(std::move(*rows.front()));
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }

            /**
             *  TOTAL(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return total value (the same as SUM but not nullable. More details here
             * https://www.sqlite.org/lang_aggfunc.html)
             */
            template<class F, class O, class... Args>
            double total(F O::*m, Args &&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::total(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  Select a single column into std::vector<T> or multiple columns into std::vector<std::tuple<...>>.
             *  For a single column use `auto rows = storage.select(&User::id, where(...));
             *  For multicolumns use `auto rows = storage.select(columns(&User::id, &User::name), where(...));
             */
            template<class T, class... Args, class R = typename column_result_t<self, T>::type>
            std::vector<R> select(T m, Args... args) {
                static_assert(!is_base_of_template<T, compound_operator>::value ||
                                  std::tuple_size<std::tuple<Args...>>::value == 0,
                              "Cannot use args with a compound operator");
                auto statement = this->prepare(sqlite_orm::select(std::move(m), std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  Returns a string representation of object of a class mapped to the storage.
             *  Type of string has json-like style.
             */
            template<class O>
            std::string dump(const O &o) {
                this->assert_mapped_type<O>();
                return this->impl.dump(o);
            }

            /**
             *  This is REPLACE (INSERT OR REPLACE) function.
             *  Also if you need to insert value with knows id you should
             *  also you this function instead of insert cause inserts ignores
             *  id and creates own one.
             */
            template<class O>
            void replace(const O &o) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::replace(std::ref(o)));
                this->execute(statement);
            }

            template<class It>
            void replace_range(It from, It to) {
                using O = typename std::iterator_traits<It>::value_type;
                this->assert_mapped_type<O>();
                if(from == to) {
                    return;
                }

                auto statement = this->prepare(sqlite_orm::replace_range(from, to));
                this->execute(statement);
            }

            template<class O, class... Cols>
            int insert(const O &o, columns_t<Cols...> cols) {
                constexpr const size_t colsCount = std::tuple_size<std::tuple<Cols...>>::value;
                static_assert(colsCount > 0, "Use insert or replace with 1 argument instead");
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::insert(std::ref(o), std::move(cols)));
                return int(this->execute(statement));
            }

            /**
             *  Insert routine. Inserts object with all non primary key fields in passed object. Id of passed
             *  object doesn't matter.
             *  @return id of just created object.
             */
            template<class O>
            int insert(const O &o) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::insert(std::ref(o)));
                return int(this->execute(statement));
            }

            template<class It>
            void insert_range(It from, It to) {
                using O = typename std::iterator_traits<It>::value_type;
                this->assert_mapped_type<O>();
                if(from == to) {
                    return;
                }

                auto statement = this->prepare(sqlite_orm::insert_range(from, to));
                this->execute(statement);
            }

          protected:
            template<class... Tss, class... Cols>
            sync_schema_result sync_table(storage_impl<internal::index_t<Cols...>, Tss...> *impl, sqlite3 *db, bool) {
                auto res = sync_schema_result::already_in_sync;
                std::stringstream ss;
                ss << "CREATE ";
                if(impl->table.unique) {
                    ss << "UNIQUE ";
                }
                using columns_type = typename decltype(impl->table)::columns_type;
                using head_t = typename std::tuple_element<0, columns_type>::type;
                using indexed_type = typename internal::table_type<head_t>::type;
                ss << "INDEX IF NOT EXISTS '" << impl->table.name << "' ON '"
                   << this->impl.template find_table_name<indexed_type>() << "' ( ";
                std::vector<std::string> columnNames;
                iterate_tuple(impl->table.columns, [&columnNames, this](auto &v) {
                    columnNames.push_back(this->impl.column_name(v));
                });
                for(size_t i = 0; i < columnNames.size(); ++i) {
                    ss << "'" << columnNames[i] << "'";
                    if(i < columnNames.size() - 1) {
                        ss << ",";
                    }
                    ss << " ";
                }
                ss << ") ";
                auto query = ss.str();
                auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
                if(rc != SQLITE_OK) {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }

            template<class... Tss, class... Cs>
            sync_schema_result sync_table(storage_impl<table_t<Cs...>, Tss...> *impl, sqlite3 *db, bool preserve) {
                auto res = sync_schema_result::already_in_sync;

                auto schema_stat = impl->schema_status(db, preserve);
                if(schema_stat != decltype(schema_stat)::already_in_sync) {
                    if(schema_stat == decltype(schema_stat)::new_table_created) {
                        this->create_table(db, impl->table.name, impl);
                        res = decltype(res)::new_table_created;
                    } else {
                        if(schema_stat == sync_schema_result::old_columns_removed ||
                           schema_stat == sync_schema_result::new_columns_added ||
                           schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                            //  get table info provided in `make_table` call..
                            auto storageTableInfo = impl->table.get_table_info();

                            //  now get current table info from db using `PRAGMA table_info` query..
                            auto dbTableInfo = impl->get_table_info(impl->table.name, db);

                            //  this vector will contain pointers to columns that gotta be added..
                            std::vector<table_info *> columnsToAdd;

                            impl->get_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo);

                            if(schema_stat == sync_schema_result::old_columns_removed) {

                                //  extra table columns than storage columns
                                this->backup_table(db, impl);
                                res = decltype(res)::old_columns_removed;
                            }

                            if(schema_stat == sync_schema_result::new_columns_added) {
                                for(auto columnPointer: columnsToAdd) {
                                    impl->add_column(*columnPointer, db);
                                }
                                res = decltype(res)::new_columns_added;
                            }

                            if(schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                                // remove extra columns
                                this->backup_table(db, impl);
                                for(auto columnPointer: columnsToAdd) {
                                    impl->add_column(*columnPointer, db);
                                }
                                res = decltype(res)::new_columns_added_and_old_columns_removed;
                            }
                        } else if(schema_stat == sync_schema_result::dropped_and_recreated) {
                            this->drop_table_internal(impl->table.name, db);
                            this->create_table(db, impl->table.name, impl);
                            res = decltype(res)::dropped_and_recreated;
                        }
                    }
                }
                return res;
            }

          public:
            /**
             *  This is a cute function used to replace migration up/down functionality.
             *  It performs check storage schema with actual db schema and:
             *  * if there are excess tables exist in db they are ignored (not dropped)
             *  * every table from storage is compared with it's db analog and
             *      * if table doesn't exist it is being created
             *      * if table exists its colums are being compared with table_info from db and
             *          * if there are columns in db that do not exist in storage (excess) table will be dropped and
             * recreated
             *          * if there are columns in storage that do not exist in db they will be added using `ALTER TABLE
             * ... ADD COLUMN ...' command
             *          * if there is any column existing in both db and storage but differs by any of
             * properties/constraints (type, pk, notnull, dflt_value) table will be dropped and recreated Be aware that
             * `sync_schema` doesn't guarantee that data will not be dropped. It guarantees only that it will make db
             * schema the same as you specified in `make_storage` function call. A good point is that if you have no db
             * file at all it will be created and all tables also will be created with exact tables and columns you
             * specified in `make_storage`, `make_table` and `make_column` call. The best practice is to call this
             * function right after storage creation.
             *  @param preserve affects on function behaviour in case it is needed to remove a column. If it is `false`
             * so table will be dropped if there is column to remove, if `true` -  table is being copied into another
             * table, dropped and copied table is renamed with source table name. Warning: sync_schema doesn't check
             * foreign keys cause it is unable to do so in sqlite3. If you know how to get foreign key info please
             * submit an issue https://github.com/fnc12/sqlite_orm/issues
             *  @return std::map with std::string key equal table name and `sync_schema_result` as value.
             * `sync_schema_result` is a enum value that stores table state after syncing a schema. `sync_schema_result`
             * can be printed out on std::ostream with `operator<<`.
             */
            std::map<std::string, sync_schema_result> sync_schema(bool preserve = false) {
                auto con = this->get_connection();
                std::map<std::string, sync_schema_result> result;
                auto db = con.get();
                this->impl.for_each([&result, db, preserve, this](auto impl) {
                    auto res = this->sync_table(impl, db, preserve);
                    result.insert({impl->table.name, res});
                });
                return result;
            }

            /**
             *  This function returns the same map that `sync_schema` returns but it
             *  doesn't perform `sync_schema` actually - just simulates it in case you want to know
             *  what will happen if you sync your schema.
             */
            std::map<std::string, sync_schema_result> sync_schema_simulate(bool preserve = false) {
                auto con = this->get_connection();
                std::map<std::string, sync_schema_result> result;
                auto db = con.get();
                this->impl.for_each([&result, db, preserve](auto impl) {
                    result.insert({impl->table.name, impl->schema_status(db, preserve)});
                });
                return result;
            }

            /**
             *  Checks whether table exists in db. Doesn't check storage itself - works only with actual database.
             *  Note: table can be not mapped to a storage
             *  @return true if table with a given name exists in db, false otherwise.
             */
            bool table_exists(const std::string &tableName) {
                auto con = this->get_connection();
                return this->impl.table_exists(tableName, con.get());
            }

            template<class T, class... Args>
            prepared_statement_t<select_t<T, Args...>> prepare(select_t<T, Args...> sel) {
                sel.highest_level = true;
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(sel, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(sel), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Args>
            prepared_statement_t<get_all_t<T, Args...>> prepare(get_all_t<T, Args...> get) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(get, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(get), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Args>
            prepared_statement_t<get_all_pointer_t<T, Args...>> prepare(get_all_pointer_t<T, Args...> get) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(get, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(get), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Args>
            prepared_statement_t<get_all_optional_t<T, Args...>> prepare(get_all_optional_t<T, Args...> get) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(get, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(get), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class... Args, class... Wargs>
            prepared_statement_t<update_all_t<set_t<Args...>, Wargs...>>
            prepare(update_all_t<set_t<Args...>, Wargs...> upd) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(upd, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(upd), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Args>
            prepared_statement_t<remove_all_t<T, Args...>> prepare(remove_all_t<T, Args...> rem) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(rem, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(rem), stmt, std::move(con)};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Ids>
            prepared_statement_t<get_t<T, Ids...>> prepare(get_t<T, Ids...> g) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(g, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(g), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Ids>
            prepared_statement_t<get_pointer_t<T, Ids...>> prepare(get_pointer_t<T, Ids...> g) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(g, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(g), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Ids>
            prepared_statement_t<get_optional_t<T, Ids...>> prepare(get_optional_t<T, Ids...> g) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(g, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(g), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class T>
            prepared_statement_t<update_t<T>> prepare(update_t<T> upd) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(upd, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(upd), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Ids>
            prepared_statement_t<remove_t<T, Ids...>> prepare(remove_t<T, Ids...> rem) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(rem, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(rem), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T>
            prepared_statement_t<insert_t<T>> prepare(insert_t<T> ins) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(ins, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(ins), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T>
            prepared_statement_t<replace_t<T>> prepare(replace_t<T> rep) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(rep, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(rep), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class It>
            prepared_statement_t<insert_range_t<It>> prepare(insert_range_t<It> ins) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(ins, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(ins), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class It>
            prepared_statement_t<replace_range_t<It>> prepare(replace_range_t<It> rep) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(rep, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(rep), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Cols>
            prepared_statement_t<insert_explicit<T, Cols...>> prepare(insert_explicit<T, Cols...> ins) {
                auto con = this->get_connection();
                sqlite3_stmt *stmt;
                auto db = con.get();
                auto query = this->string_from_expression(ins, false);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return {std::move(ins), stmt, con};
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Cols>
            int64 execute(const prepared_statement_t<insert_explicit<T, Cols...>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;
                auto index = 1;
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto &impl = this->get_impl<object_type>();
                auto &o = statement.t.obj;
                sqlite3_reset(stmt);
                iterate_tuple(statement.t.columns.columns, [&o, &index, &stmt, &impl, db](auto &m) {
                    using column_type = typename std::decay<decltype(m)>::type;
                    using field_type = typename column_result_t<self, column_type>::type;
                    const field_type *value = impl.table.template get_object_field_pointer<field_type>(o, m);
                    if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, *value)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    return sqlite3_last_insert_rowid(db);
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class It>
            void execute(const prepared_statement_t<replace_range_t<It>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_type::object_type;
                auto &impl = this->get_impl<object_type>();
                auto index = 1;
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                sqlite3_reset(stmt);
                for(auto it = statement.t.range.first; it != statement.t.range.second; ++it) {
                    auto &o = *it;
                    impl.table.for_each_column([&o, &index, &stmt, db](auto &c) {
                        using column_type = typename std::decay<decltype(c)>::type;
                        using field_type = typename column_type::field_type;
                        if(c.member_pointer) {
                            if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer)) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        } else {
                            using getter_type = typename column_type::getter_type;
                            field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                            if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        }
                    });
                }
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class It>
            void execute(const prepared_statement_t<insert_range_t<It>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_type::object_type;
                auto index = 1;
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto &impl = this->get_impl<object_type>();
                sqlite3_reset(stmt);
                for(auto it = statement.t.range.first; it != statement.t.range.second; ++it) {
                    auto &o = *it;
                    impl.table.for_each_column([&o, &index, &stmt, db](auto &c) {
                        if(!c.template has<constraints::primary_key_t<>>()) {
                            using column_type = typename std::decay<decltype(c)>::type;
                            using field_type = typename column_type::field_type;
                            if(c.member_pointer) {
                                if(SQLITE_OK !=
                                   statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer)) {
                                    throw std::system_error(
                                        std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
                                }
                            } else {
                                using getter_type = typename column_type::getter_type;
                                field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                                if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                                    throw std::system_error(
                                        std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
                                }
                            }
                        }
                    });
                }
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T>
            void execute(const prepared_statement_t<replace_t<T>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                auto &o = get_object(statement.t);
                auto &impl = this->get_impl<object_type>();
                sqlite3_reset(stmt);
                impl.table.for_each_column([&o, &index, &stmt, db](auto &c) {
                    using column_type = typename std::decay<decltype(c)>::type;
                    using field_type = typename column_type::field_type;
                    if(c.member_pointer) {
                        if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer)) {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    } else {
                        using getter_type = typename column_type::getter_type;
                        field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                        if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T>
            int64 execute(const prepared_statement_t<insert_t<T>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;
                int64 res = 0;
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                auto &impl = this->get_impl<object_type>();
                auto &o = get_object(statement.t);
                auto compositeKeyColumnNames = impl.table.composite_key_columns_names();
                sqlite3_reset(stmt);
                impl.table.for_each_column([&o, &index, &stmt, &impl, &compositeKeyColumnNames, db](auto &c) {
                    if(impl.table._without_rowid || !c.template has<constraints::primary_key_t<>>()) {
                        auto it = std::find(compositeKeyColumnNames.begin(), compositeKeyColumnNames.end(), c.name);
                        if(it == compositeKeyColumnNames.end()) {
                            using column_type = typename std::decay<decltype(c)>::type;
                            using field_type = typename column_type::field_type;
                            if(c.member_pointer) {
                                if(SQLITE_OK !=
                                   statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer)) {
                                    throw std::system_error(
                                        std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
                                }
                            } else {
                                using getter_type = typename column_type::getter_type;
                                field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                                if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                                    throw std::system_error(
                                        std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                        sqlite3_errmsg(db));
                                }
                            }
                        }
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    res = sqlite3_last_insert_rowid(db);
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
                return res;
            }

            template<class T, class... Ids>
            void execute(const prepared_statement_t<remove_t<T, Ids...>> &statement) {
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t.ids, [stmt, &index, db](auto &v) {
                    using field_type = typename std::decay<decltype(v)>::type;
                    if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, v)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T>
            void execute(const prepared_statement_t<update_t<T>> &statement) {
                using statement_type = typename std::decay<decltype(statement)>::type;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;
                auto con = this->get_connection();
                auto db = con.get();
                auto &impl = this->get_impl<object_type>();
                auto stmt = statement.stmt;
                auto index = 1;
                auto &o = get_object(statement.t);
                sqlite3_reset(stmt);
                impl.table.for_each_column([&o, stmt, &index, db](auto &c) {
                    if(!c.template has<constraints::primary_key_t<>>()) {
                        using column_type = typename std::decay<decltype(c)>::type;
                        using field_type = typename column_type::field_type;
                        if(c.member_pointer) {
                            auto bind_res = statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer);
                            if(SQLITE_OK != bind_res) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        } else {
                            using getter_type = typename column_type::getter_type;
                            field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                            if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        }
                    }
                });
                impl.table.for_each_column([&o, stmt, &index, db](auto &c) {
                    if(c.template has<constraints::primary_key_t<>>()) {
                        using column_type = typename std::decay<decltype(c)>::type;
                        using field_type = typename column_type::field_type;
                        if(c.member_pointer) {
                            if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, o.*c.member_pointer)) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        } else {
                            using getter_type = typename column_type::getter_type;
                            field_value_holder<getter_type> valueHolder{((o).*(c.getter))()};
                            if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, valueHolder.value)) {
                                throw std::system_error(
                                    std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                    sqlite3_errmsg(db));
                            }
                        }
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Ids>
            std::unique_ptr<T> execute(const prepared_statement_t<get_pointer_t<T, Ids...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t.ids, [stmt, &index, db](auto &v) {
                    using field_type = typename std::decay<decltype(v)>::type;
                    if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, v)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                auto stepRes = sqlite3_step(stmt);
                switch(stepRes) {
                    case SQLITE_ROW: {
                        auto res = std::make_unique<T>();
                        index = 0;
                        impl.table.for_each_column([&index, &res, stmt](auto &c) {
                            using field_type = typename std::decay<decltype(c)>::type::field_type;
                            auto value = row_extractor<field_type>().extract(stmt, index++);
                            if(c.member_pointer) {
                                (*res).*c.member_pointer = std::move(value);
                            } else {
                                ((*res).*(c.setter))(std::move(value));
                            }
                        });
                        return res;
                    } break;
                    case SQLITE_DONE: {
                        return {};
                    } break;
                    default: {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                }
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Ids>
            std::optional<T> execute(const prepared_statement_t<get_optional_t<T, Ids...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t.ids, [stmt, &index, db](auto &v) {
                    using field_type = typename std::decay<decltype(v)>::type;
                    if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, v)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                auto stepRes = sqlite3_step(stmt);
                switch(stepRes) {
                    case SQLITE_ROW: {
                        auto res = std::make_optional<T>();
                        index = 0;
                        impl.table.for_each_column([&index, &res, stmt](auto &c) {
                            using field_type = typename std::decay<decltype(c)>::type::field_type;
                            auto value = row_extractor<field_type>().extract(stmt, index++);
                            if(c.member_pointer) {
                                (*res).*c.member_pointer = std::move(value);
                            } else {
                                ((*res).*(c.setter))(std::move(value));
                            }
                        });
                        return res;
                    } break;
                    case SQLITE_DONE: {
                        return {};
                    } break;
                    default: {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                }
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class T, class... Ids>
            T execute(const prepared_statement_t<get_t<T, Ids...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t.ids, [stmt, &index, db](auto &v) {
                    using field_type = typename std::decay<decltype(v)>::type;
                    if(SQLITE_OK != statement_binder<field_type>().bind(stmt, index++, v)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                auto stepRes = sqlite3_step(stmt);
                switch(stepRes) {
                    case SQLITE_ROW: {
                        T res;
                        index = 0;
                        impl.table.for_each_column([&index, &res, stmt](auto &c) {
                            using column_type = typename std::decay<decltype(c)>::type;
                            using field_type = typename column_type::field_type;
                            auto value = row_extractor<field_type>().extract(stmt, index++);
                            if(c.member_pointer) {
                                res.*c.member_pointer = std::move(value);
                            } else {
                                ((res).*(c.setter))(std::move(value));
                            }
                        });
                        return res;
                    } break;
                    case SQLITE_DONE: {
                        throw std::system_error(std::make_error_code(sqlite_orm::orm_error_code::not_found));
                    } break;
                    default: {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                }
            }

            template<class T, class... Args>
            void execute(const prepared_statement_t<remove_all_t<T, Args...>> &statement) {
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t.conditions, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class... Args, class... Wargs>
            void execute(const prepared_statement_t<update_all_t<set_t<Args...>, Wargs...>> &statement) {
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_tuple(statement.t.set.assigns, [&index, stmt, db](auto &setArg) {
                    iterate_ast(setArg, [&index, stmt, db](auto &node) {
                        using node_type = typename std::decay<decltype(node)>::type;
                        conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                        if(SQLITE_OK != binder(node)) {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    });
                });
                iterate_ast(statement.t.conditions, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                if(sqlite3_step(stmt) == SQLITE_DONE) {
                    //  done..
                } else {
                    throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                            sqlite3_errmsg(db));
                }
            }

            template<class T, class... Args, class R = typename column_result_t<self, T>::type>
            std::vector<R> execute(const prepared_statement_t<select_t<T, Args...>> &statement) {
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                std::vector<R> res;
                int stepRes;
                do {
                    stepRes = sqlite3_step(stmt);
                    switch(stepRes) {
                        case SQLITE_ROW: {
                            res.push_back(row_extractor<R>().extract(stmt, 0));
                        } break;
                        case SQLITE_DONE:
                            break;
                        default: {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                } while(stepRes != SQLITE_DONE);
                return res;
            }

            template<class T, class... Args>
            std::vector<T> execute(const prepared_statement_t<get_all_t<T, Args...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                std::vector<T> res;
                int stepRes;
                do {
                    stepRes = sqlite3_step(stmt);
                    switch(stepRes) {
                        case SQLITE_ROW: {
                            T obj;
                            auto index = 0;
                            impl.table.for_each_column([&index, &obj, stmt](auto &c) {
                                using field_type = typename std::decay<decltype(c)>::type::field_type;
                                auto value = row_extractor<field_type>().extract(stmt, index++);
                                if(c.member_pointer) {
                                    obj.*c.member_pointer = std::move(value);
                                } else {
                                    ((obj).*(c.setter))(std::move(value));
                                }
                            });
                            res.push_back(std::move(obj));
                        } break;
                        case SQLITE_DONE:
                            break;
                        default: {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                } while(stepRes != SQLITE_DONE);
                return res;
            }

            template<class T, class... Args>
            std::vector<std::unique_ptr<T>>
            execute(const prepared_statement_t<get_all_pointer_t<T, Args...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                std::vector<std::unique_ptr<T>> res;
                int stepRes;
                do {
                    stepRes = sqlite3_step(stmt);
                    switch(stepRes) {
                        case SQLITE_ROW: {
                            auto obj = std::make_unique<T>();
                            auto index = 0;
                            impl.table.for_each_column([&index, &obj, stmt](auto &c) {
                                using field_type = typename std::decay<decltype(c)>::type::field_type;
                                auto value = row_extractor<field_type>().extract(stmt, index++);
                                if(c.member_pointer) {
                                    (*obj).*c.member_pointer = std::move(value);
                                } else {
                                    ((*obj).*(c.setter))(std::move(value));
                                }
                            });
                            res.push_back(std::move(obj));
                        } break;
                        case SQLITE_DONE:
                            break;
                        default: {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                } while(stepRes != SQLITE_DONE);
                return res;
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Args>
            std::vector<std::optional<T>>
            execute(const prepared_statement_t<get_all_optional_t<T, Args...>> &statement) {
                auto &impl = this->get_impl<T>();
                auto con = this->get_connection();
                auto db = con.get();
                auto stmt = statement.stmt;
                auto index = 1;
                sqlite3_reset(stmt);
                iterate_ast(statement.t, [stmt, &index, db](auto &node) {
                    using node_type = typename std::decay<decltype(node)>::type;
                    conditional_binder<node_type, is_bindable<node_type>> binder{stmt, index};
                    if(SQLITE_OK != binder(node)) {
                        throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                sqlite3_errmsg(db));
                    }
                });
                std::vector<std::optional<T>> res;
                int stepRes;
                do {
                    stepRes = sqlite3_step(stmt);
                    switch(stepRes) {
                        case SQLITE_ROW: {
                            auto obj = std::make_optional<T>();
                            auto index = 0;
                            impl.table.for_each_column([&index, &obj, stmt](auto &c) {
                                using field_type = typename std::decay<decltype(c)>::type::field_type;
                                auto value = row_extractor<field_type>().extract(stmt, index++);
                                if(c.member_pointer) {
                                    (*obj).*c.member_pointer = std::move(value);
                                } else {
                                    ((*obj).*(c.setter))(std::move(value));
                                }
                            });
                            res.push_back(std::move(obj));
                        } break;
                        case SQLITE_DONE:
                            break;
                        default: {
                            throw std::system_error(std::error_code(sqlite3_errcode(db), get_sqlite_error_category()),
                                                    sqlite3_errmsg(db));
                        }
                    }
                } while(stepRes != SQLITE_DONE);
                return res;
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
        };  // struct storage_t

        template<class T>
        struct is_storage : std::false_type {};

        template<class... Ts>
        struct is_storage<storage_t<Ts...>> : std::true_type {};
    }

    template<class... Ts>
    internal::storage_t<Ts...> make_storage(const std::string &filename, Ts... tables) {
        return {filename, internal::storage_impl<Ts...>(tables...)};
    }

    /**
     *  sqlite3_threadsafe() interface.
     */
    inline int threadsafe() {
        return sqlite3_threadsafe();
    }
}
#pragma once

#if defined(_MSC_VER)
#if defined(__RESTORE_MIN__)
__pragma(pop_macro("min"))
#undef __RESTORE_MIN__
#endif
#if defined(__RESTORE_MAX__)
    __pragma(pop_macro("max"))
#undef __RESTORE_MAX__
#endif
#endif  // defined(_MSC_VER)
#pragma once

#include <tuple>  //  std::tuple
#include <utility>  //  std::pair
#include <functional>  //  std::reference_wrapper

    // #include "conditions.h"

    // #include "operators.h"

    // #include "select_constraints.h"

    // #include "prepared_statement.h"

    // #include "optional_container.h"

    // #include "core_functions.h"

    namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct node_tuple {
            using type = std::tuple<T>;
        };

        template<>
        struct node_tuple<void, void> {
            using type = std::tuple<>;
        };

        template<class T>
        struct node_tuple<std::reference_wrapper<T>, void> {
            using type = typename node_tuple<T>::type;
        };

        template<class C>
        struct node_tuple<conditions::where_t<C>, void> {
            using node_type = conditions::where_t<C>;
            using type = typename node_tuple<C>::type;
        };

        template<class T>
        struct node_tuple<T,
                          typename std::enable_if<is_base_of_template<T, conditions::binary_condition>::value>::type> {
            using node_type = T;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_node_tuple = typename node_tuple<left_type>::type;
            using right_node_tuple = typename node_tuple<right_type>::type;
            using type = typename conc_tuple<left_node_tuple, right_node_tuple>::type;
        };

        template<class L, class R, class... Ds>
        struct node_tuple<binary_operator<L, R, Ds...>, void> {
            using node_type = binary_operator<L, R, Ds...>;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_node_tuple = typename node_tuple<left_type>::type;
            using right_node_tuple = typename node_tuple<right_type>::type;
            using type = typename conc_tuple<left_node_tuple, right_node_tuple>::type;
        };

        template<class... Args>
        struct node_tuple<columns_t<Args...>, void> {
            using node_type = columns_t<Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };

        template<class L, class A>
        struct node_tuple<conditions::in_t<L, A>, void> {
            using node_type = conditions::in_t<L, A>;
            using left_tuple = typename node_tuple<L>::type;
            using right_tuple = typename node_tuple<A>::type;
            using type = typename conc_tuple<left_tuple, right_tuple>::type;
        };

        template<class T>
        struct node_tuple<T, typename std::enable_if<is_base_of_template<T, compound_operator>::value>::type> {
            using node_type = T;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_tuple = typename node_tuple<left_type>::type;
            using right_tuple = typename node_tuple<right_type>::type;
            using type = typename conc_tuple<left_tuple, right_tuple>::type;
        };

        template<class T, class... Args>
        struct node_tuple<select_t<T, Args...>, void> {
            using node_type = select_t<T, Args...>;
            using columns_tuple = typename node_tuple<T>::type;
            using args_tuple = typename conc_tuple<typename node_tuple<Args>::type...>::type;
            using type = typename conc_tuple<columns_tuple, args_tuple>::type;
        };

        template<class T, class... Args>
        struct node_tuple<get_all_t<T, Args...>, void> {
            using node_type = get_all_t<T, Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };

        template<class T, class... Args>
        struct node_tuple<get_all_pointer_t<T, Args...>, void> {
            using node_type = get_all_pointer_t<T, Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct node_tuple<get_all_optional_t<T, Args...>, void> {
            using node_type = get_all_optional_t<T, Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class... Args, class... Wargs>
        struct node_tuple<update_all_t<set_t<Args...>, Wargs...>, void> {
            using node_type = update_all_t<set_t<Args...>, Wargs...>;
            using set_tuple = typename conc_tuple<typename node_tuple<Args>::type...>::type;
            using conditions_tuple = typename conc_tuple<typename node_tuple<Wargs>::type...>::type;
            using type = typename conc_tuple<set_tuple, conditions_tuple>::type;
        };

        template<class T, class... Args>
        struct node_tuple<remove_all_t<T, Args...>, void> {
            using node_type = remove_all_t<T, Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };

        template<class T>
        struct node_tuple<conditions::having_t<T>, void> {
            using node_type = conditions::having_t<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class T, class E>
        struct node_tuple<conditions::cast_t<T, E>, void> {
            using node_type = conditions::cast_t<T, E>;
            using type = typename node_tuple<E>::type;
        };

        template<class T>
        struct node_tuple<conditions::exists_t<T>, void> {
            using node_type = conditions::exists_t<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class T>
        struct node_tuple<optional_container<T>, void> {
            using node_type = optional_container<T>;
            using type = typename node_tuple<T>::type;
        };

        template<>
        struct node_tuple<optional_container<void>, void> {
            using node_type = optional_container<void>;
            using type = std::tuple<>;
        };

        template<class A, class T, class E>
        struct node_tuple<conditions::like_t<A, T, E>, void> {
            using node_type = conditions::like_t<A, T, E>;
            using arg_tuple = typename node_tuple<A>::type;
            using pattern_tuple = typename node_tuple<T>::type;
            using escape_tuple = typename node_tuple<E>::type;
            using type = typename conc_tuple<arg_tuple, pattern_tuple, escape_tuple>::type;
        };

        template<class A, class T>
        struct node_tuple<conditions::glob_t<A, T>, void> {
            using node_type = conditions::glob_t<A, T>;
            using arg_tuple = typename node_tuple<A>::type;
            using pattern_tuple = typename node_tuple<T>::type;
            using type = typename conc_tuple<arg_tuple, pattern_tuple>::type;
        };

        template<class A, class T>
        struct node_tuple<conditions::between_t<A, T>, void> {
            using node_type = conditions::between_t<A, T>;
            using expression_tuple = typename node_tuple<A>::type;
            using lower_tuple = typename node_tuple<T>::type;
            using upper_tuple = typename node_tuple<T>::type;
            using type = typename conc_tuple<expression_tuple, lower_tuple, upper_tuple>::type;
        };

        template<class T>
        struct node_tuple<conditions::named_collate<T>, void> {
            using node_type = conditions::named_collate<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class T>
        struct node_tuple<conditions::is_null_t<T>, void> {
            using node_type = conditions::is_null_t<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class T>
        struct node_tuple<conditions::is_not_null_t<T>, void> {
            using node_type = conditions::is_not_null_t<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class C>
        struct node_tuple<conditions::negated_condition_t<C>, void> {
            using node_type = conditions::negated_condition_t<C>;
            using type = typename node_tuple<C>::type;
        };

        template<class R, class S, class... Args>
        struct node_tuple<core_functions::core_function_t<R, S, Args...>, void> {
            using node_type = core_functions::core_function_t<R, S, Args...>;
            using type = typename conc_tuple<typename node_tuple<Args>::type...>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::left_join_t<T, O>, void> {
            using node_type = conditions::left_join_t<T, O>;
            using type = typename node_tuple<O>::type;
        };

        template<class T>
        struct node_tuple<conditions::on_t<T>, void> {
            using node_type = conditions::on_t<T>;
            using type = typename node_tuple<T>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::join_t<T, O>, void> {
            using node_type = conditions::join_t<T, O>;
            using type = typename node_tuple<O>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::left_outer_join_t<T, O>, void> {
            using node_type = conditions::left_outer_join_t<T, O>;
            using type = typename node_tuple<O>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::inner_join_t<T, O>, void> {
            using node_type = conditions::inner_join_t<T, O>;
            using type = typename node_tuple<O>::type;
        };

        template<class R, class T, class E, class... Args>
        struct node_tuple<simple_case_t<R, T, E, Args...>, void> {
            using node_type = simple_case_t<R, T, E, Args...>;
            using case_tuple = typename node_tuple<T>::type;
            using args_tuple = typename conc_tuple<typename node_tuple<Args>::type...>::type;
            using else_tuple = typename node_tuple<E>::type;
            using type = typename conc_tuple<case_tuple, args_tuple, else_tuple>::type;
        };

        template<class L, class R>
        struct node_tuple<std::pair<L, R>, void> {
            using node_type = std::pair<L, R>;
            using left_tuple = typename node_tuple<L>::type;
            using right_tuple = typename node_tuple<R>::type;
            using type = typename conc_tuple<left_tuple, right_tuple>::type;
        };

        template<class T, class E>
        struct node_tuple<as_t<T, E>, void> {
            using node_type = as_t<T, E>;
            using type = typename node_tuple<E>::type;
        };

        template<class T>
        struct node_tuple<conditions::limit_t<T, false, false, void>, void> {
            using node_type = conditions::limit_t<T, false, false, void>;
            using type = typename node_tuple<T>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::limit_t<T, true, false, O>, void> {
            using node_type = conditions::limit_t<T, true, false, O>;
            using type = typename conc_tuple<typename node_tuple<T>::type, typename node_tuple<O>::type>::type;
        };

        template<class T, class O>
        struct node_tuple<conditions::limit_t<T, true, true, O>, void> {
            using node_type = conditions::limit_t<T, true, true, O>;
            using type = typename conc_tuple<typename node_tuple<O>::type, typename node_tuple<T>::type>::type;
        };
    }
}
#pragma once

#include <type_traits>  //  std::is_same, std::decay, std::remove_reference

// #include "prepared_statement.h"

// #include "ast_iterator.h"

// #include "static_magic.h"

// #include "expression_object_type.h"

namespace sqlite_orm {

    template<int N, class It>
    auto &get(internal::prepared_statement_t<internal::insert_range_t<It>> &statement) {
        return std::get<N>(statement.t.range);
    }

    template<int N, class It>
    const auto &get(const internal::prepared_statement_t<internal::insert_range_t<It>> &statement) {
        return std::get<N>(statement.t.range);
    }

    template<int N, class It>
    auto &get(internal::prepared_statement_t<internal::replace_range_t<It>> &statement) {
        return std::get<N>(statement.t.range);
    }

    template<int N, class It>
    const auto &get(const internal::prepared_statement_t<internal::replace_range_t<It>> &statement) {
        return std::get<N>(statement.t.range);
    }

    template<int N, class T, class... Ids>
    auto &get(internal::prepared_statement_t<internal::get_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T, class... Ids>
    const auto &get(const internal::prepared_statement_t<internal::get_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T, class... Ids>
    auto &get(internal::prepared_statement_t<internal::get_pointer_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T, class... Ids>
    const auto &get(const internal::prepared_statement_t<internal::get_pointer_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<int N, class T, class... Ids>
    auto &get(internal::prepared_statement_t<internal::get_optional_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T, class... Ids>
    const auto &get(const internal::prepared_statement_t<internal::get_optional_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

    template<int N, class T, class... Ids>
    auto &get(internal::prepared_statement_t<internal::remove_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T, class... Ids>
    const auto &get(const internal::prepared_statement_t<internal::remove_t<T, Ids...>> &statement) {
        return internal::get_ref(std::get<N>(statement.t.ids));
    }

    template<int N, class T>
    auto &get(internal::prepared_statement_t<internal::update_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for update statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    const auto &get(const internal::prepared_statement_t<internal::update_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for update statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T, class... Cols>
    auto &get(internal::prepared_statement_t<internal::insert_explicit<T, Cols...>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T, class... Cols>
    const auto &get(const internal::prepared_statement_t<internal::insert_explicit<T, Cols...>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    auto &get(internal::prepared_statement_t<internal::replace_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for replace statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    const auto &get(const internal::prepared_statement_t<internal::replace_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for replace statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    auto &get(internal::prepared_statement_t<internal::insert_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    const auto &get(const internal::prepared_statement_t<internal::insert_t<T>> &statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.t.obj);
    }

    template<int N, class T>
    const auto &get(const internal::prepared_statement_t<T> &statement) {
        using statement_type = typename std::decay<decltype(statement)>::type;
        using expression_type = typename statement_type::expression_type;
        using node_tuple = typename internal::node_tuple<expression_type>::type;
        using bind_tuple = typename internal::bindable_filter<node_tuple>::type;
        using result_tupe = typename std::tuple_element<N, bind_tuple>::type;
        const result_tupe *result = nullptr;
        auto index = -1;
        internal::iterate_ast(statement.t, [&result, &index](auto &node) {
            using node_type = typename std::decay<decltype(node)>::type;
            if(internal::is_bindable<node_type>::value) {
                ++index;
            }
            if(index == N) {
                internal::static_if<std::is_same<result_tupe, node_type>{}>([](auto &result, auto &node) {
                    result = const_cast<typename std::remove_reference<decltype(result)>::type>(&node);
                })(result, node);
            }
        });
        return internal::get_ref(*result);
    }

    template<int N, class T>
    auto &get(internal::prepared_statement_t<T> &statement) {
        using statement_type = typename std::decay<decltype(statement)>::type;
        using expression_type = typename statement_type::expression_type;
        using node_tuple = typename internal::node_tuple<expression_type>::type;
        using bind_tuple = typename internal::bindable_filter<node_tuple>::type;
        using result_tupe = typename std::tuple_element<N, bind_tuple>::type;
        result_tupe *result = nullptr;
        auto index = -1;
        internal::iterate_ast(statement.t, [&result, &index](auto &node) {
            using node_type = typename std::decay<decltype(node)>::type;
            if(internal::is_bindable<node_type>::value) {
                ++index;
            }
            if(index == N) {
                internal::static_if<std::is_same<result_tupe, node_type>{}>([](auto &result, auto &node) {
                    result = const_cast<typename std::remove_reference<decltype(result)>::type>(&node);
                })(result, node);
            }
        });
        return internal::get_ref(*result);
    }
}
#pragma once

#include <sstream>
#include <string>  //  std::string

// #include "core_functions.h"

// #include "constraints.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct statement_serializator {
            using statement_type = T;

            std::string operator()(const statement_type &t) const {
                std::stringstream ss;
                ss << t;
                return ss.str();
            }
        };

        template<class T>
        std::string serialize(const T &t) {
            statement_serializator<T> serializator;
            return serializator(t);
        }

        template<class R, class S, class... Args>
        struct statement_serializator<core_functions::core_function_t<R, S, Args...>, void> {
            using statement_type = core_functions::core_function_t<R, S, Args...>;

            std::string operator()(const statement_type &c) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << "(";
                std::vector<std::string> args;
                using args_type = typename std::decay<decltype(c)>::type::args_type;
                args.reserve(std::tuple_size<args_type>::value);
                iterate_tuple(c.args, [&args](auto &v) {
                    args.push_back(serialize(v));
                });
                for(size_t i = 0; i < args.size(); ++i) {
                    ss << args[i];
                    if(i < args.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << ")";
                return ss.str();
            }
        };

        template<>
        struct statement_serializator<constraints::autoincrement_t, void> {
            using statement_type = constraints::autoincrement_t;

            std::string operator()(const statement_type &c) const {
                return static_cast<std::string>(c);
            }
        };

        template<class... Cs>
        struct statement_serializator<constraints::primary_key_t<Cs...>, void> {
            using statement_type = constraints::primary_key_t<Cs...>;

            std::string operator()(const statement_type &c) const {
                return static_cast<std::string>(c);
            }
        };

        template<>
        struct statement_serializator<constraints::unique_t, void> {
            using statement_type = constraints::unique_t;

            std::string operator()(const statement_type &c) const {
                return static_cast<std::string>(c);
            }
        };

        template<>
        struct statement_serializator<constraints::collate_t, void> {
            using statement_type = constraints::collate_t;

            std::string operator()(const statement_type &c) const {
                return static_cast<std::string>(c);
            }
        };

        template<class T>
        struct statement_serializator<constraints::default_t<T>, void> {
            using statement_type = constraints::default_t<T>;

            std::string operator()(const statement_type &c) const {
                return static_cast<std::string>(c) + " (" + serialize(c.value) + ")";
            }
        };

        template<>
        struct statement_serializator<std::string, void> {
            using statement_type = std::string;

            std::string operator()(const statement_type &c) const {
                return "\"" + c + "\"";
            }
        };

        template<>
        struct statement_serializator<const char *, void> {
            using statement_type = const char *;

            std::string operator()(const char *c) const {
                return std::string("'") + c + "'";
            }
        };

    }
}
