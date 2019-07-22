//
// Created by qliu on 2019/6/08.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "bar_handle.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun;

PYBIND11_MODULE(kfext_bar, m) {
    py::class_<BarHandle>(m, "bar")
            .def(py::init<location_ptr, const std::string, int, bool>(), py::arg("home"), py::arg("source"), py::arg("frequency"), py::arg("low_latency") = false)
            .def("run", &BarHandle::run);
}