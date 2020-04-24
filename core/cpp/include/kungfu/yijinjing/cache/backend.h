//
// Created by Keren Dong on 2020/3/27.
//

#ifndef KUNGFU_CACHE_BACKEND_H
#define KUNGFU_CACHE_BACKEND_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/cache/sqlite_orm_ext.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::cache {
constexpr auto make_storage = [](const std::string &state_db_file, const auto &types) {
  constexpr auto make_table = [](const auto &types) {
    return [&](auto key) {
      using DataType = typename decltype(+types[key])::type;
      auto columns = boost::hana::transform(boost::hana::accessors<DataType>(), [](auto it) {
        auto name = boost::hana::first(it);
        auto accessor = boost::hana::second(it);
        // MSVC bug: can not use static call
        return sqlite_orm::make_column(name.c_str(), member_pointer_trait<decltype(accessor)>().pointer());
      });
      auto pk_members = boost::hana::transform(DataType::primary_keys, [](auto pk) {
        auto filter = [&](auto it) { return pk == boost::hana::first(it); };
        auto pk_member = boost::hana::find_if(boost::hana::accessors<DataType>(), filter);
        auto accessor = boost::hana::second(*pk_member);
        return member_pointer_trait<decltype(accessor)>().pointer();
      });
      auto make_primary_keys = [](auto... keys) { return sqlite_orm::primary_key(keys...); };
      auto primary_keys = boost::hana::unpack(pk_members, make_primary_keys);
      constexpr auto table_maker = [](const std::string &table_name, const auto &primary_keys) {
        return [&](auto... columns) { return sqlite_orm::make_table(table_name, columns..., primary_keys); };
      };
      return boost::hana::unpack(columns, table_maker(key.c_str(), primary_keys));
    };
  };
  constexpr auto storage_maker = [](const std::string &state_db_file) {
    return [&](auto... tables) {
      auto storage = sqlite_orm::make_storage(state_db_file, tables...);
      storage.busy_timeout(1000);
      return storage;
    };
  };
  auto tables = boost::hana::transform(boost::hana::keys(types), make_table(types));
  return boost::hana::unpack(tables, storage_maker(state_db_file));
};

using ProfileStorageType = decltype(make_storage(std::string(), longfist::ProfileDataTypes));
using SessionStorageType = decltype(make_storage(std::string(), longfist::SessionDataTypes));
using StateStorageType = decltype(make_storage(std::string(), longfist::StateDataTypes));

template <typename, typename = void, bool = true> struct time_spec;

template <typename DataType> struct time_spec<DataType, std::enable_if_t<not DataType::has_timestamp>> {
  static std::vector<DataType> get_all(StateStorageType &storage, int64_t from, int64_t to) {
    return storage.get_all<DataType>();
  };
};

template <typename DataType> struct time_spec<DataType, std::enable_if_t<DataType::has_timestamp>> {
  static std::vector<DataType> get_all(StateStorageType &storage, int64_t from, int64_t to) {
    using namespace sqlite_orm;
    auto just = boost::hana::find_if(boost::hana::accessors<DataType>(),
                                     [](auto it) { return DataType::timestamp_key.value() == boost::hana::first(it); });
    auto accessor = boost::hana::second(*just);
    auto ts = member_pointer_trait<decltype(accessor)>().pointer();
    return storage.get_all<DataType>(where(greater_or_equal(ts, from) and lesser_or_equal(ts, to)));
  };
};

class shift {
public:
  shift() = default;

  explicit shift(yijinjing::data::location_ptr location);

  shift(const shift &copy);

  template <typename TargetType> void operator>>(TargetType &target) {
    for (auto dest : location_->locator->list_location_dest(location_)) {
      ensure_storage(dest);
    }
    boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
      using DataType = typename decltype(+boost::hana::second(it))::type;
      for (auto &pair : storage_map_) {
        restore<DataType>(target, pair.first, pair.second);
      }
    });
  }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    ensure_storage(event->dest());
    storage_map_.at(event->dest()).replace(event->template data<DataType>());
  }

  template <typename DataType> void operator-=(const typed_event_ptr<DataType> &event) {
    ensure_storage(event->dest());
    storage_map_.at(event->dest()).template remove_all<DataType>();
  }

  template <typename DataType> void operator/=(const typed_event_ptr<DataType> &event) {
    for (auto &pair : storage_map_) {
      pair.second.template remove_all<DataType>();
    }
  }

private:
  yijinjing::data::location_ptr location_;
  std::unordered_map<uint32_t, StateStorageType> storage_map_;

  void ensure_storage(uint32_t dest);

  template <typename DataType>
  std::enable_if_t<not std::is_same_v<DataType, longfist::types::DailyAsset>>
  restore(yijinjing::journal::writer_ptr &writer, uint32_t dest, StateStorageType &storage) {
    for (auto &data : time_spec<DataType>::get_all(storage, yijinjing::time::today_start(), INT64_MAX)) {
      writer->write(0, data);
    }
  }

  template <typename DataType>
  std::enable_if_t<std::is_same_v<DataType, longfist::types::DailyAsset>>
  restore(yijinjing::journal::writer_ptr &writer, uint32_t dest, StateStorageType &storage) {}

  template <typename DataType> void restore(yijinjing::cache::bank &bank, uint32_t dest, StateStorageType &storage) {
    auto from = yijinjing::time::today_start();
    for (auto &data : time_spec<DataType>::get_all(storage, from, INT64_MAX)) {
      bank << state(location_->uid, dest, from, data);
    }
  }
};
DECLARE_PTR(shift)
} // namespace kungfu::yijinjing::cache

#endif // KUNGFU_CACHE_BACKEND_H
