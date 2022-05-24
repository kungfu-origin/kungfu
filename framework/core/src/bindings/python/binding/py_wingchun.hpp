//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_WINGCHUN_HPP
#define KUNGFU_PY_WINGCHUN_HPP

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/marketdata.h>
#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/wingchun/service/bar.h>
#include <kungfu/wingchun/service/cached.h>
#include <kungfu/wingchun/service/ledger.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runner.h>

PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::CommissionMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::InstrumentMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::PositionMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::PositionDetailMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::OrderInputMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::OrderMap)
PYBIND11_MAKE_OPAQUE(kungfu::wingchun::book::TradeMap)

namespace kungfu::wingchun {
namespace py = pybind11;

using namespace kungfu;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::book;
using namespace kungfu::wingchun::broker;
using namespace kungfu::wingchun::service;

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

class PyAccountingMethod : public AccountingMethod {
public:
  using AccountingMethod::AccountingMethod;

  void apply_trading_day(Book_ptr &book, int64_t trading_day) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, apply_trading_day, book, trading_day);
  }

  void apply_quote(Book_ptr &book, const Quote &quote) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, apply_quote, book, quote);
  }

  void apply_order_input(Book_ptr &book, const OrderInput &input) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, apply_order_input, book, input);
  }

  void apply_order(Book_ptr &book, const Order &order) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, apply_order, book, order);
  }

  void apply_trade(Book_ptr &book, const Trade &trade) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, apply_trade, book, trade);
  }

  void update_position(Book_ptr &book, Position &position) override {
    PYBIND11_OVERLOAD_PURE(void, AccountingMethod, update_position, book, position);
  }
};

class PyRunner : public strategy::Runner {
public:
  using strategy::Runner::Runner;

  void on_trading_day(const event_ptr &event, int64_t daytime) override {
    PYBIND11_OVERLOAD(void, strategy::Runner, on_trading_day, event, daytime);
  }
};

class PyStrategy : public strategy::Strategy {
public:
  using strategy::Strategy::Strategy; // Inherit constructors

  void pre_start(strategy::Context_ptr &context) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, pre_start, context);
  }

  void post_start(strategy::Context_ptr &context) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, post_start, context);
  }

  void pre_stop(strategy::Context_ptr &context) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, pre_stop, context);
  }

  void post_stop(strategy::Context_ptr &context) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, post_stop, context);
  }

  void on_trading_day(strategy::Context_ptr &context, int64_t daytime) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_trading_day, context, daytime);
  }

  void on_quote(strategy::Context_ptr &context, const Quote &quote) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_quote, context, quote);
  }

  void on_bar(strategy::Context_ptr &context, const Bar &bar) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_bar, context, bar);
  }

  void on_top_of_book(strategy::Context_ptr &context, const TopOfBook &top_of_book) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_top_of_book, context, top_of_book);
  }

  void on_entrust(strategy::Context_ptr &context, const Entrust &entrust) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_entrust, context, entrust);
  }

  void on_transaction(strategy::Context_ptr &context, const Transaction &transaction) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_transaction, context, transaction);
  }

  void on_order(strategy::Context_ptr &context, const Order &order) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_order, context, order);
  }

  void on_order_action_error(strategy::Context_ptr &context, const OrderActionError &error) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_order_action_error, context, error);
  }

  void on_trade(strategy::Context_ptr &context, const Trade &trade) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_trade, context, trade);
  }

  void on_history_order(strategy::Context_ptr &context, const HistoryOrder &history_order) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy,

                      on_history_order, context, history_order);
  }
  void on_history_trade(strategy::Context_ptr &context, const HistoryTrade &history_trade) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_history_trade, context, history_trade);
  }

  void on_book_sync_reset(strategy::Context_ptr &context, const Book &old_book, const Book &new_book) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_book_sync_reset, context, old_book, new_book);
  }

  void on_asset_sync_reset(strategy::Context_ptr &context, const Asset &old_asset, const Asset &new_asset) override {
    PYBIND11_OVERLOAD(void, strategy::Strategy, on_asset_sync_reset, context, old_asset, new_asset);
  }
};

