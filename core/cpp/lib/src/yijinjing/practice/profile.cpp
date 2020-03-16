//
// Created by Keren Dong on 2020/2/25.
//

#include <kungfu/yijinjing/practice/profile.h>

namespace kungfu::yijinjing::practice
{
    using namespace longfist;

    const std::string default_db_file(const yijinjing::data::locator_ptr &locator)
    {
        auto location = std::make_shared<yijinjing::data::location>(enums::mode::LIVE, enums::category::SYSTEM, "etc", "kungfu", locator);
        return locator->layout_file(location, enums::layout::SQLITE, "config");
    }

    profile::profile(const yijinjing::data::locator_ptr &locator) : profile(default_db_file(locator))
    {}

    profile::profile(const std::string &profile_db_file) : profile_db_file_(profile_db_file)
    {
        get_storage().sync_schema();
    }

    longfist::sqlite::ProfileStorageType &profile::get_storage()
    {
        static auto storage = longfist::sqlite::make_storage(profile_db_file_, ProfileDataTypes);
        return storage;
    }
}
