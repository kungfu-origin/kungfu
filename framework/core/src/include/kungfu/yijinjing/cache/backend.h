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
template <typename ValueType> std::enable_if_t<std::is_arithmetic_v<ValueType>, ValueType> make_default() { return 0; }

template <typename ValueType> std::enable_if_t<std::is_enum_v<ValueType>, int> make_default() { return 0; }

template <typename ValueType> std::enable_if_t<is_array_v<ValueType>, std::string> make_default() {
  return std::string();
}

template <typename ValueType>
std::enable_if_t<not is_numeric_v<ValueType> and not is_array_v<ValueType>, ValueType> make_default() {
  return ValueType();
}

constexpr auto make_storage_ptr = [](const std::string &db_file, const auto &types) {
  using namespace boost::hana;
  using namespace sqlite_orm;

  constexpr auto make_tables = [](const auto &types) {
    return [&](auto key) {
      using DataType = typename decltype(+types[key])::type;
      auto columns = transform(accessors<DataType>(), [](auto it) {
        auto name = first(it);
        auto accessor = second(it);
        auto member_pointer = member_pointer_trait<decltype(accessor)>().pointer();
        using MemberType = std::decay_t<decltype(accessor(DataType()))>;
        return make_column(name.c_str(), member_pointer, default_value(make_default<MemberType>()));
      });
      auto pk_members = transform(DataType::primary_keys, [](auto pk) {
        auto filter = [&](auto it) { return pk == first(it); };
        auto pk_member = find_if(accessors<DataType>(), filter);
        auto accessor = second(*pk_member);
        return member_pointer_trait<decltype(accessor)>().pointer();
      });
      auto make_primary_keys = [](auto... keys) { return primary_key(keys...); };
      auto primary_keys = unpack(pk_members, make_primary_keys);
      constexpr auto table_maker = [](const std::string &table_name, const auto &primary_keys) {
        return [&](auto... columns) { return make_table(table_name, columns..., primary_keys); };
      };
      return unpack(columns, table_maker(key.c_str(), primary_keys));
    };
  };
  constexpr auto storage_ptr_maker = [](const std::string &db_file) {
    return [&](auto... tables) {
      using storage_type = decltype(make_storage(db_file, tables...));
      auto storage_ptr = std::make_shared<storage_type>(make_storage(db_file, tables...));
      storage_ptr->busy_timeout(time_unit::MILLISECONDS_PER_SECOND);
      return storage_ptr;
    };
  };
  auto tables = transform(keys(types), make_tables(types));
  return unpack(tables, storage_ptr_maker(db_file));
};

using ProfileStoragePtr = decltype(make_storage_ptr(std::string(), longfist::ProfileDataTypes));
using SessionStoragePtr = decltype(make_storage_ptr(std::string(), longfist::SessionDataTypes));
using StateStoragePtr = decltype(make_storage_ptr(std::string(), longfist::StateDataTypes));

template <typename, typename = void, bool = true> struct time_spec;

template <typename DataType> struct time_spec<DataType, std::enable_if_t<not DataType::has_timestamp>> {
  static std::vector<DataType> get_all(StateStoragePtr &storage, int64_t from, int64_t to) {
    return storage->get_all<DataType>();
  };
};

template <typename DataType> struct time_spec<DataType, std::enable_if_t<DataType::has_timestamp>> {
  static std::vector<DataType> get_all(StateStoragePtr &storage, int64_t from, int64_t to) {
    using namespace boost::hana;
    using namespace sqlite_orm;
    auto comparator = [](auto it) { return DataType::timestamp_key.value() == boost::hana::first(it); };
    auto just = find_if(accessors<DataType>(), comparator);
    auto accessor = second(*just);
    auto ts = member_pointer_trait<decltype(accessor)>().pointer();
    return storage->get_all<DataType>(where(greater_or_equal(ts, from) and lesser_or_equal(ts, to)));
  };
};

class shift {
public:
  shift() = default;

  explicit shift(yijinjing::data::location_ptr location);

  shift(const shift &copy);

  void ensure_storage(uint32_t dest);

  template <typename TargetType> void operator>>(TargetType &target) {
    for (auto dest : location_->locator->list_location_dest_by_db(location_)) {
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
    storage_map_.at(event->dest())->replace(event->template data<DataType>());
  }

  template <typename DataType> void operator<<(const state<DataType> &s) {
    ensure_storage(s.dest);
    storage_map_.at(s.dest)->replace(s.data);
  }

  template <typename DataType> void operator-=(const typed_event_ptr<DataType> &event) {
    ensure_storage(event->dest());
    storage_map_.at(event->dest())->template remove_all<DataType>();
  }

  template <typename DataType> void operator/=(const typed_event_ptr<DataType> &event) {
    for (auto &pair : storage_map_) {
      pair.second->template remove_all<DataType>();
    }
  }

private:
  yijinjing::data::location_ptr location_;
  std::unordered_map<uint32_t, StateStoragePtr> storage_map_;

  template <typename DataType> void restore(yijinjing::journal::writer_ptr &writer, uint32_t dest, StateStoragePtr &storage) {
    for (auto &data : time_spec<DataType>::get_all(storage, yijinjing::time::today_start(), INT64_MAX)) {
      writer->write(0, data);
    }
  }

  template <typename DataType> void restore(yijinjing::cache::bank &bank, uint32_t dest, StateStoragePtr &storage) {
    auto from = yijinjing::time::today_start();
    for (auto &data : time_spec<DataType>::get_all(storage, from, INT64_MAX)) {
      bank << state(location_->uid, dest, from, data);
    }
  }
};
DECLARE_PTR(shift)
} // namespace kungfu::yijinjing::cache

#endif // KUNGFU_CACHE_BACKEND_H
