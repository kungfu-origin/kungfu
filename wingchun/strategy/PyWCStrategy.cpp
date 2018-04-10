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
#include "TypeConvert.hpp"
#include <csignal>

USING_WC_NAMESPACE

namespace bp = boost::python;

void PyWCStrategy::init()
{
    if (py_init != bp::object())
    {
        PyGILState_STATE gstate = PyGILState_Ensure();
        py_init();
        PyGILState_Release(gstate);
    }
}

void PyWCStrategy::start()
{
    Py_Initialize();
    if (!PyEval_ThreadsInitialized())
        PyEval_InitThreads();
    std::signal(SIGTERM, IWCDataProcessor::signal_handler);
    std::signal(SIGINT, IWCDataProcessor::signal_handler);
    data_thread = ThreadPtr(new std::thread(&WCDataWrapper::run, data.get()));
    KF_LOG_INFO(logger, "[start] data started");
}

void PyWCStrategy::on_market_data(const LFMarketDataField* data, short source, long rcv_time)
{
    bp::object& obj = py_on_data[MSG_TYPE_LF_MD];
    if (obj != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        obj((uintptr_t)data, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time)
{
    bp::object& obj = py_on_data[MSG_TYPE_LF_BAR_MD];
    if (obj != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        bp::dict bar_d;
        for (auto &iter: data)
        {
            uintptr_t bar_a = (uintptr_t) (&iter.second);
            bar_d[iter.first] = bar_a;
        }
        obj(bar_d, min_interval, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time)
{
    bp::object& obj = py_on_data[MSG_TYPE_LF_RTN_ORDER];
    if (obj != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        obj((uintptr_t)data, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time)
{
    bp::object& obj = py_on_data[MSG_TYPE_LF_RTN_TRADE];
    if (obj != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        obj((uintptr_t)data, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId, const char* errorMsg)
{
    if (errorId != 0)
    {
        if (py_on_error != bp::object() && IWCDataProcessor::signal_received <= 0)
        {
            START_PYTHON_FUNC_CALLING
            string error_msg = errorMsg;
            py_on_error(errorId, error_msg, request_id, source, rcv_time);
            END_PYTHON_FUNC_CALLING
        }
    }
}

void PyWCStrategy::on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time)
{
    if (py_on_pos != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        py_on_pos(posMap, request_id, source, rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

void PyWCStrategy::on_switch_day(long rcv_time)
{
    if (py_on_switch_day != bp::object() && IWCDataProcessor::signal_received <= 0)
    {
        START_PYTHON_FUNC_CALLING
        py_on_switch_day(rcv_time);
        END_PYTHON_FUNC_CALLING
    }
}

boost::python::list PyWCStrategy::get_effective_orders() const
{
    return kungfu::yijinjing::std_vector_to_py_list<int>(data->get_effective_orders());
}

void PyWCStrategy::set_init(bp::object func)
{
    py_init = func;
}

void PyWCStrategy::set_on_data(short msg_type, bp::object func)
{
    py_on_data[msg_type] = func;
}

void PyWCStrategy::set_on_pos(bp::object func)
{
    py_on_pos = func;
}

void PyWCStrategy::set_on_error(bp::object func)
{
    py_on_error = func;
}

void PyWCStrategy::set_on_switch_day(bp::object func)
{
    py_on_switch_day = func;
}

PosHandlerPtr create_empty_pos(short source)
{
    return PosHandler::create(source);
}

PosHandlerPtr create_msg_pos(short source, string pos_str)
{
    return PosHandler::create(source, pos_str);
}

BOOST_PYTHON_MODULE(libwingchunstrategy)
{
    bp::class_<WCDataWrapper, WCDataWrapperPtr>("DataWrapper", bp::no_init)
    .def("stop", &WCDataWrapper::stop)
    .def("disable_auto_sub_mode", &WCDataWrapper::disable_auto_sub_mode)
    .def("add_market_data", &WCDataWrapper::add_market_data, (bp::arg("source")))
    .def("add_trade_engine", &WCDataWrapper::add_register_td, (bp::arg("source")))
    .def("set_pos", &WCDataWrapper::set_pos, (bp::arg("pos_handler"), bp::arg("source")))
    .def("get_pos", &WCDataWrapper::get_pos, (bp::arg("source")))
    .def("get_ticker_pnl", &WCDataWrapper::get_ticker_pnl, (bp::arg("source"), bp::arg("ticker"), bp::arg("include_fee")=false))
    .def("register_bar_md", &WCDataWrapper::register_bar_md, (bp::arg("source"), bp::arg("min_interval"), bp::arg("start_time"), bp::arg("end_time")));

    bp::class_<PyWCStrategy, PyWCStrategyPtr>("Strategy")
    .def(bp::init<string>(bp::arg("name")))
    .def("get_strategy_util", &PyWCStrategy::get_strategy_util)
    .def("get_data_wrapper", &PyWCStrategy::get_data_wrapper)
    .def("get_name", &IWCStrategy::get_name)
    .def("get_effective_orders", &PyWCStrategy::get_effective_orders)
    .def("log_debug", &PyWCStrategy::log_debug, (bp::arg("msg")))
    .def("log_info", &PyWCStrategy::log_info, (bp::arg("msg")))
    .def("log_error", &PyWCStrategy::log_error, (bp::arg("msg")))
    .def("log_fatal", &PyWCStrategy::log_fatal, (bp::arg("msg")))
    .def("init", &PyWCStrategy::init)
    .def("run", &PyWCStrategy::run)
    .def("start", &PyWCStrategy::start)
    .def("block", &PyWCStrategy::block)
    .def("set_init", &PyWCStrategy::set_init, (bp::arg("func")))
    .def("set_on_data", &PyWCStrategy::set_on_data, (bp::arg("msg_type"), bp::arg("func")))
    .def("set_on_pos", &PyWCStrategy::set_on_pos, (bp::arg("func")))
    .def("set_on_switch_day", &PyWCStrategy::set_on_switch_day, (bp::arg("func")))
    .def("insert_market_order", &PyWCStrategy::insert_market_order_py, (bp::arg("source"), bp::arg("ticker"), bp::arg("exchange_id"), bp::arg("volume"), bp::arg("direction"), bp::arg("offset")))
    .def("insert_limit_order", &PyWCStrategy::insert_limit_order_py, (bp::arg("source"), bp::arg("ticker"), bp::arg("exchange_id"), bp::arg("price"), bp::arg("volume"), bp::arg("direction"), bp::arg("offset")))
    .def("insert_fok_order", &PyWCStrategy::insert_fok_order_py, (bp::arg("source"), bp::arg("ticker"), bp::arg("exchange_id"), bp::arg("price"), bp::arg("volume"), bp::arg("direction"), bp::arg("offset")))
    .def("insert_fak_order", &PyWCStrategy::insert_fak_order_py, (bp::arg("source"), bp::arg("ticker"), bp::arg("exchange_id"), bp::arg("price"), bp::arg("volume"), bp::arg("direction"), bp::arg("offset")))
    .def("req_position", &PyWCStrategy::req_position, (bp::arg("source")))
    .def("cancel_order", &PyWCStrategy::cancel_order, (bp::arg("source"), bp::arg("order_id")))
    .def("set_on_error", &PyWCStrategy::set_on_error, (bp::arg("func")));

    bp::class_<WCStrategyUtil, WCStrategyUtilPtr>("Util", bp::no_init)
    .def("subscribe_market_data", &WCStrategyUtil::subscribe_market_data, (bp::arg("tickers"), bp::arg("source")))
    .def("insert_callback", &WCStrategyUtil::insert_callback_py, (bp::arg("nano"), bp::arg("func")))
    .def("get_nano", &WCStrategyUtil::get_nano)
    .def("get_time", &WCStrategyUtil::get_time)
    .def("parse_time", &WCStrategyUtil::parse_time, (bp::arg("time_str")))
    .def("parse_nano", &WCStrategyUtil::parse_nano, (bp::arg("nano_time")));

    bp::class_<PosHandler, PosHandlerPtr>("PosHandler", bp::no_init)
    .def("update", &PosHandler::update_py, (bp::arg("ticker"), bp::arg("volume"), bp::arg("direction"), bp::arg("trade_off"), bp::arg("order_off")))
    .def("add_pos", &PosHandler::add_pos_py, (bp::arg("ticker"), bp::arg("direction"), bp::arg("tot_pos"), bp::arg("yd_pos")=0, bp::arg("balance")=0, bp::arg("fee")=0))
    .def("set_pos", &PosHandler::set_pos_py, (bp::arg("ticker"), bp::arg("direction"), bp::arg("tot_pos"), bp::arg("yd_pos")=0, bp::arg("balance")=0, bp::arg("fee")=0))
    .def("dump", &PosHandler::to_string)
    .def("load", &PosHandler::init, (bp::arg("json_str")))
    .def("switch_day", &PosHandler::switch_day)
    .def("get_tickers", &PosHandler::get_py_tickers)
    .def("get_net_tot", &PosHandler::get_net_total, (bp::arg("ticker")))
    .def("get_net_yd", &PosHandler::get_net_yestd, (bp::arg("ticker")))
    .def("get_long_tot", &PosHandler::get_long_total, (bp::arg("ticker")))
    .def("get_long_yd", &PosHandler::get_long_yestd, (bp::arg("ticker")))
    .def("get_short_tot", &PosHandler::get_short_total, (bp::arg("ticker")))
    .def("get_short_yd", &PosHandler::get_short_yestd, (bp::arg("ticker")))
    .def("get_net_fee", &PosHandler::get_net_fee, (bp::arg("ticker")))
    .def("get_net_balance", &PosHandler::get_net_balance, (bp::arg("ticker")))
    .def("get_long_fee", &PosHandler::get_long_fee, (bp::arg("ticker")))
    .def("get_long_balance", &PosHandler::get_long_balance, (bp::arg("ticker")))
    .def("get_short_fee", &PosHandler::get_short_fee, (bp::arg("ticker")))
    .def("get_short_balance", &PosHandler::get_short_balance, (bp::arg("ticker")));

    bp::def("create_pos_handler", &create_empty_pos, (bp::arg("source")));
    bp::def("create_msg_handler", &create_msg_pos, (bp::arg("source"), bp::arg("pos_str")));
}