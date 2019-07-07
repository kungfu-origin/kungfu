//
// Created by qlu on 2019/2/19.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "md_gateway.h"
#include "td_gateway.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::xtp;

PYBIND11_MODULE(kfext_xtp, m) {
    py::class_<MdGateway, kungfu::practice::apprentice, std::shared_ptr<MdGateway>>(m, "MD")
            .def(py::init<bool, locator_ptr, std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
            .def("run", &MdGateway::run);

    py::class_<TdGateway, kungfu::practice::apprentice, std::shared_ptr<TdGateway>>(m, "TD")
            .def(py::init<bool, locator_ptr, std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
            .def("run", &TdGateway::run);
}
