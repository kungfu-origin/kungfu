//
// Created by Keren Dong on 2019-07-11.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "marketdata.h"
#include "trader.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::passive;

PYBIND11_MODULE(kfext_passive, m) {
py::class_<PassiveMarketData, kungfu::practice::apprentice, std::shared_ptr<PassiveMarketData>>(m, "MD")
.def(py::init<bool, locator_ptr, std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
.def("run", &PassiveMarketData::run);

py::class_<PassiveTrader, kungfu::practice::apprentice, std::shared_ptr<PassiveTrader>>(m, "TD")
.def(py::init<bool, locator_ptr, std::map<std::string, std::string> &, std::map<std::string, int> &, std::map<std::string, double> &>())
.def("run", &PassiveTrader::run);
}

