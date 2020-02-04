//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_SQL_H
#define KUNGFU_LONGFIST_SQL_H

#include <spdlog/spdlog.h>
#include <boost/hana.hpp>
#include <sqlite_orm/sqlite_orm.h>
#include <kungfu/common.h>
#include <kungfu/longfist.h>

namespace kungfu::longfist::sqlite
{

    constexpr auto make_storage = [](const std::string &state_db_file)
    {
        constexpr auto make_table = [](auto types)
        {
            return [=](auto key)
            {
                auto value = types[key];
                using DataType = typename decltype(+value)::type;
                auto columns = boost::hana::transform(boost::hana::accessors<DataType>(), [](auto it) -> decltype(auto)
                {
                    auto name = boost::hana::first(it);
                    auto accessor = boost::hana::second(it);
                    return sqlite_orm::make_column(name.c_str(), member_pointer_trait<decltype(accessor)>::pointer());
                });

                constexpr auto named_table = [](const std::string &table_name)
                {
                    return [&](auto ...columns)
                    {
                        return sqlite_orm::make_table(table_name, columns...);
                    };
                };
                return boost::hana::unpack(columns, named_table(key.c_str()));
            };
        };

        auto tables = boost::hana::transform(boost::hana::keys(types::DATA_STRUCTS), make_table(types::DATA_STRUCTS));

        constexpr auto named_storage = [](const std::string &state_db_file)
        {
            return [&](auto ...tables)
            {
                return sqlite_orm::make_storage(state_db_file, tables...);
            };
        };
        return boost::hana::unpack(tables, named_storage(state_db_file));
    };

    using StorageType = decltype(make_storage(""));
    DECLARE_PTR(StorageType)

    struct Writer
    {
        StorageType_ptr storage;

        explicit Writer(StorageType_ptr _storage) : storage(std::move(_storage))
        {}

        template<typename DataType>
        void operator()(const char *type_name, boost::hana::basic_type<DataType> type, const event_ptr &event) const
        {
            auto data = event->data<DataType>();
            storage->insert(event->data<DataType>());
        }
    };

    constexpr auto write = [](auto storage)
    {
        return Writer{storage};
    };
}

namespace sqlite_orm
{

    template<typename T>
    struct type_printer<T, std::enable_if_t<std::is_enum<T>::value && !std::is_convertible<T, int>::value>> : public integer_printer
    {
    };

    template<typename T>
    struct type_printer<T, std::enable_if_t<std::is_array<T>::value && std::is_same<std::decay_t<T>, char *>::value>> : public text_printer
    {
    };

    template<typename T>
    struct type_printer<T, std::enable_if_t<std::is_array<T>::value && !std::is_same<std::decay_t<T>, char *>::value>> : public blob_printer
    {
    };

}

namespace sqlite_orm
{
    template<typename V>
    struct statement_binder<V, std::enable_if_t<std::is_enum<V>::value && !std::is_convertible<V, int>::value>>
    {
        int bind(sqlite3_stmt *stmt, int index, const V &value)
        {
            return sqlite3_bind_int(stmt, index, static_cast<int>(value));
        }
    };

    template<typename V>
    struct statement_binder<V, std::enable_if_t<std::is_array<V>::value && std::is_same<std::decay_t<V>, char *>::value>>
    {
        int bind(sqlite3_stmt *stmt, int index, const V &value)
        {
            return sqlite3_bind_text(stmt, index, static_cast<const char *>(value), -1, SQLITE_TRANSIENT);
        }
    };

    template<typename V>
    struct statement_binder<V, std::enable_if_t<std::is_array<V>::value && !std::is_same<std::decay_t<V>, char *>::value>>
    {
        int bind(sqlite3_stmt *stmt, int index, const V &value)
        {
            return sqlite3_bind_blob(stmt, index, static_cast<const void *>(value), sizeof(V), SQLITE_TRANSIENT);
        }
    };
}

#endif //KUNGFU_LONGFIST_SQL_H
