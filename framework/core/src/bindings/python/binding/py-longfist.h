//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_LONGFIST_HPP
#define KUNGFU_PY_LONGFIST_HPP

#include <pybind11/pybind11.h>

namespace kungfu::longfist {

void bind(pybind11::module &&m);

namespace pybind {
void bind_types(pybind11::module &m);
void bind_enums(pybind11::module &m);
} // namespace pybind

} // namespace kungfu::longfist

#endif // KUNGFU_PY_LONGFIST_HPP
