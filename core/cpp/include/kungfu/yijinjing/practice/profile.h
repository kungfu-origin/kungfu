//
// Created by Keren Dong on 2020/2/25.
//

#ifndef KUNGFU_CONFIG_STORE_H
#define KUNGFU_CONFIG_STORE_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/longfist/serialize/sql.h>
#include <kungfu/yijinjing/common.h>

namespace kungfu::yijinjing::practice {
class profile {
public:
  explicit profile(const yijinjing::data::locator_ptr &locator);

  template <typename DataType> void set(const DataType &data) { get_storage().replace(data); }

  template <typename DataType> DataType get(const DataType &query) {
    auto pk_members = boost::hana::transform(DataType::primary_keys, [&](auto pk) {
      auto pk_member = boost::hana::find_if(boost::hana::accessors<DataType>(),
                                            [&](auto it) { return pk == boost::hana::first(it); });
      auto accessor = boost::hana::second(*pk_member);
      return accessor(query);
    });
    auto operation = [&](auto... ids) { return get_storage().get<DataType>(ids...); };
    return boost::hana::unpack(pk_members, operation);
  }

  template <typename DataType> std::vector<DataType> get_all(const DataType &query) {
    return get_storage().get_all<DataType>();
  }

  template <typename DataType> void remove(const DataType &query) {
    auto pk_members = boost::hana::transform(DataType::primary_keys, [&](auto pk) {
      auto just = boost::hana::find_if(boost::hana::accessors<DataType>(),
                                       [&](auto it) { return pk == boost::hana::first(it); });
      auto accessor = boost::hana::second(*just);
      return accessor(query);
    });
    auto operation = [&](auto... ids) { get_storage().remove<DataType>(ids...); };
    boost::hana::unpack(pk_members, operation);
  }

  template <typename DataType> void remove_all() { get_storage().remove_all<DataType>(); }

private:
  const std::string profile_db_file_;

  explicit profile(std::string profile_db_file);

  longfist::sqlite::ProfileStorageType &get_storage();
};
} // namespace kungfu::yijinjing::practice

#endif // KUNGFU_CONFIG_STORE_H
