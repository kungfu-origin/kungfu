//
// Created by qlu on 2019/2/19.
//

#include "md_gateway.h"
#include "td_gateway.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(kfext_ctp, m)
{
    py::class_<kungfu::ctp::MdGateway>(m, "MD")
    .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&, int>())
    .def("init", &kungfu::ctp::MdGateway::init)
    .def("start", &kungfu::ctp::MdGateway::start)
    .def("stop", &kungfu::ctp::MdGateway::stop)
    ;

    py::class_<kungfu::ctp::TdGateway>(m, "TD")
    .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&, int>())
    .def("init", &kungfu::ctp::TdGateway::init)
    .def("start", &kungfu::ctp::TdGateway::start)
    .def("stop", &kungfu::ctp::TdGateway::stop)
    ;
}
