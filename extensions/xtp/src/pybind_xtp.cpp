//
// Created by qlu on 2019/2/19.
//

#include "md_gateway.h"
#include "td_gateway.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(kfext_xtp, m)
{
    py::class_<kungfu::xtp::MdGateway>(m, "MD")
    .def(py::init<std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("init", &kungfu::xtp::MdGateway::init)
    .def("start", &kungfu::xtp::MdGateway::start)
    .def("stop", &kungfu::xtp::MdGateway::stop)
    ;

    py::class_<kungfu::xtp::TdGateway>(m, "TD")
    .def(py::init<std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("init", &kungfu::xtp::TdGateway::init)
    .def("start", &kungfu::xtp::TdGateway::start)
    .def("stop", &kungfu::xtp::TdGateway::stop)
    ;
}
