//
// Created by Keren Dong on 2020/3/27.
//

#ifndef KUNGFU_CACHE_RUNTIME_H
#define KUNGFU_CACHE_RUNTIME_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::yijinjing::cache {
class bank {
public:
  template <typename DataType> void operator<<(const state<DataType> &state) {
    auto &target_map = state_map_[boost::hana::type_c<DataType>];
    target_map.insert_or_assign(state.data.uid(), state);
  }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    auto &target_map = state_map_[boost::hana::type_c<DataType>];
    target_map.insert_or_assign(event->template data<DataType>().uid(), *event);
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

template <typename DataTypes, typename DataTypesMap> class type_bank {
public:
  explicit type_bank(const DataTypes &types) : types_(types), state_map_(longfist::build_state_map(types_)) {}

  template <typename DataType> void operator<<(const state<DataType> &state) {
    auto &target_map = state_map_[boost::hana::type_c<DataType>];
    target_map.insert_or_assign(state.data.uid(), state);
  }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    auto &target_map = state_map_[boost::hana::type_c<DataType>];
    target_map.insert_or_assign(event->template data<DataType>().uid(), *event);
  }

  void operator>>(const yijinjing::journal::writer_ptr &writer) const {
    boost::hana::for_each(types_, [&](auto it) {
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
  DataTypes types_;
  DataTypesMap state_map_;
};

} // namespace kungfu::yijinjing::cache

#endif // KUNGFU_CACHE_RUNTIME_H
