//
// Created by qlu on 2019/2/19.
//

#include "md_struct.h"
#include "oms_struct.h"
#include "strategy/src/strategy_util.h"
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

uintptr_t py_get_last_md(const kungfu::StrategyUtil& util, const std::string& instrument_id, const std::string& exchange_id)
{
    auto* quote = util.get_last_md(instrument_id, exchange_id);
    kungfu::Quote* py_quote = nullptr;
    if (nullptr != quote)
    {
        py_quote = (kungfu::Quote*)malloc(sizeof(kungfu::Quote));
        memcpy(py_quote, quote, sizeof(kungfu::Quote));
    }
    return (uintptr_t)py_quote;
}

uintptr_t py_get_position(const kungfu::StrategyUtil& util, const std::string& instrument_id, const std::string& exchange_id, kungfu::Direction direction = kungfu::DirectionLong, const std::string& account_id = "")
{
    auto pos = util.get_position(instrument_id, exchange_id, direction, account_id);
    auto* py_pos = (kungfu::Position*)malloc(sizeof(kungfu::Position));
    memcpy(py_pos, &pos, sizeof(kungfu::Position));
    return (uintptr_t)py_pos;
}

uintptr_t py_get_portfolio_info(const kungfu::StrategyUtil& util)
{
    auto pnl = util.get_portfolio_info();
    auto* py_pnl = (kungfu::PortfolioInfo*)malloc(sizeof(kungfu::PortfolioInfo));
    memcpy(py_pnl, &pnl, sizeof(kungfu::PortfolioInfo));
    return (uintptr_t)py_pnl;
}

uintptr_t py_get_sub_portfolio_info(const kungfu::StrategyUtil& util, const std::string& account_id)
{
    auto sub_pnl = util.get_sub_portfolio_info(account_id);
    auto* py_sub_pnl = (kungfu::SubPortfolioInfo*)malloc(sizeof(kungfu::SubPortfolioInfo));
    memcpy(py_sub_pnl, &sub_pnl, sizeof(kungfu::SubPortfolioInfo));
    return (uintptr_t)py_sub_pnl;
}

void py_release_ptr(uintptr_t ptr)
{
    auto p = (void*)ptr;
    if (nullptr != p)
    {
        free(p);
    }
}

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

    .def("on_push_by_min", &kungfu::StrategyUtil::on_push_by_min)
    .def("on_push_by_day", &kungfu::StrategyUtil::on_push_by_day)
    ;

    m.def("get_last_md", &py_get_last_md, py::arg("strategy"), py::arg("instrument_id"), py::arg("exchange_id"));
    m.def("get_position", &py_get_position, py::arg("strategy"), py::arg("instrument_id"), py::arg("exchange_id"), py::arg("direction")=kungfu::DirectionLong, py::arg("account_id")="");
    m.def("get_portfolio_info", &py_get_portfolio_info, py::arg("strategy"));
    m.def("get_sub_portfolio_info", &py_get_sub_portfolio_info, py::arg("strategy"), py::arg("account_id"));
    m.def("release_ptr", &py_release_ptr, py::arg("ptr"));

}
