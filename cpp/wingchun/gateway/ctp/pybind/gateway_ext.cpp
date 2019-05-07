//
// Created by qlu on 2019/2/19.
//

#include "../md/md_gateway.h"
#include "../td/td_gateway.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(ctp, m)
{
py::class_<kungfu::ctp::MdGateway>(m, "MdGateway")
        .def("init", &kungfu::ctp::MdGateway::init)
        .def("start", &kungfu::ctp::MdGateway::start)
        .def("stop", &kungfu::ctp::MdGateway::stop)
        .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&, int>());

py::class_<kungfu::ctp::TdGateway>(m, "TdGateway")
        .def("init", &kungfu::ctp::TdGateway::init)
        .def("start", &kungfu::ctp::TdGateway::start)
        .def("stop", &kungfu::ctp::TdGateway::stop)
        .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&, int>());
}
