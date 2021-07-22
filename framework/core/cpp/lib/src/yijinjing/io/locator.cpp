/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by Keren Dong on 2021/5/21.
//

#include <cstdlib>
#include <kungfu/common.h>
#include <kungfu/yijinjing/common.h>
#include <regex>

namespace kungfu::yijinjing::data {

namespace fs = std::filesystem;
namespace es = longfist::enums;

std::string get_home() {
  auto kf_home = std::getenv("KF_HOME");
  if (kf_home != nullptr) {
    return kf_home;
  }
#ifdef _WINDOWS
  auto appdata = std::getenv("APPDATA");
  auto root = fs::path(appdata);
#elif __APPLE__
  auto user_home = std::getenv("HOME");
  auto root = fs::path(user_home) / "Library" / "Application Support";
#elif __linux__
  auto user_home = std::getenv("HOME");
  auto root = fs::path(user_home) / ".config";
#endif // _WINDOWS
  return (root / "kungfu" / "home" / "runtime").string();
}

locator::locator() : root_(get_home()) {}

bool locator::has_env(const std::string &name) const { return std::getenv(name.c_str()) != nullptr; }

std::string locator::get_env(const std::string &name) const { return std::getenv(name.c_str()); }

std::string locator::layout_dir(const location_ptr &location, es::layout layout) const {
  auto dir = root_ /                                     //
             es::get_category_name(location->category) / //
             location->group /                           //
             location->name /                            //
             es::get_layout_name(layout) /               //
             es::get_mode_name(location->mode);
  if (not fs::exists(dir)) {
    fs::create_directories(dir);
  }
  return dir.string();
}

std::string locator::layout_file(const location_ptr &location, es::layout layout, const std::string &name) const {
  auto path = fs::path(layout_dir(location, layout)) / fmt::format("{}.{}", name, es::get_layout_name(layout));
  return path.string();
}

std::string locator::default_to_system_db(const location_ptr &location, const std::string &name) const {
  auto sqlite_layout = es::layout::SQLITE;
  auto db_file = layout_file(location, sqlite_layout, name);
  if (not fs::exists(db_file)) {
    auto system_db_file = root_ /                                     //
                          es::get_category_name(location->category) / //
                          location->group /                           //
                          location->name /                            //
                          es::get_layout_name(sqlite_layout) /        //
                          es::get_mode_name(location->mode);
    fs::copy(system_db_file, db_file);
  }
  return db_file;
}

std::vector<uint32_t> locator::list_page_id(const location_ptr &location, uint32_t dest_id) const {
  std::vector<uint32_t> result = {};
  auto dest_id_str = fmt::format("{:08x}", dest_id);
  auto dir = fs::path(layout_dir(location, es::layout::JOURNAL));
  for (auto &it : fs::recursive_directory_iterator(dir)) {
    auto basename = it.path().stem();
    if (it.is_regular_file() and it.path().extension() == ".journal" and basename.stem() == dest_id_str) {
      auto index = std::atoi(basename.extension().string().c_str() + 1);
      result.push_back(index);
    }
  }
  return result;
}

static constexpr auto g = [](const std::string &pattern) { return fmt::format("({})", pattern); };

std::vector<location_ptr> locator::list_locations(const std::string &category, const std::string &group,
                                                  const std::string &name, const std::string &mode) const {
  fs::path search_path = root_ / g(category) / g(group) / g(name) / "journal" / g(mode);
  std::smatch match;
  std::regex search_regex(search_path.string());
  std::vector<location_ptr> result = {};
  for (auto &it : fs::recursive_directory_iterator(root_)) {
    auto path = it.path().string();
    if (it.is_directory() and std::regex_match(path, match, search_regex)) {
      auto l = location::make_shared(es::get_mode_by_name(match[4].str()),     //
                                     es::get_category_by_name(match[1].str()), //
                                     match[2].str(),                           //
                                     match[3].str(),                           //
                                     std::make_shared<locator>(root_.string()));
      result.push_back(l);
    }
  }
  return result;
}

std::vector<uint32_t> locator::list_location_dest(const location_ptr &location) const {
  std::unordered_set<uint32_t> set = {};
  auto dir = fs::path(layout_dir(location, es::layout::JOURNAL));
  for (auto &it : fs::recursive_directory_iterator(dir)) {
    auto basename = it.path().stem();
    if (it.is_regular_file() and it.path().extension() == ".journal") {
      set.emplace(std::stoul(basename.stem(), nullptr, 16));
    }
  }
  return std::vector<uint32_t>(set.begin(), set.end());
}
} // namespace kungfu::yijinjing::data