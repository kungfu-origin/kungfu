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
namespace kfj = kungfu::journal;

PYBIND11_MODULE(pywingchun, m)
{
    py::class_<kfj::Instrument>(m, "Instrument")
    .def_readonly("instrument_type", &kfj::Instrument::instrument_type)
    .def_property_readonly("instrument_id", &kfj::Instrument::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Instrument::get_exchange_id)
    ;
    py::class_<kfj::FutureInstrument>(m, "FutureInstrument")
    .def_readonly("contract_multiplier", &kfj::FutureInstrument::contract_multiplier)
    .def_readonly("price_tick", &kfj::FutureInstrument::price_tick)
    .def_readonly("delivery_year", &kfj::FutureInstrument::delivery_year)
    .def_readonly("delivery_month", &kfj::FutureInstrument::delivery_month)
    .def_readonly("is_trading", &kfj::FutureInstrument::is_trading)
    .def_readonly("long_margin_ratio", &kfj::FutureInstrument::long_margin_ratio)
    .def_readonly("short_margin_ratio", &kfj::FutureInstrument::short_margin_ratio)
    .def_property_readonly("product_id", &kfj::FutureInstrument::get_product_id)
    .def_property_readonly("open_date", &kfj::FutureInstrument::get_open_date)
    .def_property_readonly("create_date", &kfj::FutureInstrument::get_create_date)
    .def_property_readonly("expire_date", &kfj::FutureInstrument::get_expire_date)
    ;
    py::class_<kfj::Quote>(m, "Quote")
    .def_property_readonly("source_id", &kfj::Quote::get_source_id)
    .def_property_readonly("trading_day", &kfj::Quote::get_trading_day)
    .def_readonly("rcv_time", &kfj::Quote::rcv_time)
    .def_readonly("data_time", &kfj::Quote::data_time)
    .def_property_readonly("instrument_id", &kfj::Quote::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Quote::get_exchange_id)
    .def_readonly("instrument_type", &kfj::Quote::instrument_type)
    .def_readonly("pre_close_price", &kfj::Quote::pre_close_price)
    .def_readonly("pre_settlement_price", &kfj::Quote::pre_settlement_price)
    .def_readonly("last_price", &kfj::Quote::last_price)
    .def_readonly("volume", &kfj::Quote::volume)
    .def_readonly("turnover", &kfj::Quote::turnover)
    .def_readonly("pre_open_interest", &kfj::Quote::pre_open_interest)
    .def_readonly("open_interest", &kfj::Quote::open_interest)
    .def_readonly("open_price", &kfj::Quote::open_price)
    .def_readonly("high_price", &kfj::Quote::high_price)
    .def_readonly("low_price", &kfj::Quote::low_price)
    .def_readonly("upper_limit_price", &kfj::Quote::upper_limit_price)
    .def_readonly("lower_limit_price", &kfj::Quote::lower_limit_price)
    .def_readonly("close_price", &kfj::Quote::close_price)
    .def_readonly("settlement_price", &kfj::Quote::settlement_price)
    .def_property_readonly("bid_price", &kfj::Quote::get_bid_price)
    .def_property_readonly("ask_price", &kfj::Quote::get_ask_price)
    .def_property_readonly("bid_volume", &kfj::Quote::get_bid_volume)
    .def_property_readonly("ask_volume", &kfj::Quote::get_ask_volume)
    ;
    py::class_<kfj::Entrust>(m, "Entrust")
    .def_property_readonly("source_id", &kfj::Entrust::get_source_id)
    .def_property_readonly("trading_day", &kfj::Entrust::get_trading_day)
    .def_readonly("rcv_time", &kfj::Entrust::rcv_time)
    .def_readonly("data_time", &kfj::Entrust::data_time)
    .def_property_readonly("instrument_id", &kfj::Entrust::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Entrust::get_exchange_id)
    .def_readonly("instrument_type", &kfj::Entrust::instrument_type)
    .def_readonly("price", &kfj::Entrust::price)
    .def_readonly("volume", &kfj::Entrust::volume)
    .def_readonly("side", &kfj::Entrust::side)
    .def_readonly("price_type", &kfj::Entrust::price_type)
    .def_readonly("main_seq", &kfj::Entrust::main_seq)
    .def_readonly("seq", &kfj::Entrust::seq)
    ;
    py::class_<kfj::Transaction>(m, "Transaction")
    .def_property_readonly("source_id", &kfj::Transaction::get_source_id)
    .def_property_readonly("trading_day", &kfj::Transaction::get_trading_day)
    .def_readonly("rcv_time", &kfj::Transaction::rcv_time)
    .def_readonly("data_time", &kfj::Transaction::data_time)
    .def_property_readonly("instrument_id", &kfj::Transaction::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Transaction::get_exchange_id)
    .def_readonly("instrument_type", &kfj::Transaction::instrument_type)
    .def_readonly("price", &kfj::Transaction::price)
    .def_readonly("volume", &kfj::Transaction::volume)
    .def_readonly("bid_no", &kfj::Transaction::bid_no)
    .def_readonly("ask_no", &kfj::Transaction::ask_no)
    .def_readonly("exec_type", &kfj::Transaction::exec_type)
    .def_readonly("bs_flag", &kfj::Transaction::bs_flag)
    .def_readonly("main_seq", &kfj::Transaction::main_seq)
    .def_readonly("seq", &kfj::Transaction::seq)
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
