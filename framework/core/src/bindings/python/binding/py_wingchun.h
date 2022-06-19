//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_WINGCHUN_HPP
#define KUNGFU_PY_WINGCHUN_HPP

#include <pybind11/pybind11.h>

namespace kungfu::wingchun {

void bind(pybind11::module &&m);

void bind_book(pybind11::module &m);

void bind_broker(pybind11::module &m);

void bind_service(pybind11::module &m);

void bind_strategy(pybind11::module &m);

} // namespace kungfu::wingchun

#endif // KUNGFU_PY_WINGCHUN_HPP
