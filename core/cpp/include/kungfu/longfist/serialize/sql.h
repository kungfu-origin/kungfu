//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_SQL_H
#define KUNGFU_LONGFIST_SQL_H

#include <sqlite_orm/sqlite_orm.h>
#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::longfist::sqlite
{
    constexpr auto make_storage = [](const std::string &state_db_file, const auto &types)
    {
        constexpr auto make_table = [](const auto &types)
        {
            return [&](auto key)
            {
                auto value = types[key];
                using DataType = typename decltype(+value)::type;
                auto columns = boost::hana::transform(boost::hana::accessors<DataType>(), [](auto it)
                {
                    auto name = boost::hana::first(it);
                    auto accessor = boost::hana::second(it);
                    // MSVC bug: can not use static call
                    return sqlite_orm::make_column(name.c_str(), member_pointer_trait<decltype(accessor)>().pointer());
                });
                constexpr auto named_table = [](const std::string &table_name)
                {
                    return [&](auto ...columns)
                    {
                        auto pk_members = boost::hana::transform(DataType::primary_keys, [](auto pk)
                        {
                            auto just = boost::hana::find_if(boost::hana::accessors<DataType>(), [&](auto it)
                            {
                                return pk == boost::hana::first(it);
                            });
                            auto accessor = boost::hana::second(*just);
                            return member_pointer_trait<decltype(accessor)>().pointer();
                        });
                        auto primary_keys = boost::hana::unpack(pk_members, [](auto ... keys)
                        {
                            return sqlite_orm::primary_key(keys...);
                        });
                        return sqlite_orm::make_table(table_name, columns..., primary_keys);
                    };
                };
                return boost::hana::unpack(columns, named_table(key.c_str()));
            };
        };

        auto tables = boost::hana::transform(boost::hana::keys(types), make_table(types));

        constexpr auto named_storage = [](const std::string &state_db_file)
        {
            return [&](auto ...tables)
            {
                return sqlite_orm::make_storage(state_db_file, tables...);
            };
        };
        return boost::hana::unpack(tables, named_storage(state_db_file));
    };

    using StorageType = decltype(make_storage(std::string(), DataTypes));

    struct sqlizer
    {
        const DataTypesT &types;
        StorageType storage;

        explicit sqlizer(const std::string &state_db_file) : types(DataTypes), storage(longfist::sqlite::make_storage(state_db_file, DataTypes))
        {}

        void restore(const yijinjing::journal::writer_ptr &writer)
        {
            boost::hana::for_each(types, [&](auto it)
            {
                using DataType = typename decltype(+boost::hana::second(it))::type;
                for (auto &data : storage.get_all<DataType>())
                {
                    writer->write(0, DataType::tag, data);
                }
            });
        }

        template<typename DataType>
        void operator()(const std::string &name, boost::hana::basic_type<DataType> type, const event_ptr &event)
        {
            storage.replace(event->data<DataType>());;;;;;;;;;;;;;;;;;
        }
    };

    DECLARE_PTR(sqlizer)
}

namespace sqlite_orm
{

    template<typename T>
    struct type_printer<T, std::enable_if_t<std::is_enum<T>::value && !std::is_convertible<T, int>::value>> : public integer_printer
    {
    };

    template<size_t N>
    struct type_printer<kungfu::array<char, N>> : public text_printer
    {
    };

    template<typename T, size_t N>
    struct type_printer<kungfu::array<T, N>> : public blob_printer
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

    template<size_t N>
    struct statement_binder<kungfu::array<char, N>>
    {
        int bind(sqlite3_stmt *stmt, int index, const kungfu::array<char, N> &value)
        {
            return sqlite3_bind_text(stmt, index, static_cast<const char *>(value.value), -1, SQLITE_TRANSIENT);
        }
    };

    template<typename V, size_t N>
    struct statement_binder<kungfu::array<V, N>>
    {
        int bind(sqlite3_stmt *stmt, int index, const kungfu::array<V, N> &value)
        {
            return sqlite3_bind_blob(stmt, index, static_cast<const void *>(value.value), sizeof(value.value), SQLITE_TRANSIENT);
        }
    };
}

namespace sqlite_orm
{
    template<typename V>
    struct row_extractor<V, std::enable_if_t<std::is_enum<V>::value && !std::is_convertible<V, int>::value && !std::is_same<V, journal_mode>::value>>
    {
        V extract(const char *row_value)
        {
            return static_cast<V>(atoi(row_value));
        }

        V extract(sqlite3_stmt *stmt, int columnIndex)
        {
            return static_cast<V>(sqlite3_column_int(stmt, columnIndex));
        }
    };

    template<size_t N>
    struct row_extractor<kungfu::array<char, N>>
    {
        kungfu::array<char, N> extract(const char *row_value)
        {
            return kungfu::array<char, N>{row_value};
        }

        kungfu::array<char, N> extract(sqlite3_stmt *stmt, int columnIndex)
        {
            return kungfu::array<char, N>{sqlite3_column_text(stmt, columnIndex)};
        }
    };

    template<typename V, size_t N>
    struct row_extractor<kungfu::array<V, N>>
    {
        kungfu::array<V, N> extract(const char *row_value)
        {
            return kungfu::array<V, N>{row_value};
        }

        kungfu::array<V, N> extract(sqlite3_stmt *stmt, int columnIndex)
        {
            return kungfu::array<V, N>{sqlite3_column_text(stmt, columnIndex)};
        }
    };
}

#endif //KUNGFU_LONGFIST_SQL_H
