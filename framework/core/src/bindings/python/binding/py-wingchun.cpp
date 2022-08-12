#include "py-wingchun.h"

namespace kungfu::wingchun {
void bind(pybind11::module &&m) {
  pybind::bind_book(m);
  pybind::bind_broker(m);
  pybind::bind_service(m);
  pybind::bind_strategy(m);
  pybind::bind_utils(m);
}
} // namespace kungfu::wingchun
