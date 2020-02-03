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

    struct Writer
    {
        template<typename DataType>
        void operator()(const char *type_name, boost::hana::basic_type<DataType> type, const event_ptr &event) const
        {
            SPDLOG_WARN("to sql {}", event->msg_type());
            auto data = event->data<DataType>();
        }
    };

    constexpr auto write = Writer{};
}

namespace sqlite_orm
{
    template<>
    struct type_printer<char *, void> : public text_printer {};

    template<>
    struct type_printer<int8_t *, void> : public blob_printer {};

    template<>
    struct type_printer<int16_t *, void> : public blob_printer {};

    template<>
    struct type_printer<int32_t *, void> : public blob_printer {};

    template<>
    struct type_printer<int64_t *, void> : public blob_printer {};

    template<>
    struct type_printer<double *, void> : public blob_printer {};

    template<>
    struct type_printer<kungfu::longfist::InstrumentType, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::ExecType, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::BsFlag, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::Side, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::Offset, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::HedgeFlag, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::OrderActionFlag, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::PriceType, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::VolumeCondition, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::TimeCondition, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::OrderStatus, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::Direction, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::AccountType, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::CommissionRateMode, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::LedgerCategory, void> : public integer_printer {};

    template<>
    struct type_printer<kungfu::longfist::BrokerState, void> : public integer_printer {};
}

#endif //KUNGFU_LONGFIST_SQL_H
