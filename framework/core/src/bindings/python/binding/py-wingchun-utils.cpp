#include "py-wingchun.h"

#include <kungfu/longfist/types.h>
#include <kungfu/wingchun/common.h>

using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;

namespace kungfu::wingchun::pybind {
void bind_utils(pybind11::module &m) {
  auto m_utils = m.def_submodule("utils");
  m_utils.def("hash_instrument", pybind11::overload_cast<const char *, const char *>(&hash_instrument));
  m_utils.def("is_valid_price", &is_valid_price);
  m_utils.def("is_final_status", &is_final_status);
  m_utils.def("get_instrument_type", &get_instrument_type);
  m_utils.def("order_from_input", [](const OrderInput &input) {
    Order order = {};
    order_from_input(input, order);
    return order;
  });
}
} // namespace kungfu::wingchun::pybind
