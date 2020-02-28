//
// Created by Keren Dong on 2020/2/25.
//

#ifndef KUNGFU_CONFIG_STORE_H
#define KUNGFU_CONFIG_STORE_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/longfist/serialize/sql.h>

namespace kungfu::yijinjing::practice
{
    class config_store
    {
    public:
        explicit config_store(const yijinjing::data::locator_ptr &locator);

        template<typename DataType>
        void set(const DataType &data)
        {
            storage.replace(data);
            tables[boost::hana::type_c<DataType>][data.uid()] = data;
        }

        template<typename DataType>
        DataType &get(const DataType &query)
        {
            return tables[boost::hana::type_c<DataType>][query.uid()];
        }

        template<typename DataType>
        std::unordered_map<uint64_t, DataType> &get_all(const DataType &type)
        {
            return tables[boost::hana::type_c<DataType>];
        }

        template<typename DataType>
        void remove(const DataType &data)
        {
            auto pk_members = boost::hana::transform(DataType::primary_keys, [&](auto pk)
            {
                auto just = boost::hana::find_if(boost::hana::accessors<DataType>(), [&](auto it)
                {
                    return pk == boost::hana::first(it);
                });
                auto accessor = boost::hana::second(*just);
                return accessor(data);
            });
            auto operation = [&](auto... ids) {
                storage.remove<DataType>(ids...);
            };
            boost::hana::unpack(pk_members, operation);
            tables[boost::hana::type_c<DataType>].erase(data.uid());
        }

    private:
        longfist::sqlite::ConfigStorageType storage;
        longfist::ConfigMapType tables;

        explicit config_store(const std::string &config_db_file);
    };
}

#endif //KUNGFU_CONFIG_STORE_H
