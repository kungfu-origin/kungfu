//
// Created by Keren Dong on 2020/2/25.
//

#ifndef KUNGFU_CONFIG_STORE_H
#define KUNGFU_CONFIG_STORE_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/cache/backend.h>
#include <kungfu/yijinjing/common.h>

namespace kungfu::yijinjing::practice {
class profile {
public:
  explicit profile(const yijinjing::data::locator_ptr &locator);

  void setup();

  template <typename DataType> void set(const DataType &data) {
    try {
      get_storage()->replace(data);
    } catch (const std::exception &e) {
      SPDLOG_ERROR("Unexpected exception by profile set {}", e.what());
    }
  }

  template <typename DataType> DataType get(const DataType &query) {
    using namespace boost::hana;
    auto pk_members = transform(DataType::primary_keys, [&](auto pk) {
      auto pk_member = find_if(accessors<DataType>(), [&](auto it) { return pk == first(it); });
      auto accessor = second(*pk_member);
      return accessor(query);
    });
    auto operation = [&](auto... ids) { return get_storage()->get<DataType>(ids...); };
    return unpack(pk_members, operation);
  }

  template <typename DataType> std::vector<DataType> get_all(const DataType &query) {
    return get_storage()->get_all<DataType>();
  }

  template <typename DataType> void remove(const DataType &query) {
    using namespace boost::hana;
    auto pk_members = transform(DataType::primary_keys, [&](auto pk) {
      auto just = find_if(accessors<DataType>(), [&](auto it) { return pk == first(it); });
      auto accessor = second(*just);
      return accessor(query);
    });
    auto operation = [&](auto... ids) { get_storage()->remove<DataType>(ids...); };
    unpack(pk_members, operation);
  }

  template <typename DataType> void remove_all() { get_storage()->remove_all<DataType>(); }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    get_storage()->replace(event->template data<DataType>());
  }

  template <typename DataType> void operator<<(const state<DataType> &s) { get_storage()->replace(s.data); }

private:
  const std::string profile_db_file_;

  explicit profile(std::string profile_db_file);

  yijinjing::cache::ProfileStoragePtr &get_storage();
};
} // namespace kungfu::yijinjing::practice

#endif // KUNGFU_CONFIG_STORE_H
