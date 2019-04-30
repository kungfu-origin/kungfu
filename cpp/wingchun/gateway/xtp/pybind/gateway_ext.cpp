//
// Created by qlu on 2019/2/19.
//

#include "../md/md_gateway.h"
#include "../td/td_gateway.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(xtp, m)
{
py::class_<kungfu::xtp::MdGateway>(m, "MdGateway")
        .def("init", &kungfu::xtp::MdGateway::init)
        .def("start", &kungfu::xtp::MdGateway::start)
        .def("stop", &kungfu::xtp::MdGateway::stop)
        .def(py::init<int, const std::string&, const std::string&, int, const std::string&, const std::string&, int>());

py::class_<kungfu::xtp::TdGateway>(m, "TdGateway")
        .def("init", &kungfu::xtp::TdGateway::init)
        .def("start", &kungfu::xtp::TdGateway::start)
        .def("stop", &kungfu::xtp::TdGateway::stop)
        .def(py::init<int, const std::string&, const std::string&, const std::string&, int, const std::string&, const std::string&, int>());
}
