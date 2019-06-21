//
// Created by qlu on 2019/2/19.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/io.h>

#include "md_gateway.h"
#include "td_gateway.h"

namespace py = pybind11;

using namespace kungfu::wingchun::xtp;

PYBIND11_MODULE(kfext_xtp, m) {
    py::class_<MdGateway, std::shared_ptr<MdGateway>>(m, "MD")
            .def(py::init<std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>());

    py::class_<TdGateway, std::shared_ptr<TdGateway>>(m, "TD")
            .def(py::init<std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>());
}
