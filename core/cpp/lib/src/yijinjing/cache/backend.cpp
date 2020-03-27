//
// Created by Keren Dong on 2020/3/27.
//

#include <kungfu/yijinjing/cache/backend.h>

namespace kungfu::yijinjing::cache {
shift::shift(yijinjing::data::location_ptr location) : location_(std::move(location)), storage_map_() {}

shift::shift(const shift &copy) : location_(copy.location_), storage_map_(copy.storage_map_) {}

void shift::operator>>(const yijinjing::journal::writer_ptr &writer) {
  for (auto dest : location_->locator->list_location_dest(location_)) {
    ensure_storage(dest);
  }
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    for (auto &pair : storage_map_) {
      restore<DataType>(writer, pair.second);
    }
  });
}

void shift::ensure_storage(uint32_t dest) {
  if (storage_map_.find(dest) == storage_map_.end()) {
    auto locator = location_->locator;
    auto db_file = locator->layout_file(location_, longfist::enums::layout::SQLITE, fmt::format("{:08x}", dest));
    storage_map_.emplace(dest, make_storage(db_file, longfist::StateDataTypes));
    storage_map_.at(dest).sync_schema();
  }
}
} // namespace kungfu::yijinjing::cache
