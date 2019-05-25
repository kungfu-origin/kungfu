//
// Created by qlu on 2019/3/18.
//


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include "util/journal_printer/journal_printer.h"

PYBIND11_MODULE(pyjournalprinter, m)
{
    py::class_<kungfu::JournalPrinter>(m, "JournalPrinter")
        .def(py::init<const std::string&, const std::string&, int, bool, bool, bool, bool>())
        .def("run", &kungfu::JournalPrinter::run, py::arg("start_time"), py::arg("end_time"), py::arg("keep"))
        ;
}
