//
// Created by PolarAir on 2019-02-28.
//

#include "calendar/include/calendar_service.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(pycalendar, m)
{
py::class_<kungfu::CalendarService>(m, "CalendarService")
    .def(py::init<const std::string&>())

    .def("run", &kungfu::CalendarService::run)
    .def("join", &kungfu::CalendarService::join)
    .def("stop", &kungfu::CalendarService::stop)

    .def("current_day", &kungfu::CalendarService::get_current_day)
    .def("calculate_trading_day", &kungfu::CalendarService::calculate_trading_day)
    ;
}