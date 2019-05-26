#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "md_gateway.h"
#include "td_gateway.h"

namespace py = pybind11;

PYBIND11_MODULE(kfextdemo, m)
{
    py::class_<kungfu::kfext_demo::MdGateway>(m, "MD")
    .def(py::init<std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("init", &kungfu::kfext_demo::MdGateway::init)
    .def("start", &kungfu::kfext_demo::MdGateway::start)
    .def("stop", &kungfu::kfext_demo::MdGateway::stop)
    ;

    py::class_<kungfu::kfext_demo::TdGateway>(m, "TD")
    .def(py::init<std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("init", &kungfu::kfext_demo::TdGateway::init)
    .def("start", &kungfu::kfext_demo::TdGateway::start)
    .def("stop", &kungfu::kfext_demo::TdGateway::stop)
    ;
}
