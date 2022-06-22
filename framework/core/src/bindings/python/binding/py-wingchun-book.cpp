#include "py-wingchun.h"

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <kungfu/wingchun/book/accounting.h>
#include <kungfu/wingchun/book/book.h>
#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::book;

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(CommissionMap)
PYBIND11_MAKE_OPAQUE(InstrumentMap)
PYBIND11_MAKE_OPAQUE(PositionMap)
PYBIND11_MAKE_OPAQUE(OrderInputMap)
PYBIND11_MAKE_OPAQUE(OrderMap)
PYBIND11_MAKE_OPAQUE(TradeMap)

namespace kungfu::wingchun::pybind {
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

void bind_book(pybind11::module &m) {
  py::bind_map<CommissionMap>(m, "CommissionMap");
  py::bind_map<InstrumentMap>(m, "InstrumentMap");
  py::bind_map<PositionMap>(m, "PositionMap");
  py::bind_map<OrderInputMap>(m, "OrderInputMap");
  py::bind_map<OrderMap>(m, "OrderMap");
  py::bind_map<TradeMap>(m, "TradeMap");

  py::class_<Book, Book_ptr>(m, "Book")
      .def_readonly("asset", &Book::asset, py::return_value_policy::reference)
      .def_readonly("asset_margin", &Book::asset_margin, py::return_value_policy::reference)
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
}
} // namespace kungfu::wingchun::pybind