//
// Created by qlu on 2019/2/19.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "marketdata_ctp.h"
#include "trader_ctp.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::ctp;

PYBIND11_MODULE(kfext_ctp, m)
{
    py::class_<MarketDataCTP>(m, "MD")
    .def(py::init<bool, locator_ptr, const std::string &>())
    .def("run", &MarketDataCTP::run, py::call_guard<py::gil_scoped_release>())
    ;

    py::class_<TraderCTP>(m, "TD")
    .def(py::init<bool, locator_ptr, const std::string &, const std::string &>())
    .def("run", &TraderCTP::run, py::call_guard<py::gil_scoped_release>())
    ;
}
