#include "py_wingchun.h"

#include <kungfu/wingchun/service/bar.h>
#include <kungfu/wingchun/service/ledger.h>

namespace kungfu::wingchun {
namespace py = pybind11;

using namespace kungfu;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::service;

void bind_service(pybind11::module &m) {

  py::class_<Ledger, kungfu::yijinjing::practice::apprentice, std::shared_ptr<Ledger>>(m, "Ledger")
      .def(py::init<yijinjing::data::locator_ptr, longfist::enums::mode, bool>())
      .def_property_readonly("io_device", &Ledger::get_io_device)
      .def_property_readonly("usable", &Ledger::is_usable)
      .def_property_readonly("bookkeeper", &Ledger::get_bookkeeper, py::return_value_policy::reference)
      .def("set_begin_time", &Ledger::set_begin_time)
      .def("set_end_time", &Ledger::set_end_time)
      .def("now", &Ledger::now)
      .def("run", &Ledger::run)
      .def("on_exit", &Ledger::on_exit);

  py::class_<BarGenerator, kungfu::yijinjing::practice::apprentice, std::shared_ptr<BarGenerator>>(m, "BarGenerator")
      .def(py::init<yijinjing::data::locator_ptr, longfist::enums::mode, bool, std::string &>())
      .def("run", &service::BarGenerator::run);
}
} // namespace kungfu::wingchun