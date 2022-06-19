#include "py_wingchun.h"

namespace kungfu::wingchun {
void bind(pybind11::module &&m) {
  bind_book(m);
  bind_broker(m);
  bind_service(m);
  bind_strategy(m);
  bind_utils(m);
}
} // namespace kungfu::wingchun
