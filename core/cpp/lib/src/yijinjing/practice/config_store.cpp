//
// Created by Keren Dong on 2020/2/25.
//

#include <kungfu/yijinjing/practice/config_store.h>

namespace kungfu::yijinjing::practice
{
    using namespace longfist;

    const std::string default_db_file(const yijinjing::data::locator_ptr &locator)
    {
        auto location = std::make_shared<yijinjing::data::location>(enums::mode::LIVE, enums::category::SYSTEM, "etc", "kungfu", locator);
        return locator->layout_file(location, enums::layout::SQLITE, "config");
    }

    config_store::config_store(const yijinjing::data::locator_ptr &locator) : config_store(default_db_file(locator))
    {}

    config_store::config_store(const std::string &config_db_file) : storage(longfist::sqlite::make_storage(config_db_file, ConfigDataTypes))
    {
        storage.sync_schema();
        boost::hana::for_each(ConfigDataTypes, [&](auto it)
        {
            using DataType = typename decltype(+boost::hana::second(it))::type;
            for (auto &data : storage.get_all<DataType>())
            {
                tables[boost::hana::type_c<DataType>][data.uid()] = data;
            }
        });
    }
}
