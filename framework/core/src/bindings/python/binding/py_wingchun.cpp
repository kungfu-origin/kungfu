#include "py_wingchun.h"

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <kungfu/wingchun/book/book.h>

PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::CommissionMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::InstrumentMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::PositionMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::PositionDetailMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::OrderInputMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::OrderMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::TradeMap)

namespace kungfu::wingchun {
namespace py = pybind11;

using namespace kungfu;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::book;

void bind(pybind11::module &&m) {

  auto m_utils = m.def_submodule("utils");
  m_utils.def("hash_instrument", py::overload_cast<const char *, const char *>(&kungfu::wingchun::hash_instrument));
  m_utils.def("is_valid_price", &kungfu::wingchun::is_valid_price);
  m_utils.def("is_final_status", &kungfu::wingchun::is_final_status);
  m_utils.def("get_instrument_type", &kungfu::wingchun::get_instrument_type);
  m_utils.def("order_from_input", [](const OrderInput &input) {
    Order order = {};
    order_from_input(input, order);
    return order;
  });

  bind_book(m);
  bind_broker(m);
  bind_service(m);
  bind_strategy(m);
}
} // namespace kungfu::wingchun
