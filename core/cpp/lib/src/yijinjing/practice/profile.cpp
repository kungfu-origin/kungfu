//
// Created by Keren Dong on 2020/2/25.
//

#include <kungfu/yijinjing/practice/profile.h>

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing::data;

namespace kungfu::yijinjing::practice {

std::string default_db_file(const locator_ptr &locator) {
  auto config_location = std::make_shared<location>(mode::LIVE, category::SYSTEM, "etc", "kungfu", locator);
  return locator->layout_file(config_location, enums::layout::SQLITE, "config");
}

profile::profile(const yijinjing::data::locator_ptr &locator) : profile(default_db_file(locator)) {}

profile::profile(std::string profile_db_file) : profile_db_file_(std::move(profile_db_file)) {
  if (not get_storage()->sync_schema_simulate().empty()) {
    get_storage()->sync_schema();
  }
}

yijinjing::cache::ProfileStoragePtr &profile::get_storage() {
  static auto storage = yijinjing::cache::make_storage_ptr(profile_db_file_, ProfileDataTypes);
  return storage;
}
} // namespace kungfu::yijinjing::practice
