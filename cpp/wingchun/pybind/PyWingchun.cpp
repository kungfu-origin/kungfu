/*****************************************************************************
 * Copyright [2019] [taurus.ai]
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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "md_struct.h"
#include "oms_struct.h"
#include "strategy/include/strategy.h"
#include "strategy/src/strategy_util.h"
#include "calendar/include/calendar_service.h"

namespace py = pybind11;
namespace kf = kungfu;

PYBIND11_MODULE(pywingchun, m)
{
    py::class_<kf::Instrument>(m, "Instrument")
    .def_readonly("instrument_type", &kf::Instrument::instrument_type)
    .def_property_readonly("instrument_id", &kf::Instrument::get_instrument_id)
    .def_property_readonly("exchange_id", &kf::Instrument::get_exchange_id)
    ;
    py::class_<kf::FutureInstrument>(m, "FutureInstrument")
    .def_readonly("contract_multiplier", &kf::FutureInstrument::contract_multiplier)
    .def_readonly("price_tick", &kf::FutureInstrument::price_tick)
    .def_readonly("delivery_year", &kf::FutureInstrument::delivery_year)
    .def_readonly("delivery_month", &kf::FutureInstrument::delivery_month)
    .def_readonly("is_trading", &kf::FutureInstrument::is_trading)
    .def_readonly("long_margin_ratio", &kf::FutureInstrument::long_margin_ratio)
    .def_readonly("short_margin_ratio", &kf::FutureInstrument::short_margin_ratio)
    .def_property_readonly("product_id", &kf::FutureInstrument::get_product_id)
    .def_property_readonly("open_date", &kf::FutureInstrument::get_open_date)
    .def_property_readonly("create_date", &kf::FutureInstrument::get_create_date)
    .def_property_readonly("expire_date", &kf::FutureInstrument::get_expire_date)
    ;
    py::class_<kf::Quote>(m, "Quote")
    .def_property_readonly("source_id", &kf::Quote::get_source_id)
    .def_property_readonly("trading_day", &kf::Quote::get_trading_day)
    .def_readonly("rcv_time", &kf::Quote::rcv_time)
    .def_readonly("data_time", &kf::Quote::data_time)
    .def_property_readonly("instrument_id", &kf::Quote::get_instrument_id)
    .def_property_readonly("exchange_id", &kf::Quote::get_exchange_id)
    .def_readonly("instrument_type", &kf::Quote::instrument_type)
    .def_readonly("pre_close_price", &kf::Quote::pre_close_price)
    .def_readonly("pre_settlement_price", &kf::Quote::pre_settlement_price)
    .def_readonly("last_price", &kf::Quote::last_price)
    .def_readonly("volume", &kf::Quote::volume)
    .def_readonly("turnover", &kf::Quote::turnover)
    .def_readonly("pre_open_interest", &kf::Quote::pre_open_interest)
    .def_readonly("open_interest", &kf::Quote::open_interest)
    .def_readonly("open_price", &kf::Quote::open_price)
    .def_readonly("high_price", &kf::Quote::high_price)
    .def_readonly("low_price", &kf::Quote::low_price)
    .def_readonly("upper_limit_price", &kf::Quote::upper_limit_price)
    .def_readonly("lower_limit_price", &kf::Quote::lower_limit_price)
    .def_readonly("close_price", &kf::Quote::close_price)
    .def_readonly("settlement_price", &kf::Quote::settlement_price)
    .def_property_readonly("bid_price", &kf::Quote::get_bid_price)
    .def_property_readonly("ask_price", &kf::Quote::get_ask_price)
    .def_property_readonly("bid_volume", &kf::Quote::get_bid_volume)
    .def_property_readonly("ask_volume", &kf::Quote::get_ask_volume)
    ;
    py::class_<kf::Entrust>(m, "Entrust")
    .def_property_readonly("source_id", &kf::Entrust::get_source_id)
    .def_property_readonly("trading_day", &kf::Entrust::get_trading_day)
    .def_readonly("rcv_time", &kf::Entrust::rcv_time)
    .def_readonly("data_time", &kf::Entrust::data_time)
    .def_property_readonly("instrument_id", &kf::Entrust::get_instrument_id)
    .def_property_readonly("exchange_id", &kf::Entrust::get_exchange_id)
    .def_readonly("instrument_type", &kf::Entrust::instrument_type)
    .def_readonly("price", &kf::Entrust::price)
    .def_readonly("volume", &kf::Entrust::volume)
    .def_readonly("side", &kf::Entrust::side)
    .def_readonly("price_type", &kf::Entrust::price_type)
    .def_readonly("main_seq", &kf::Entrust::main_seq)
    .def_readonly("seq", &kf::Entrust::seq)
    ;
    py::class_<kf::Transaction>(m, "Transaction")
    .def_property_readonly("source_id", &kf::Transaction::get_source_id)
    .def_property_readonly("trading_day", &kf::Transaction::get_trading_day)
    .def_readonly("rcv_time", &kf::Transaction::rcv_time)
    .def_readonly("data_time", &kf::Transaction::data_time)
    .def_property_readonly("instrument_id", &kf::Transaction::get_instrument_id)
    .def_property_readonly("exchange_id", &kf::Transaction::get_exchange_id)
    .def_readonly("instrument_type", &kf::Transaction::instrument_type)
    .def_readonly("price", &kf::Transaction::price)
    .def_readonly("volume", &kf::Transaction::volume)
    .def_readonly("bid_no", &kf::Transaction::bid_no)
    .def_readonly("ask_no", &kf::Transaction::ask_no)
    .def_readonly("exec_type", &kf::Transaction::exec_type)
    .def_readonly("bs_flag", &kf::Transaction::bs_flag)
    .def_readonly("main_seq", &kf::Transaction::main_seq)
    .def_readonly("seq", &kf::Transaction::seq)
    ;

    py::class_<kf::Strategy>(m, "Strategy")
    .def(py::init<const std::string&>())
    .def("run", &kf::Strategy::run)
    .def("stop", &kf::Strategy::stop)
    .def("add_md", &kf::Strategy::add_md)
    .def("add_account", &kf::Strategy::add_account)
    .def("register_algo_service", &kf::Strategy::register_algo_service)
    .def("get_nano", &kf::Strategy::get_nano)
    // .def("get_last_md", &kf::Strategy::get_last_md)
    .def("get_position", &kf::Strategy::get_position)
    .def("get_portfolio_info", &kf::Strategy::get_portfolio_info)
    .def("get_sub_portfolio_info", &kf::Strategy::get_sub_portfolio_info)
    .def("subscribe", &kf::Strategy::subscribe)
    .def("insert_limit_order", &kf::Strategy::insert_limit_order)
    .def("insert_fok_order", &kf::Strategy::insert_fok_order)
    .def("insert_fak_order", &kf::Strategy::insert_fak_order)
    .def("insert_market_order", &kf::Strategy::insert_market_order)
    .def("cancel_order", &kf::Strategy::cancel_order)
    .def("_register_nanotime_callback", &kf::Strategy::register_nanotime_callback)
    .def("insert_algo_order", &kf::Strategy::insert_algo_order)
    .def("modify_algo_order", &kf::Strategy::modify_algo_order)
    // .def("try_frozen", &kf::Strategy::try_frozen)
    .def("cancel_frozen", &kf::Strategy::cancel_frozen)
    .def("commit_frozen", &kf::Strategy::commit_frozen)
    .def("try_modify_position", &kf::Strategy::try_modify_position)
    .def("cancel_modify_position", &kf::Strategy::cancel_modify_position)
    .def("commit_modify_position", &kf::Strategy::commit_modify_position)
    .def("init", &kf::Strategy::init)
    .def("pre_run", &kf::Strategy::pre_run)
    .def("pre_quit", &kf::Strategy::pre_quit)
    .def("on_switch_day", &kf::Strategy::on_switch_day)
    .def("on_quote", &kf::Strategy::on_quote)
    .def("on_entrust", &kf::Strategy::on_entrust)
    .def("on_transaction", &kf::Strategy::on_transaction)
    .def("on_order", &kf::Strategy::on_order)
    .def("on_trade", &kf::Strategy::on_trade)
    .def("on_algo_order_status", &kf::Strategy::on_algo_order_status)
    ;

    py::class_<kf::CalendarService>(m, "CalendarService")
    .def(py::init())
    .def("current_day", &kf::CalendarService::get_current_day)
    .def("calculate_trading_day", &kf::CalendarService::calculate_trading_day)
    ;
}
