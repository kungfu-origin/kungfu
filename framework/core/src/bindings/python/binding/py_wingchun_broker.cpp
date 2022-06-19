#include "py_wingchun.h"

#include <pybind11/functional.h>

#include <kungfu/wingchun/broker/marketdata.h>
#include <kungfu/wingchun/broker/trader.h>

namespace kungfu::wingchun {
namespace py = pybind11;

using namespace kungfu;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::broker;

class PyBrokerVendor : public BrokerVendor {
public:
  using BrokerVendor::BrokerVendor;

protected:
  BrokerService_ptr get_service() override { PYBIND11_OVERLOAD_PURE(BrokerService_ptr, BrokerVendor, get_service); }
};

class PyMarketData : public MarketData {
public:
  using MarketData::MarketData;

  bool subscribe(const std::vector<InstrumentKey> &instruments) override {
    PYBIND11_OVERLOAD_PURE(bool, MarketData, subscribe, instruments);
  }

  bool subscribe_all() override { PYBIND11_OVERLOAD_PURE(bool, MarketData, subscribe_all); }

  bool unsubscribe(const std::vector<InstrumentKey> &instruments) override {
    PYBIND11_OVERLOAD_PURE(bool, MarketData, unsubscribe, instruments);
  }

  void on_start() override { PYBIND11_OVERLOAD(void, MarketData, on_start, ); }
};

class PyTrader : public Trader {
public:
  using Trader::Trader;

  [[nodiscard]] AccountType get_account_type() const override {
    PYBIND11_OVERLOAD_PURE(const AccountType, Trader, get_account_type, );
  }

  bool insert_order(const kungfu::event_ptr &event) override {
    PYBIND11_OVERLOAD_PURE(bool, Trader, insert_order, event);
  }

  bool cancel_order(const kungfu::event_ptr &event) override {
    PYBIND11_OVERLOAD_PURE(bool, Trader, cancel_order, event);
  }

  bool req_position() override { PYBIND11_OVERLOAD_PURE(bool, Trader, req_position, ); }

  bool req_account() override { PYBIND11_OVERLOAD_PURE(bool, Trader, req_account, ); }

  bool req_history_order(const event_ptr &event) override { PYBIND11_OVERLOAD(bool, Trader, req_history_order, event); }

  bool req_history_trade(const event_ptr &event) override { PYBIND11_OVERLOAD(bool, Trader, req_history_trade, event); }

  void on_start() override { PYBIND11_OVERLOAD(void, Trader, on_start, ); }

  void on_exit() override { PYBIND11_OVERLOAD(void, Trader, on_exit, ); }
};

void bind_broker(pybind11::module &m) {
  py::class_<BrokerVendor, PyBrokerVendor, std::shared_ptr<BrokerVendor>>(m, "BrokerVendor")
      .def(py::init<location_ptr, bool>())
      .def_property_readonly("home", &BrokerVendor::get_home)
      .def("run", &BrokerVendor::run);

  py::class_<MarketData, PyMarketData, std::shared_ptr<MarketData>>(m, "MarketData")
      .def(py::init<BrokerVendor &>())
      .def_property_readonly("state", &MarketData::get_state)
      .def_property_readonly("runtime_folder", &MarketData::get_runtime_folder)
      .def_property_readonly("config", &MarketData::get_config)
      .def_property_readonly("home", &MarketData::get_home)
      .def("on_start", &MarketData::on_start)
      .def("now", &MarketData::now)
      .def("get_writer", &MarketData::get_writer)
      .def("add_timer", &MarketData::add_timer)
      .def("add_time_interval", &MarketData::add_time_interval)
      .def("update_broker_state", &MarketData::update_broker_state)
      .def("subscribe", &MarketData::subscribe)
      .def("subscribe_all", &MarketData::subscribe_all)
      .def("unsubscribe", &MarketData::unsubscribe);

  py::class_<Trader, PyTrader, std::shared_ptr<Trader>>(m, "Trader")
      .def(py::init<BrokerVendor &>())
      .def_property_readonly("state", &Trader::get_state)
      .def_property_readonly("runtime_folder", &Trader::get_runtime_folder)
      .def_property_readonly("config", &Trader::get_config)
      .def_property_readonly("home", &Trader::get_home)
      .def("on_start", &Trader::on_start)
      .def("now", &Trader::now)
      .def("get_writer", &Trader::get_writer)
      .def("get_asset_writer", &Trader::get_asset_writer)
      .def("get_position_writer", &Trader::get_position_writer)
      .def("enable_asset_sync", &Trader::enable_asset_sync)
      .def("enable_positions_sync", &Trader::enable_positions_sync)
      .def("get_account_type", &Trader::get_account_type)
      .def("add_timer", &Trader::add_timer)
      .def("add_time_interval", &Trader::add_time_interval)
      .def("update_broker_state", &Trader::update_broker_state)
      .def("insert_order", &Trader::insert_order)
      .def("cancel_order", &Trader::cancel_order)
      .def("req_history_order", &Trader::req_history_order)
      .def("req_history_trade", &Trader::req_history_trade);

  py::class_<MarketDataVendor, BrokerVendor, std::shared_ptr<MarketDataVendor>>(m, "MarketDataVendor")
      .def(py::init<locator_ptr, const std::string &, const std::string &, bool>())
      .def("set_service", &MarketDataVendor::set_service);

  py::class_<TraderVendor, BrokerVendor, std::shared_ptr<TraderVendor>>(m, "TraderVendor")
      .def(py::init<locator_ptr, const std::string &, const std::string &, bool>())
      .def("set_service", &TraderVendor::set_service);
}
} // namespace kungfu::wingchun