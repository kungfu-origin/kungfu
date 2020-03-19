//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_JOURNAL_H
#define KUNGFU_LONGFIST_JOURNAL_H

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::longfist::journal {
class publisher {
public:
  explicit publisher(const StateMapType &state_map) : types(StateDataTypes), state_map_(state_map) {}

  void operator()(const yijinjing::journal::writer_ptr &writer, int64_t trigger_time) {
    boost::hana::for_each(types, [&](auto it) {
      auto type = boost::hana::second(it);
      using DataType = typename decltype(+type)::type;
      for (auto element : state_map_[type]) {
        writer->write(trigger_time, element.second.data);
      }
    });
  }

private:
  const StateDataTypesT &types;
  const StateMapType &state_map_;
};
} // namespace kungfu::longfist::journal

#endif // KUNGFU_LONGFIST_JOURNAL_H