void bind(pybind11::module &&m) {
  py::bind_map<CommissionMap>(m, "CommissionMap");
  py::bind_map<InstrumentMap>(m, "InstrumentMap");
  py::bind_map<PositionMap>(m, "PositionMap");
  py::bind_map<PositionDetailMap>(m, "PositionDetailMap");
  py::bind_map<OrderInputMap>(m, "OrderInputMap");
  py::bind_map<OrderMap>(m, "OrderMap");
  py::bind_map<TradeMap>(m, "TradeMap");

  auto m_utils = m.def_submodule("utils");
  m_utils.def("hash_instrument", py::overload_cast<const char *, const char *>(&kungfu::wingchun::hash_instrument));
  m_utils.def("is_valid_price", &kungfu::wingchun::is_valid_price);
  m_utils.def("is_final_status", &kungfu::wingchun::is_final_status);
  m_utils.def("get_instrument_type", &kungfu::wingchun::get_instrument_type);
  m_utils.def("order_from_input", [](const OrderInput &input) {
    Order order = {};
    order_from_input(input, order);
    return order;
  });

  py::class_<Book, Book_ptr>(m, "Book")
      .def_readonly("asset", &Book::asset, py::return_value_policy::reference)
      .def_readonly("long_positions", &Book::long_positions, py::return_value_policy::reference)
      .def_readonly("short_positions", &Book::short_positions, py::return_value_policy::reference)
      .def_readonly("order_inputs", &Book::order_inputs, py::return_value_policy::reference)
      .def_readonly("orders", &Book::orders, py::return_value_policy::reference)
      .def_readonly("trades", &Book::trades, py::return_value_policy::reference)
      .def("update", &Book::update)
      .def("has_long_position", &Book::has_long_position)
      .def("has_short_position", &Book::has_short_position)
      .def("has_position", &Book::has_position)
      .def("get_long_position", &Book::get_long_position)
      .def("get_short_position", &Book::get_short_position)
      .def("get_position", &Book::get_position)
      .def("has_position_for", py::overload_cast<const Quote &>(&Book::has_position_for<Quote>, py::const_))
      .def("has_position_for", py::overload_cast<const OrderInput &>(&Book::has_position_for<OrderInput>, py::const_))
      .def("has_position_for", py::overload_cast<const Order &>(&Book::has_position_for<Order>, py::const_))
      .def("has_position_for", py::overload_cast<const Trade &>(&Book::has_position_for<Trade>, py::const_))
      .def("get_position_for", py::overload_cast<Direction, const Quote &>(&Book::get_position_for<Quote>),
           py::return_value_policy::reference)
      .def("get_position_for", py::overload_cast<const OrderInput &>(&Book::get_position_for<OrderInput>),
           py::return_value_policy::reference)
      .def("get_position_for", py::overload_cast<const Order &>(&Book::get_position_for<Order>),
           py::return_value_policy::reference)
      .def("get_position_for", py::overload_cast<const Trade &>(&Book::get_position_for<Trade>),
           py::return_value_policy::reference);

  py::class_<AccountingMethod, PyAccountingMethod, AccountingMethod_ptr>(m, "AccountingMethod")
      .def(py::init<>())
      .def("apply_trading_day", &AccountingMethod::apply_trading_day)
      .def("apply_quote", &AccountingMethod::apply_quote)
      .def("apply_order_input", &AccountingMethod::apply_order_input)
      .def("apply_order", &AccountingMethod::apply_order)
      .def("apply_trade", &AccountingMethod::apply_trade);

  py::class_<Bookkeeper, std::shared_ptr<Bookkeeper>>(m, "Bookkeeper")
      .def("has_book", &Bookkeeper::has_book)
      .def("get_book", &Bookkeeper::get_book)
      .def("get_books", &Bookkeeper::get_books)
      .def("set_accounting_method", &Bookkeeper::set_accounting_method)
      .def("on_trading_day", &Bookkeeper::on_trading_day);

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

  py::class_<CacheD, kungfu::yijinjing::practice::apprentice, std::shared_ptr<CacheD>>(m, "CacheD")
      .def(py::init<yijinjing::data::locator_ptr, longfist::enums::mode, bool>())
      .def_property_readonly("io_device", &CacheD::get_io_device)
      .def_property_readonly("usable", &CacheD::is_usable)
      .def("set_begin_time", &CacheD::set_begin_time)
      .def("set_end_time", &CacheD::set_end_time)
      .def("now", &CacheD::now)
      .def("run", &CacheD::run)
      .def("on_exit", &CacheD::on_exit);

  py::class_<strategy::Runner, PyRunner, kungfu::yijinjing::practice::apprentice, std::shared_ptr<strategy::Runner>>(
      m, "Runner")
      .def(py::init<kungfu::yijinjing::data::locator_ptr, const std::string &, const std::string &,
                    longfist::enums::mode, bool>())
      .def_property_readonly("context", &strategy::Runner::get_context)
      .def("set_begin_time", &strategy::Runner::set_begin_time)
      .def("set_end_time", &strategy::Runner::set_end_time)
      .def("now", &strategy::Runner::now)
      .def("run", &strategy::Runner::run)
      .def("setup", &strategy::Runner::setup)
      .def("step", &strategy::Runner::step)
      .def("on_trading_day", &strategy::Runner::on_trading_day)
      .def("on_exit", &strategy::Runner::on_exit)
      .def("add_strategy", &strategy::Runner::add_strategy);

  py::class_<strategy::Context, std::shared_ptr<strategy::Context>>(m, "Context")
      .def_property_readonly("trading_day", &strategy::Context::get_trading_day)
      .def("now", &strategy::Context::now)
      .def("add_timer", &strategy::Context::add_timer)
      .def("add_time_interval", &strategy::Context::add_time_interval)
      .def("add_account", &strategy::Context::add_account)
      .def("get_account_cash_limit", &strategy::Context::get_account_cash_limit)
      .def("subscribe", &strategy::Context::subscribe)
      .def("subscribe_all", &strategy::Context::subscribe_all)
      .def("insert_order", &strategy::Context::insert_order, py::arg("symbol"), py::arg("exchange"), py::arg("account"),
           py::arg("limit_price"), py::arg("volume"), py::arg("type"), py::arg("side"),
           py::arg("offset") = Offset::Open, py::arg("hedge_flag") = HedgeFlag::Speculation)
      .def("cancel_order", &strategy::Context::cancel_order)
      .def("req_history_order", &strategy::Context::req_history_order)
      .def("req_history_trade", &strategy::Context::req_history_trade)
      .def("hold_book", &strategy::Context::hold_book)
      .def("hold_positions", &strategy::Context::hold_positions)
      .def("is_book_held", &strategy::Context::is_book_held)
      .def("is_positions_mirrored", &strategy::Context::is_positions_mirrored)
      .def("req_deregister", &strategy::Context::req_deregister)
      .def("update_strategy_state", &strategy::Context::update_strategy_state);

  py::class_<strategy::RuntimeContext, strategy::Context, strategy::RuntimeContext_ptr>(m, "RuntimeContext")
      .def_property_readonly("bookkeeper", &strategy::RuntimeContext::get_bookkeeper,
                             py::return_value_policy::reference);

  py::class_<strategy::Strategy, PyStrategy, strategy::Strategy_ptr>(m, "Strategy")
      .def(py::init())
      .def("pre_start", &strategy::Strategy::pre_start)
      .def("post_start", &strategy::Strategy::post_start)
      .def("pre_stop", &strategy::Strategy::pre_stop)
      .def("post_stop", &strategy::Strategy::post_stop)
      .def("on_trading_day", &strategy::Strategy::on_trading_day)
      .def("on_quote", &strategy::Strategy::on_quote)
      .def("on_bar", &strategy::Strategy::on_bar)
      .def("on_top_of_book", &strategy::Strategy::on_top_of_book)
      .def("on_entrust", &strategy::Strategy::on_entrust)
      .def("on_transaction", &strategy::Strategy::on_transaction)
      .def("on_order", &strategy::Strategy::on_order)
      .def("on_trade", &strategy::Strategy::on_trade)
      .def("on_book_sync_reset", &strategy::Strategy::on_book_sync_reset)
      .def("on_asset_sync_reset", &strategy::Strategy::on_asset_sync_reset)
      .def("on_history_order", &strategy::Strategy::on_history_order)
      .def("on_history_trade", &strategy::Strategy::on_history_trade);
  ;

  py::class_<BarGenerator, kungfu::yijinjing::practice::apprentice, std::shared_ptr<BarGenerator>>(m, "BarGenerator")
      .def(py::init<yijinjing::data::locator_ptr, longfist::enums::mode, bool, std::string &>())
      .def("run", &service::BarGenerator::run);
}
} // namespace kungfu::wingchun

#endif // KUNGFU_PY_WINGCHUN_HPP
