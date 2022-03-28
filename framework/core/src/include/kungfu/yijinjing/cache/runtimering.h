#ifndef KUNGFU_CACHE_RUNTIME_RING_H
#define KUNGFU_CACHE_RUNTIME_RING_H

#include <kungfu/longfist/longfist.h>

namespace kungfu::yijinjing::cache {
class order_bank {
public:
  template <typename DataType> void operator<<(const state<DataType> &state) {
    auto &target_queue = order_map_[boost::hana::type_c<DataType>];
    target_queue->push(state);
  }

  template <typename DataType> void operator<<(const typed_event_ptr<DataType> &event) {
    auto &target_queue = order_map_[boost::hana::type_c<DataType>];
    kungfu::state<DataType> s(*event);
    target_queue->push(s);
  }

  // void operator>>(const yijinjing::journal::writer_ptr &writer) const {
  //   boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
  //     auto type = boost::hana::second(it);
  //     using DataType = typename decltype(+type)::type;
  //     for (const auto &element : order_map_[type]) {
  //       writer->write(0, element.second.data);
  //     }
  //   });
  // }

  template <typename DataType>
  kungfu::yijinjing::cache::ringqueue<state<DataType>> &operator[](const boost::hana::basic_type<DataType> &type) {
    return *(order_map_[type]);
  }

  ~order_bank(){
    boost::hana::for_each(order_map_, [&](const auto& x) {
      SPDLOG_INFO(" ~order_bank {}", typeid(x).name());
    delete (+boost::hana::second(x));
});
  }

private:
  longfist::OrderMapType order_map_ = longfist::build_order_map(longfist::OrderDataTypes);
  // static  kungfu::yijinjing::cache::ringqueue<kungfu::state<longfist::types::OrderInput>> order_input_queue_(1024); 
  // decltype(boost::hana::make_map(std::declval<int,int>)) order_map_; 
  // = boost::hana::make_map(boost::hana::make_pair(longfist::types::OrderInput::tag,kungfu::yijinjing::cache::ringqueue<kungfu::state<longfist::types::OrderInput>>(1024)));
};
}
#endif