//
// Created by qlu on 2019/2/19.
//

#include "strategy/src/strategy_util.h"
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;
PYBIND11_MODULE(pystrategy, m)
{
    py::class_<kungfu::StrategyUtil>(m, "Util")
    .def(py::init<const std::string&>())
    .def("add_md", &kungfu::StrategyUtil::add_md, py::arg("source_id"))
    .def("add_account", &kungfu::StrategyUtil::add_account, py::arg("source_id"), py::arg("account_id"), py::arg("cash_limit"))
    .def("register_algo_service", &kungfu::StrategyUtil::register_algo_service)

    .def("on_quote", &kungfu::StrategyUtil::on_quote_py, py::arg("quote"))
    .def("on_order", &kungfu::StrategyUtil::on_order_py, py::arg("order"))
    .def("on_trade", &kungfu::StrategyUtil::on_trade_py, py::arg("trade"))

    .def("register_switch_day_callback", &kungfu::StrategyUtil::register_switch_day_callback, py::arg("cb"))

    .def("subscribe", &kungfu::StrategyUtil::subscribe, py::arg("source"), py::arg("instruments"), py::arg("exchange_id"), py::arg("is_level2")=false)
    .def("is_subscribed", &kungfu::StrategyUtil::is_subscribed, py::arg("source"), py::arg("instrument"), py::arg("exchange_id"))

    .def("insert_limit_order", &kungfu::StrategyUtil::insert_limit_order, py::arg("instrument_id"), py::arg("exchange_id"),
         py::arg("account_id"), py::arg("limit_price"), py::arg("volume"), py::arg("side"), py::arg("offset"))
    .def("insert_fok_order", &kungfu::StrategyUtil::insert_fok_order, py::arg("instrument_id"), py::arg("exchange_id"),
         py::arg("account_id"), py::arg("limit_price"), py::arg("volume"), py::arg("side"), py::arg("offset"))
    .def("insert_fak_order", &kungfu::StrategyUtil::insert_fak_order, py::arg("instrument_id"), py::arg("exchange_id"),
         py::arg("account_id"), py::arg("limit_price"), py::arg("volume"), py::arg("side"), py::arg("offset"))
    .def("insert_market_order", &kungfu::StrategyUtil::insert_market_order, py::arg("instrument_id"), py::arg("exchange_id"),
         py::arg("account_id"), py::arg("volume"), py::arg("side"), py::arg("offset"))
    .def("cancel_order", &kungfu::StrategyUtil::cancel_order, py::arg("order_id"))

    .def("insert_algo_order", &kungfu::StrategyUtil::insert_algo_order, py::arg("algo_type"), py::arg("order_input_msg"))
    .def("modify_algo_order", &kungfu::StrategyUtil::modify_algo_order, py::arg("order_id"), py::arg("cmd"))

    .def("set_log_level", &kungfu::StrategyUtil::set_log_level, py::arg("level"))
    .def("log_info", &kungfu::StrategyUtil::log_info, py::arg("msg"))
    .def("log_warn", &kungfu::StrategyUtil::log_warn, py::arg("msg"))
    .def("log_error", &kungfu::StrategyUtil::log_error, py::arg("msg"))

    .def("get_last_md", &kungfu::StrategyUtil::get_last_md_py, py::arg("instrument_id"), py::arg("exchange_id"))

    .def("on_push_by_min", &kungfu::StrategyUtil::on_push_by_min)
    .def("on_push_by_day", &kungfu::StrategyUtil::on_push_by_day)

    .def("get_initial_equity", &kungfu::StrategyUtil::get_initial_equity)
    .def("get_static_equity", &kungfu::StrategyUtil::get_static_equity)
    .def("get_dynamic_equity", &kungfu::StrategyUtil::get_dynamic_equity)
    .def("get_accumulated_pnl", &kungfu::StrategyUtil::get_accumulated_pnl)
    .def("get_accumulated_pnl_ratio", &kungfu::StrategyUtil::get_accumulated_pnl_ratio)
    .def("get_intraday_pnl", &kungfu::StrategyUtil::get_intraday_pnl)
    .def("get_intraday_pnl_ratio", &kungfu::StrategyUtil::get_intraday_pnl_ratio)
    .def("get_long_tot", &kungfu::StrategyUtil::get_long_tot, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_tot_avail", &kungfu::StrategyUtil::get_long_tot_avail, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_tot_fro", &kungfu::StrategyUtil::get_long_tot_fro, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_yd", &kungfu::StrategyUtil::get_long_yd, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_yd_avail", &kungfu::StrategyUtil::get_long_yd_avail, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_yd_fro", &kungfu::StrategyUtil::get_long_yd_fro, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_realized_pnl", &kungfu::StrategyUtil::get_long_realized_pnl, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_unrealized_pnl", &kungfu::StrategyUtil::get_long_unrealized_pnl, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_cost_price", &kungfu::StrategyUtil::get_long_cost_price, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_long_pos", &kungfu::StrategyUtil::get_long_pos, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_tot", &kungfu::StrategyUtil::get_short_tot, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_tot_avail", &kungfu::StrategyUtil::get_short_tot_avail, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_tot_fro", &kungfu::StrategyUtil::get_short_tot_fro, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_yd", &kungfu::StrategyUtil::get_short_yd, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_yd_avail", &kungfu::StrategyUtil::get_short_yd_avail, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_yd_fro", &kungfu::StrategyUtil::get_short_yd_fro, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_realized_pnl", &kungfu::StrategyUtil::get_short_realized_pnl, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_unrealized_pnl", &kungfu::StrategyUtil::get_short_unrealized_pnl, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_cost_price", &kungfu::StrategyUtil::get_short_cost_price, py::arg("instrument_id"), py::arg("exchange_id"))
    .def("get_short_pos", &kungfu::StrategyUtil::get_short_pos, py::arg("instrument_id"), py::arg("exchange_id"))
    ;
}
