#include "py-longfist.h"

namespace kungfu::longfist {
void bind(pybind11::module &&m) {
  pybind::bind_enums(m);
  pybind::bind_types(m);
}
} // namespace kungfu::longfist
