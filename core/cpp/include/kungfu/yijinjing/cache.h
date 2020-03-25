//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_SQL_H
#define KUNGFU_LONGFIST_SQL_H

#include <sqlite_orm/sqlite_orm.h>

#include <kungfu/longfist/longfist.h>
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
      constexpr auto named_table = [](const std::string &table_name, const auto &primary_keys) {
        return [&](auto... columns) { return sqlite_orm::make_table(table_name, columns..., primary_keys); };
      };
      return boost::hana::unpack(columns, named_table(key.c_str(), primary_keys));
    };
  };

  auto tables = boost::hana::transform(boost::hana::keys(types), make_table(types));

  constexpr auto named_storage = [](const std::string &state_db_file) {
    return [&](auto... tables) { return sqlite_orm::make_storage(state_db_file, tables...); };
  };

  return boost::hana::unpack(tables, named_storage(state_db_file));
};

using ProfileStorageType = decltype(make_storage(std::string(), longfist::ProfileDataTypes));
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

  explicit shift(yijinjing::data::location_ptr location) : location_(std::move(location)), storage_map_() {}

  shift(const shift &copy) : location_(copy.location_), storage_map_(copy.storage_map_) {}

  void operator>>(const yijinjing::journal::writer_ptr &writer) {
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

  void ensure_storage(uint32_t dest) {
    if (storage_map_.find(dest) == storage_map_.end()) {
      auto locator = location_->locator;
      auto db_file = locator->layout_file(location_, longfist::enums::layout::SQLITE, fmt::format("{:08x}", dest));
      storage_map_.emplace(dest, make_storage(db_file, longfist::StateDataTypes));
      storage_map_.at(dest).sync_schema();
    }
  }

  template <typename DataType>
  std::enable_if_t<not std::is_same_v<DataType, longfist::types::DailyAsset>>
  restore(const yijinjing::journal::writer_ptr &writer, StateStorageType &storage) {
    for (auto &data : time_spec<DataType>::get_all(storage, yijinjing::time::today_nano(), INT64_MAX)) {
      writer->write(0, data);
    }
  }

  template <typename DataType>
  std::enable_if_t<std::is_same_v<DataType, longfist::types::DailyAsset>>
  restore(const yijinjing::journal::writer_ptr &writer, StateStorageType &storage) {}
};
DECLARE_PTR(shift)

class bank {
public:
  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    state_map_[boost::hana::type_c<DataType>].emplace(event->template data<DataType>().uid(), *event);
  }

  void operator>>(const yijinjing::journal::writer_ptr &writer) const {
    boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
      auto type = boost::hana::second(it);
      using DataType = typename decltype(+type)::type;
      for (const auto &element : state_map_[type]) {
        writer->write(0, element.second.data);
      }
    });
  }

  template <typename DataType>
  const std::unordered_map<uint64_t, state<DataType>> &operator[](const boost::hana::basic_type<DataType> &type) const {
    return state_map_[type];
  }

private:
  longfist::StateMapType state_map_ = longfist::build_state_map(longfist::StateDataTypes);
};
} // namespace kungfu::yijinjing::cache

namespace sqlite_orm {

template <typename T> struct type_printer<T, std::enable_if_t<kungfu::is_enum_class_v<T>>> : public integer_printer {};

template <size_t N> struct type_printer<kungfu::array<char, N>> : public text_printer {};

template <typename T, size_t N> struct type_printer<kungfu::array<T, N>> : public blob_printer {};

template <typename T> struct type_printer<std::vector<T>> : public blob_printer {};
} // namespace sqlite_orm

namespace sqlite_orm {
template <typename V> struct statement_binder<V, std::enable_if_t<kungfu::is_enum_class_v<V>>> {
  int bind(sqlite3_stmt *stmt, int index, const V &value) {
    return sqlite3_bind_int(stmt, index, static_cast<int>(value));
  }
};

template <size_t N> struct statement_binder<kungfu::array<char, N>> {
  int bind(sqlite3_stmt *stmt, int index, const kungfu::array<char, N> &value) {
    return sqlite3_bind_text(stmt, index, static_cast<const char *>(value.value), -1, SQLITE_TRANSIENT);
  }
};

template <typename V, size_t N> struct statement_binder<kungfu::array<V, N>> {
  int bind(sqlite3_stmt *stmt, int index, const kungfu::array<V, N> &value) {
    return sqlite3_bind_blob(stmt, index, value, sizeof(value.value), SQLITE_TRANSIENT);
  }
};

template <typename V> struct statement_binder<std::vector<V>, std::enable_if_t<not std::is_same_v<V, char>>> {
  int bind(sqlite3_stmt *stmt, int index, const std::vector<V> &value) {
    if (value.size()) {
      int size = value.size() * sizeof(V);
      return sqlite3_bind_blob(stmt, index, (const void *)&value.front(), size, SQLITE_TRANSIENT);
    } else {
      return sqlite3_bind_blob(stmt, index, "", 0, SQLITE_TRANSIENT);
    }
  }
};
} // namespace sqlite_orm

namespace sqlite_orm {
template <typename V>
struct row_extractor<V, std::enable_if_t<kungfu::is_enum_class_v<V> and not std::is_same_v<V, journal_mode>>> {
  V extract(const char *row_value) { return static_cast<V>(atoi(row_value)); }

  V extract(sqlite3_stmt *stmt, int columnIndex) { return static_cast<V>(sqlite3_column_int(stmt, columnIndex)); }
};

template <size_t N> struct row_extractor<kungfu::array<char, N>> {
  kungfu::array<char, N> extract(const char *row_value) { return kungfu::array<char, N>{row_value}; }

  kungfu::array<char, N> extract(sqlite3_stmt *stmt, int columnIndex) {
    return kungfu::array<char, N>{sqlite3_column_text(stmt, columnIndex)};
  }
};

template <typename V, size_t N> struct row_extractor<kungfu::array<V, N>> {
  kungfu::array<V, N> extract(const char *row_value) { return kungfu::array<V, N>{row_value}; }

  kungfu::array<V, N> extract(sqlite3_stmt *stmt, int columnIndex) {
    return kungfu::array<V, N>{sqlite3_column_text(stmt, columnIndex)};
  }
};

template <typename V> struct row_extractor<std::vector<V>> {
  std::vector<V> extract(const char *row_value) {
    if (row_value) {
      auto len = ::strlen(row_value);
      return this->go(row_value, len);
    }
    return {};
  }

  std::vector<V> extract(sqlite3_stmt *stmt, int columnIndex) {
    auto bytes = static_cast<const char *>(sqlite3_column_blob(stmt, columnIndex));
    auto len = sqlite3_column_bytes(stmt, columnIndex);
    return this->go(bytes, len);
  }

protected:
  std::vector<V> go(const char *bytes, size_t len) {
    if (len) {
      std::vector<V> res;
      res.reserve(len / sizeof(V));
      std::copy(bytes, bytes + len, std::back_inserter(res));
      return res;
    }
    return {};
  }
};
} // namespace sqlite_orm

#endif // KUNGFU_LONGFIST_SQL_H
