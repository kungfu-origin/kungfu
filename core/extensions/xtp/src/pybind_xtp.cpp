//
// Created by qlu on 2019/2/19.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "marketdata_xtp.h"
#include "trader_xtp.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::xtp;

PYBIND11_MODULE(kfext_xtp, m)
{
    py::class_<MarketDataXTP, kungfu::practice::apprentice, std::shared_ptr<MarketDataXTP>>(m, "MD")
            .def(py::init<bool, locator_ptr, const std::string &>())
            .def("run", &MarketDataXTP::run, py::call_guard<py::gil_scoped_release>());

    py::class_<TraderXTP, kungfu::practice::apprentice, std::shared_ptr<TraderXTP>>(m, "TD")
            .def(py::init<bool, locator_ptr, const std::string &, const std::string &>())
            .def("run", &TraderXTP::run, py::call_guard<py::gil_scoped_release>());
}
