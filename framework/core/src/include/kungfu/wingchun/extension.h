//  SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2021/12/15.
//

#ifndef KUNGFU_EXTENSION_H
#define KUNGFU_EXTENSION_H

#include <pybind11/pybind11.h>

#define GET_MODULE_NAME_STR(N) #N
#define GET_MODULE_NAME(N) GET_MODULE_NAME_STR(N)

#define KUNGFU_EXTENSION(...) PYBIND11_MODULE(KUNGFU_MODULE_NAME, m)

#define KUNGFU_DEFINE_SERVICE(ServiceType)                                                                             \
  m.def("service",                                                                                                     \
        [&](kungfu::yijinjing::data::locator_ptr locator, kungfu::longfist::enums::mode m, bool low_latency = false) { \
          return std::static_pointer_cast<kungfu::yijinjing::practice::apprentice>(                                    \
              std::make_shared<ServiceType>(locator, m, low_latency));                                                 \
        })

#define KUNGFU_DEFINE_MD(MarketDataType)                                                                               \
  m.def("md", [&](kungfu::wingchun::broker::BrokerVendor &vendor) {                                                    \
    return std::static_pointer_cast<kungfu::wingchun::broker::MarketData>(std::make_shared<MarketDataType>(vendor));   \
  })

#define KUNGFU_DEFINE_TD(TraderType)                                                                                   \
  m.def("td", [&](kungfu::wingchun::broker::BrokerVendor &vendor) {                                                    \
    return std::static_pointer_cast<kungfu::wingchun::broker::Trader>(std::make_shared<TraderType>(vendor));           \
  })

#define KUNGFU_DEFINE_STRATEGY(StrategyType)                                                                           \
  m.def("strategy", [&]() {                                                                                            \
    return std::static_pointer_cast<kungfu::wingchun::strategy::Strategy>(std::make_shared<StrategyType>());           \
  })

#define KUNGFU_MAIN_STRATEGY(StrategyType)                                                                             \
  class StrategyType;                                                                                                  \
  PYBIND11_MODULE(KUNGFU_MODULE_NAME, m) {                                                                             \
    m.def("strategy", [&]() {                                                                                          \
      typedef kungfu::yijinjing::data::location location;                                                              \
      typedef kungfu::yijinjing::data::locator locator;                                                                \
      typedef kungfu::longfist::enums::mode mode;                                                                      \
      typedef kungfu::longfist::enums::category category;                                                              \
      std::string module_name = GET_MODULE_NAME(KUNGFU_MODULE_NAME);                                                   \
      auto lr = std::make_shared<locator>();                                                                           \
      std::string s_group(module_name);                                                                                \
      std::string s_name(module_name);                                                                                 \
      char *env_group = std::getenv("KF_STG_GROUP");                                                                   \
      if (env_group != NULL) {                                                                                         \
        s_group = env_group;                                                                                           \
      }                                                                                                                \
      char *env_name = std::getenv("KF_STG_NAME");                                                                     \
      if (env_name != NULL) {                                                                                          \
        s_name = env_name;                                                                                             \
      }                                                                                                                \
      auto home = location::make_shared(mode::LIVE, category::STRATEGY, s_group, s_name, lr);                          \
      kungfu::yijinjing::log::copy_log_settings(home, module_name);                                                    \
      return std::static_pointer_cast<kungfu::wingchun::strategy::Strategy>(std::make_shared<StrategyType>());         \
    });                                                                                                                \
  };                                                                                                                   \
  class StrategyType : public kungfu::wingchun::strategy::Strategy
#endif // KUNGFU_EXTENSION_H
