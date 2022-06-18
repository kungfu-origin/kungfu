//
// Created by Keren Dong on 2021/12/15.
//

#ifndef KUNGFU_EXTENSION_H
#define KUNGFU_EXTENSION_H

#include <pybind11/pybind11.h>

#define KUNGFU_EXTENSION(name) PYBIND11_MODULE(name, m)

#define KUNGFU_DEFINE_MD(MarketDataType)                                                                               \
  m.def("md", [&](kungfu::wingchun::broker::BrokerVendor &vendor) {                                                    \
    return std::static_pointer_cast<kungfu::wingchun::broker::MarketData>(std::make_shared<MarketDataType>(vendor));   \
  })

#define KUNGFU_DEFINE_TD(TraderType)                                                                                   \
  m.def("td", [&](kungfu::wingchun::broker::BrokerVendor &vendor) {                                                    \
    return std::static_pointer_cast<kungfu::wingchun::broker::Trader>(std::make_shared<TraderType>(vendor));           \
  })

#endif // KUNGFU_EXTENSION_H
