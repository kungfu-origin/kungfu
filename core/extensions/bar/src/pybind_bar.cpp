//
// Created by qliu on 2019/6/08.
//

#include <pybind11/pybind11.h>
#include <kungfu/yijinjing/io.h>
#include "bar.h"

namespace py = pybind11;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::bar;

PYBIND11_MODULE(kfext_bar, m) {
    py::class_<BarGenerator>(m, "MD")
        .def(py::init<bool, locator_ptr, const std::string &>())
        .def("run", &BarGenerator::run);
}