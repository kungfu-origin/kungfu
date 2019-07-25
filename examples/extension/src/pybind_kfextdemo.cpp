#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "md_gateway.h"
#include "td_gateway.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::kfext_demo;

PYBIND11_MODULE(kfextdemo, m)
{
    py::class_<MdGateway>(m, "MD")
    .def(py::init<bool, locator_ptr, std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("run", &MdGateway::run);

    py::class_<TdGateway>(m, "TD")
    .def(py::init<bool, locator_ptr, std::map<std::string, std::string>&, std::map<std::string, int>&, std::map<std::string, double>&>())
    .def("run", &TdGateway::run);
}
