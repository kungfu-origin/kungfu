/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * PyWCStrategy: utility functions for strategy.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   October, 2017
 */
#include "PyWCStrategy.h"
#include <csignal>
#include "longfist/LFDataStructPybind11Def.h"

namespace py = pybind11;
USING_WC_NAMESPACE

void PyWCStrategy::init() {
    if (!py_init.is_none()) {
        py_init();
    }
}

void PyWCStrategy::start() {
    Py_Initialize();
    if (!PyEval_ThreadsInitialized())
        PyEval_InitThreads();
    std::signal(SIGTERM, IWCDataProcessor::signal_handler);
    std::signal(SIGINT, IWCDataProcessor::signal_handler);
    data_thread = ThreadPtr(new std::thread(&WCDataWrapper::run, data.get()));
    KF_LOG_INFO(logger, "[start] data started");
}

void PyWCStrategy::block(){
    py::gil_scoped_release release;
    IWCStrategy::block();
}

void PyWCStrategy::on_market_data(const LFMarketDataField *data, short source, long rcv_time) {
    if (!py_on_market_data.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py_on_market_data(data, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_market_bar(const BarMdMap &data, int min_interval, short source, long rcv_time) {
    if (!py_on_market_bar.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py::dict bar_d;
            for (auto &iter: data) {
                auto bar_a = &iter.second;
                bar_d[iter.first.c_str()] = bar_a;
            }
            py_on_market_bar(bar_d, min_interval, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_rtn_order(const LFRtnOrderField *data, int request_id, short source, long rcv_time) {
    if (!py_on_rtn_order.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py_on_rtn_order(data, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_rtn_trade(const LFRtnTradeField *data, int request_id, short source, long rcv_time) {
    if (!py_on_rtn_trade.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py_on_rtn_trade(data, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void
PyWCStrategy::on_rsp_order(const LFInputOrderField *data, int request_id, short source, long rcv_time, short errorId,
                           const char *errorMsg) {
    if (errorId != 0) {
        if (!py_on_error.is_none() && IWCDataProcessor::signal_received <= 0) {
            START_PYTHON_FUNC_CALLING
                string error_msg = errorMsg;
                py_on_error(errorId, error_msg, request_id, source, rcv_time);
            END_PYTHON_FUNC_CALLING
        }
    }
}

void PyWCStrategy::on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time) {
    if (!py_on_pos.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py_on_pos(posMap, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_switch_day(long rcv_time) {
    if (!py_on_switch_day.is_none() && IWCDataProcessor::signal_received <= 0) {
        START_PYTHON_FUNC_CALLING
            py_on_switch_day(rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy ::on_time(long cur_time) {
    START_PYTHON_FUNC_CALLING
        IWCStrategy::on_time(cur_time);
    END_PYTHON_FUNC_CALLING
}

std::vector<int> PyWCStrategy::get_effective_orders() const {
    return data->get_effective_orders();
}


void PyWCStrategy::set_init(py::object func) {
    py_init = func;
}

void PyWCStrategy::set_on_data(short msg_type, py::object func) {
    switch (msg_type){
        case MSG_TYPE_LF_MD:
            set_on_market_data(func);
            break;
        case MSG_TYPE_LF_BAR_MD:
            set_on_market_bar(func);
            break;
        case MSG_TYPE_LF_RTN_ORDER:
            set_on_rtn_order(func);
            break;
        case MSG_TYPE_LF_RTN_TRADE:
            set_on_rtn_trade(func);
            break;
    }
}

void PyWCStrategy::set_on_market_data(py::object func) {
    py_on_market_data = func;
}

void PyWCStrategy::set_on_market_bar(py::object func) {
    py_on_market_bar = func;
}

void PyWCStrategy::set_on_rtn_order(py::object func) {
    py_on_rtn_order = func;
}

void PyWCStrategy::set_on_rtn_trade(py::object func) {
    py_on_rtn_trade = func;
}


void PyWCStrategy::set_on_pos(py::object func) {
    py_on_pos = func;
}

void PyWCStrategy::set_on_error(py::object func) {
    py_on_error = func;
}

void PyWCStrategy::set_on_switch_day(py::object func) {
    py_on_switch_day = func;
}


PosHandlerPtr create_empty_pos(short source) {
    return PosHandler::create(source);
}

PosHandlerPtr create_msg_pos(short source, string pos_str) {
    return PosHandler::create(source, pos_str);
}

PYBIND11_MODULE(libwingchunstrategy, m) {
    LF_PYBIND11_DEF_STRUCT_LFMarketDataField
    LF_PYBIND11_DEF_STRUCT_LFBarMarketDataField
    LF_PYBIND11_DEF_STRUCT_LFRtnOrderField
    LF_PYBIND11_DEF_STRUCT_LFRtnTradeField
    py::class_<WCDataWrapper,WCDataWrapperPtr>(m, "DataWrapper")
            .def("stop", &WCDataWrapper::stop)
            .def("disable_auto_sub_mode", &WCDataWrapper::disable_auto_sub_mode)
            .def("add_market_data", &WCDataWrapper::add_market_data, py::arg("source"))
            .def("add_trade_engine", &WCDataWrapper::add_register_td, py::arg("source"))
            .def("set_pos", &WCDataWrapper::set_pos, py::arg("pos_handler"), py::arg("source"))
            .def("get_pos", &WCDataWrapper::get_pos, py::arg("source"))
            .def("get_ticker_pnl", &WCDataWrapper::get_ticker_pnl,
                 py::arg("source"), py::arg("ticker"), py::arg("include_fee") = false)
            .def("register_bar_md", &WCDataWrapper::register_bar_md,
                 py::arg("source"), py::arg("min_interval"), py::arg("start_time"), py::arg("end_time"));

    py::class_<PyWCStrategy, PyWCStrategyPtr>(m, "Strategy")
            .def(py::init<const string &>())
            .def("get_strategy_util", &PyWCStrategy::get_strategy_util)
            .def("get_data_wrapper", &PyWCStrategy::get_data_wrapper)
            .def("get_name", &IWCStrategy::get_name)
            .def("get_effective_orders", &PyWCStrategy::get_effective_orders)
            .def("log_debug", &PyWCStrategy::log_debug, (py::arg("msg")))
            .def("log_info", &PyWCStrategy::log_info, (py::arg("msg")))
            .def("log_error", &PyWCStrategy::log_error, (py::arg("msg")))
            .def("log_fatal", &PyWCStrategy::log_fatal, (py::arg("msg")))
            .def("init", &PyWCStrategy::init)
            .def("run", &PyWCStrategy::run)
            .def("start", &PyWCStrategy::start)
            .def("block", &PyWCStrategy::block)
            .def("set_init", &PyWCStrategy::set_init, (py::arg("func")))
            .def("set_on_data", &PyWCStrategy::set_on_data, py::arg("msg_type"), py::arg("func"))
            .def("set_on_pos", &PyWCStrategy::set_on_pos, (py::arg("func")))
            .def("set_on_switch_day", &PyWCStrategy::set_on_switch_day, (py::arg("func")))
            .def("insert_market_order", &PyWCStrategy::insert_market_order_py,
                 py::arg("source"), py::arg("ticker"), py::arg("exchange_id"), py::arg("volume"), py::arg(
                            "direction"), py::arg("offset"))
            .def("insert_limit_order", &PyWCStrategy::insert_limit_order_py,
                 py::arg("source"), py::arg("ticker"), py::arg("exchange_id"), py::arg("price"), py::arg(
                            "volume"), py::arg("direction"), py::arg("offset"))
            .def("insert_fok_order", &PyWCStrategy::insert_fok_order_py,
                 py::arg("source"), py::arg("ticker"), py::arg("exchange_id"), py::arg("price"), py::arg(
                            "volume"), py::arg("direction"), py::arg("offset"))
            .def("insert_fak_order", &PyWCStrategy::insert_fak_order_py,
                 py::arg("source"), py::arg("ticker"), py::arg("exchange_id"), py::arg("price"), py::arg(
                            "volume"), py::arg("direction"), py::arg("offset"))
            .def("req_position", &PyWCStrategy::req_position, (py::arg("source")))
            .def("cancel_order", &PyWCStrategy::cancel_order, py::arg("source"), py::arg("order_id"))
            .def("set_on_error", &PyWCStrategy::set_on_error, (py::arg("func")));

    py::class_<WCStrategyUtil,WCStrategyUtilPtr>(m, "Util")
            .def("subscribe_market_data", &WCStrategyUtil::subscribe_market_data,
                 py::arg("tickers"), py::arg("source"))
            .def("insert_callback", &WCStrategyUtil::insert_callback_py, py::arg("nano"), py::arg("func"))
            .def("get_nano", &WCStrategyUtil::get_nano)
            .def("get_time", &WCStrategyUtil::get_time)
            .def("parse_time", &WCStrategyUtil::parse_time, (py::arg("time_str")))
            .def("parse_nano", &WCStrategyUtil::parse_nano, (py::arg("nano_time")));

    py::class_<PosHandler,PosHandlerPtr>(m, "PosHandler")
            .def("update", &PosHandler::update_py,
                 py::arg("ticker"), py::arg("volume"), py::arg("direction"), py::arg("trade_off"))
            .def("add_pos", &PosHandler::add_pos_py,
                 py::arg("ticker"), py::arg("direction"), py::arg("tot_pos"), py::arg("yd_pos") = 0, py::arg(
                            "balance") = 0, py::arg("fee") = 0)
            .def("set_pos", &PosHandler::set_pos_py,
                 py::arg("ticker"), py::arg("direction"), py::arg("tot_pos"), py::arg("yd_pos") = 0, py::arg(
                            "balance") = 0, py::arg("fee") = 0)
            .def("dump", &PosHandler::to_string)
            .def("load", &PosHandler::init, (py::arg("json_str")))
            .def("switch_day", &PosHandler::switch_day)
            .def("get_tickers", &PosHandler::get_tickers)
            .def("get_net_tot", &PosHandler::get_net_total, (py::arg("ticker")))
            .def("get_net_yd", &PosHandler::get_net_yestd, (py::arg("ticker")))
            .def("get_long_tot", &PosHandler::get_long_total, (py::arg("ticker")))
            .def("get_long_yd", &PosHandler::get_long_yestd, (py::arg("ticker")))
            .def("get_short_tot", &PosHandler::get_short_total, (py::arg("ticker")))
            .def("get_short_yd", &PosHandler::get_short_yestd, (py::arg("ticker")))
            .def("get_net_fee", &PosHandler::get_net_fee, (py::arg("ticker")))
            .def("get_net_balance", &PosHandler::get_net_balance, (py::arg("ticker")))
            .def("get_long_fee", &PosHandler::get_long_fee, (py::arg("ticker")))
            .def("get_long_balance", &PosHandler::get_long_balance, (py::arg("ticker")))
            .def("get_short_fee", &PosHandler::get_short_fee, (py::arg("ticker")))
            .def("get_short_balance", &PosHandler::get_short_balance, (py::arg("ticker")));

    m.def("create_pos_handler", &create_empty_pos, (py::arg("source")));
    m.def("create_msg_handler", &create_msg_pos, py::arg("source"), py::arg("pos_str"));
}

