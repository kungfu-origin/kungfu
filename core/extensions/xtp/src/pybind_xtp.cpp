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

PYBIND11_MODULE(kfext_xtp, m) {
    py::class_<kungfu::xtp::MdGateway, std::shared_ptr<kungfu::xtp::MdGateway>>(m, "MD")
            .def(py::init<std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
            .def("get_name", &kungfu::xtp::MdGateway::get_name)
            .def("configure_event_source", &kungfu::xtp::MdGateway::configure_event_source)
            .def("handle", &kungfu::xtp::MdGateway::handle)
            .def("finish", &kungfu::xtp::MdGateway::finish);

    py::class_<kungfu::xtp::TdGateway, std::shared_ptr<kungfu::xtp::TdGateway>>(m, "TD")
            .def(py::init<std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
            .def("get_name", &kungfu::xtp::TdGateway::get_name)
            .def("configure_event_source", &kungfu::xtp::TdGateway::configure_event_source)
            .def("handle", &kungfu::xtp::TdGateway::handle)
            .def("finish", &kungfu::xtp::TdGateway::finish);
}
