#include "py-wingchun.h"

#include <pybind11/functional.h>

#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runner.h>

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::book;

namespace py = pybind11;

namespace kungfu::wingchun::pybind {

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

void bind_strategy(pybind11::module &m) {

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
      .def("subscribe_all", &strategy::Context::subscribe_all, py::arg("source"), py::arg("exchanges_ids")=MarketType::kNone,
           py::arg("instrument_types")=SubscribeCategoryType::kNone, py::arg("callback_types")=SubscribeSecuDataType::kNone)
      .def("insert_order", &strategy::Context::insert_order, py::arg("symbol"), py::arg("exchange"), py::arg("account"),
           py::arg("limit_price"), py::arg("volume"), py::arg("type"), py::arg("side"),
           py::arg("offset") = Offset::Open, py::arg("hedge_flag") = HedgeFlag::Speculation)
      .def("make_order", &strategy::Context::make_order, py::arg("symbol"), py::arg("exchange"), py::arg("source"),
           py::arg("account"), py::arg("limit_price"), py::arg("volume"), py::arg("type"), py::arg("side"),
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
}
} // namespace kungfu::wingchun::pybind