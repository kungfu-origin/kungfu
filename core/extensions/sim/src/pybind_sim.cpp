//
// Created by Keren Dong on 2019-07-11.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "marketdata_sim.h"
#include "trader_sim.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::sim;

PYBIND11_MODULE(kfext_sim, m)
{
    py::class_<MarketDataSim, kungfu::practice::apprentice, std::shared_ptr<MarketDataSim>>(m, "MD")
            .def(py::init<bool, locator_ptr, const std::string &>())
            .def("run", &MarketDataSim::run);

    py::class_<TraderSim, kungfu::practice::apprentice, std::shared_ptr<TraderSim>>(m, "TD")
            .def(py::init<bool, locator_ptr, const std::string &, const std::string &>())
            .def("run", &TraderSim::run);
}

