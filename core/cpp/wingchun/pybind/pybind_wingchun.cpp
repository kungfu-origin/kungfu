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

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace py = pybind11;
namespace wc = kungfu::wingchun;

class PyStrategy : public wc::Strategy
{
public:
    using wc::Strategy::Strategy; // Inherit constructors

    void pre_run() override
    {PYBIND11_OVERLOAD(void, wc::Strategy, pre_run,); }

    void pre_quit() override
    {PYBIND11_OVERLOAD(void, wc::Strategy, pre_quit,); }

    void on_switch_day(const std::string &next_trading_day) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_switch_day, next_trading_day); }

    void on_quote(const wc::msg::data::Quote &quote) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_quote, quote); }

    void on_entrust(const wc::msg::data::Entrust &entrust) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_entrust, entrust); }

    void on_transaction(const wc::msg::data::Transaction &transaction) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_transaction, transaction); }

    void on_order(const wc::msg::data::Order &order) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_order, order); }

    void on_trade(const wc::msg::data::Trade &trade) override
    {PYBIND11_OVERLOAD(void, wc::Strategy, on_trade, trade); }

    void on_algo_order_status(uint64_t order_id, const std::string &algo_type, const std::string &event_msg) override
    {
        PYBIND11_OVERLOAD(void, wc::Strategy, on_algo_order_status, order_id, algo_type, event_msg);
    }
};

PYBIND11_MODULE(pywingchun, m)
{
    py::class_<wc::msg::data::Instrument>(m, "Instrument")
            .def_readonly("instrument_type", &wc::msg::data::Instrument::instrument_type)
            .def_property_readonly("instrument_id", &wc::msg::data::Instrument::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Instrument::get_exchange_id)
            .def("__repr__",
                 [](const wc::msg::data::Instrument &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::FutureInstrument>(m, "FutureInstrument")
            .def_readonly("contract_multiplier", &wc::msg::data::FutureInstrument::contract_multiplier)
            .def_readonly("price_tick", &wc::msg::data::FutureInstrument::price_tick)
            .def_readonly("delivery_year", &wc::msg::data::FutureInstrument::delivery_year)
            .def_readonly("delivery_month", &wc::msg::data::FutureInstrument::delivery_month)
            .def_readonly("is_trading", &wc::msg::data::FutureInstrument::is_trading)
            .def_readonly("long_margin_ratio", &wc::msg::data::FutureInstrument::long_margin_ratio)
            .def_readonly("short_margin_ratio", &wc::msg::data::FutureInstrument::short_margin_ratio)
            .def_property_readonly("product_id", &wc::msg::data::FutureInstrument::get_product_id)
            .def_property_readonly("open_date", &wc::msg::data::FutureInstrument::get_open_date)
            .def_property_readonly("create_date", &wc::msg::data::FutureInstrument::get_create_date)
            .def_property_readonly("expire_date", &wc::msg::data::FutureInstrument::get_expire_date)
            .def("__repr__",
                 [](const wc::msg::data::FutureInstrument &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Quote>(m, "Quote")
            .def_property_readonly("source_id", &wc::msg::data::Quote::get_source_id)
            .def_property_readonly("trading_day", &wc::msg::data::Quote::get_trading_day)
            .def_readonly("rcv_time", &wc::msg::data::Quote::rcv_time)
            .def_readonly("data_time", &wc::msg::data::Quote::data_time)
            .def_property_readonly("instrument_id", &wc::msg::data::Quote::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Quote::get_exchange_id)
            .def_readonly("instrument_type", &wc::msg::data::Quote::instrument_type)
            .def_readonly("pre_close_price", &wc::msg::data::Quote::pre_close_price)
            .def_readonly("pre_settlement_price", &wc::msg::data::Quote::pre_settlement_price)
            .def_readonly("last_price", &wc::msg::data::Quote::last_price)
            .def_readonly("volume", &wc::msg::data::Quote::volume)
            .def_readonly("turnover", &wc::msg::data::Quote::turnover)
            .def_readonly("pre_open_interest", &wc::msg::data::Quote::pre_open_interest)
            .def_readonly("open_interest", &wc::msg::data::Quote::open_interest)
            .def_readonly("open_price", &wc::msg::data::Quote::open_price)
            .def_readonly("high_price", &wc::msg::data::Quote::high_price)
            .def_readonly("low_price", &wc::msg::data::Quote::low_price)
            .def_readonly("upper_limit_price", &wc::msg::data::Quote::upper_limit_price)
            .def_readonly("lower_limit_price", &wc::msg::data::Quote::lower_limit_price)
            .def_readonly("close_price", &wc::msg::data::Quote::close_price)
            .def_readonly("settlement_price", &wc::msg::data::Quote::settlement_price)
            .def_property_readonly("bid_price", &wc::msg::data::Quote::get_bid_price)
            .def_property_readonly("ask_price", &wc::msg::data::Quote::get_ask_price)
            .def_property_readonly("bid_volume", &wc::msg::data::Quote::get_bid_volume)
            .def_property_readonly("ask_volume", &wc::msg::data::Quote::get_ask_volume)
            .def("__repr__",
                 [](const wc::msg::data::Quote &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Entrust>(m, "Entrust")
            .def_property_readonly("source_id", &wc::msg::data::Entrust::get_source_id)
            .def_property_readonly("trading_day", &wc::msg::data::Entrust::get_trading_day)
            .def_readonly("rcv_time", &wc::msg::data::Entrust::rcv_time)
            .def_readonly("data_time", &wc::msg::data::Entrust::data_time)
            .def_property_readonly("instrument_id", &wc::msg::data::Entrust::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Entrust::get_exchange_id)
            .def_readonly("instrument_type", &wc::msg::data::Entrust::instrument_type)
            .def_readonly("price", &wc::msg::data::Entrust::price)
            .def_readonly("volume", &wc::msg::data::Entrust::volume)
            .def_readonly("side", &wc::msg::data::Entrust::side)
            .def_readonly("price_type", &wc::msg::data::Entrust::price_type)
            .def_readonly("main_seq", &wc::msg::data::Entrust::main_seq)
            .def_readonly("seq", &wc::msg::data::Entrust::seq)
            .def("__repr__",
                 [](const wc::msg::data::Entrust &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Transaction>(m, "Transaction")
            .def_property_readonly("source_id", &wc::msg::data::Transaction::get_source_id)
            .def_property_readonly("trading_day", &wc::msg::data::Transaction::get_trading_day)
            .def_readonly("rcv_time", &wc::msg::data::Transaction::rcv_time)
            .def_readonly("data_time", &wc::msg::data::Transaction::data_time)
            .def_property_readonly("instrument_id", &wc::msg::data::Transaction::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Transaction::get_exchange_id)
            .def_readonly("instrument_type", &wc::msg::data::Transaction::instrument_type)
            .def_readonly("price", &wc::msg::data::Transaction::price)
            .def_readonly("volume", &wc::msg::data::Transaction::volume)
            .def_readonly("bid_no", &wc::msg::data::Transaction::bid_no)
            .def_readonly("ask_no", &wc::msg::data::Transaction::ask_no)
            .def_readonly("exec_type", &wc::msg::data::Transaction::exec_type)
            .def_readonly("bs_flag", &wc::msg::data::Transaction::bs_flag)
            .def_readonly("main_seq", &wc::msg::data::Transaction::main_seq)
            .def_readonly("seq", &wc::msg::data::Transaction::seq)
            .def("__repr__",
                 [](const wc::msg::data::Transaction &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::OrderInput>(m, "OrderInput")
            .def_readonly("order_id", &wc::msg::data::OrderInput::order_id)
            .def_readonly("instrument_type", &wc::msg::data::OrderInput::instrument_type)
            .def_readonly("limit_price", &wc::msg::data::OrderInput::limit_price)
            .def_readonly("frozen_price", &wc::msg::data::OrderInput::frozen_price)
            .def_readonly("volume", &wc::msg::data::OrderInput::volume)
            .def_readonly("side", &wc::msg::data::OrderInput::side)
            .def_readonly("offset", &wc::msg::data::OrderInput::offset)
            .def_readonly("price_type", &wc::msg::data::OrderInput::price_type)
            .def_readonly("volume_condition", &wc::msg::data::OrderInput::volume_condition)
            .def_readonly("time_condition", &wc::msg::data::OrderInput::time_condition)
            .def_readonly("parent_id", &wc::msg::data::OrderInput::parent_id)
            .def_property_readonly("instrument_id", &wc::msg::data::OrderInput::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::OrderInput::get_exchange_id)
            .def_property_readonly("account_id", &wc::msg::data::OrderInput::get_account_id)
            .def("__repr__",
                 [](const wc::msg::data::OrderInput &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Order>(m, "Order")
            .def_readonly("rcv_time", &wc::msg::data::Order::rcv_time)
            .def_readonly("order_id", &wc::msg::data::Order::order_id)
            .def_readonly("insert_time", &wc::msg::data::Order::insert_time)
            .def_readonly("update_time", &wc::msg::data::Order::update_time)
            .def_readonly("instrument_type", &wc::msg::data::Order::instrument_type)
            .def_readonly("limit_price", &wc::msg::data::Order::limit_price)
            .def_readonly("frozen_price", &wc::msg::data::Order::frozen_price)
            .def_readonly("volume", &wc::msg::data::Order::volume)
            .def_readonly("volume_traded", &wc::msg::data::Order::volume_traded)
            .def_readonly("volume_left", &wc::msg::data::Order::volume_left)
            .def_readonly("tax", &wc::msg::data::Order::tax)
            .def_readonly("commission", &wc::msg::data::Order::commission)
            .def_readonly("status", &wc::msg::data::Order::status)
            .def_readonly("error_id", &wc::msg::data::Order::error_id)
            .def_readonly("side", &wc::msg::data::Order::side)
            .def_readonly("offset", &wc::msg::data::Order::offset)
            .def_readonly("price_type", &wc::msg::data::Order::price_type)
            .def_readonly("volume_condition", &wc::msg::data::Order::volume_condition)
            .def_readonly("time_condition", &wc::msg::data::Order::time_condition)
            .def_property_readonly("trading_day", &wc::msg::data::Order::get_trading_day)
            .def_property_readonly("instrument_id", &wc::msg::data::Order::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Order::get_exchange_id)
            .def_property_readonly("account_id", &wc::msg::data::Order::get_account_id)
            .def_property_readonly("client_id", &wc::msg::data::Order::get_client_id)
            .def_property_readonly("error_msg", &wc::msg::data::Order::get_error_msg)
            .def("__repr__",
                 [](const wc::msg::data::Order &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::OrderAction>(m, "OrderAction")
            .def_readonly("order_id", &wc::msg::data::OrderAction::order_id)
            .def_readonly("order_action_id", &wc::msg::data::OrderAction::order_action_id)
            .def_readonly("action_flag", &wc::msg::data::OrderAction::action_flag)
            .def_readonly("price", &wc::msg::data::OrderAction::price)
            .def_readonly("volume", &wc::msg::data::OrderAction::volume)
            .def("__repr__",
                 [](const wc::msg::data::OrderAction &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Trade>(m, "Trade")
            .def_readonly("rcv_time", &wc::msg::data::Trade::rcv_time)
            .def_readonly("id", &wc::msg::data::Trade::id)
            .def_readonly("order_id", &wc::msg::data::Trade::order_id)
            .def_readonly("parent_order_id", &wc::msg::data::Trade::parent_order_id)
            .def_readonly("trade_time", &wc::msg::data::Trade::trade_time)
            .def_readonly("instrument_type", &wc::msg::data::Trade::instrument_type)
            .def_readonly("side", &wc::msg::data::Trade::side)
            .def_readonly("offset", &wc::msg::data::Trade::offset)
            .def_readonly("price", &wc::msg::data::Trade::price)
            .def_readonly("volume", &wc::msg::data::Trade::volume)
            .def_readonly("tax", &wc::msg::data::Trade::tax)
            .def_readonly("commission", &wc::msg::data::Trade::commission)
            .def_property_readonly("instrument_id", &wc::msg::data::Trade::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Trade::get_exchange_id)
            .def_property_readonly("account_id", &wc::msg::data::Trade::get_account_id)
            .def_property_readonly("client_id", &wc::msg::data::Trade::get_client_id)
            .def("__repr__",
                 [](const wc::msg::data::Trade &a)
                 {
                     return wc::msg::data::to_string(a);
                 }
            );
    py::class_<wc::msg::data::AccountInfo>(m, "AccountInfo")
            .def_readonly("rcv_time", &wc::msg::data::AccountInfo::rcv_time)
            .def_readonly("update_time", &wc::msg::data::AccountInfo::update_time)
            .def_readonly("initial_equity", &wc::msg::data::AccountInfo::initial_equity)
            .def_readonly("static_equity", &wc::msg::data::AccountInfo::static_equity)
            .def_readonly("dynamic_equity", &wc::msg::data::AccountInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &wc::msg::data::AccountInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &wc::msg::data::AccountInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &wc::msg::data::AccountInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &wc::msg::data::AccountInfo::intraday_pnl_ratio)
            .def_readonly("avail", &wc::msg::data::AccountInfo::avail)
            .def_readonly("market_value", &wc::msg::data::AccountInfo::market_value)
            .def_readonly("margin", &wc::msg::data::AccountInfo::margin)
            .def_readonly("accumulated_fee", &wc::msg::data::AccountInfo::accumulated_fee)
            .def_readonly("intraday_fee", &wc::msg::data::AccountInfo::intraday_fee)
            .def_readonly("frozen_cash", &wc::msg::data::AccountInfo::frozen_cash)
            .def_readonly("frozen_margin", &wc::msg::data::AccountInfo::frozen_margin)
            .def_readonly("frozen_fee", &wc::msg::data::AccountInfo::frozen_fee)
            .def_readonly("position_pnl", &wc::msg::data::AccountInfo::position_pnl)
            .def_readonly("close_pnl", &wc::msg::data::AccountInfo::close_pnl)
            .def_property_readonly("trading_day", &wc::msg::data::AccountInfo::get_trading_day)
            .def_property_readonly("account_id", &wc::msg::data::AccountInfo::get_account_id)
            .def_property_readonly("broker_id", &wc::msg::data::AccountInfo::get_broker_id)
            .def_property_readonly("source_id", &wc::msg::data::AccountInfo::get_source_id)
            .def("__repr__",
                 [](const wc::msg::data::AccountInfo &a)
                 {
                     return wc::msg::nanomsg::to_string(a);
                 }
            );
    py::class_<wc::msg::data::Position>(m, "Position")
            .def_readonly("rcv_time", &wc::msg::data::Position::rcv_time)
            .def_readonly("update_time", &wc::msg::data::Position::update_time)
            .def_readonly("instrument_type", &wc::msg::data::Position::instrument_type)
            .def_readonly("direction", &wc::msg::data::Position::direction)
            .def_readonly("volume", &wc::msg::data::Position::volume)
            .def_readonly("yesterday_volume", &wc::msg::data::Position::yesterday_volume)
            .def_readonly("frozen_total", &wc::msg::data::Position::frozen_total)
            .def_readonly("frozen_yesterday", &wc::msg::data::Position::frozen_yesterday)
            .def_readonly("last_price", &wc::msg::data::Position::last_price)
            .def_readonly("open_price", &wc::msg::data::Position::open_price)
            .def_readonly("cost_price", &wc::msg::data::Position::cost_price)
            .def_readonly("close_price", &wc::msg::data::Position::close_price)
            .def_readonly("pre_close_price", &wc::msg::data::Position::pre_close_price)
            .def_readonly("settlement_price", &wc::msg::data::Position::settlement_price)
            .def_readonly("pre_settlement_price", &wc::msg::data::Position::pre_settlement_price)
            .def_readonly("margin", &wc::msg::data::Position::margin)
            .def_readonly("position_pnl", &wc::msg::data::Position::position_pnl)
            .def_readonly("close_pnl", &wc::msg::data::Position::close_pnl)
            .def_readonly("realized_pnl", &wc::msg::data::Position::realized_pnl)
            .def_readonly("unrealized_pnl", &wc::msg::data::Position::unrealized_pnl)
            .def_property_readonly("trading_day", &wc::msg::data::Position::get_trading_day)
            .def_property_readonly("instrument_id", &wc::msg::data::Position::get_instrument_id)
            .def_property_readonly("exchange_id", &wc::msg::data::Position::get_exchange_id)
            .def_property_readonly("account_id", &wc::msg::data::Position::get_account_id)
            .def_property_readonly("client_id", &wc::msg::data::Position::get_client_id)
            .def_property_readonly("open_date", &wc::msg::data::Position::get_open_date)
            .def_property_readonly("expire_date", &wc::msg::data::Position::get_expire_date)
            .def("__repr__",
                 [](const wc::msg::data::Position &a)
                 {
                     return wc::msg::nanomsg::to_string(a);
                 }
            );
    py::class_<wc::msg::data::PortfolioInfo>(m, "PortfolioInfo")
            .def_readonly("update_time", &wc::msg::data::PortfolioInfo::update_time)
            .def_readonly("initial_equity", &wc::msg::data::PortfolioInfo::initial_equity)
            .def_readonly("static_equity", &wc::msg::data::PortfolioInfo::static_equity)
            .def_readonly("dynamic_equity", &wc::msg::data::PortfolioInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &wc::msg::data::PortfolioInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &wc::msg::data::PortfolioInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &wc::msg::data::PortfolioInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &wc::msg::data::PortfolioInfo::intraday_pnl_ratio)
            .def_property_readonly("trading_day", &wc::msg::data::PortfolioInfo::get_trading_day)
            .def_property_readonly("client_id", &wc::msg::data::PortfolioInfo::get_client_id)
            .def("__repr__",
                 [](const wc::msg::data::PortfolioInfo &a)
                 {
                     return wc::msg::nanomsg::to_string(a);
                 }
            );

    py::class_<wc::Strategy, PyStrategy>(m, "Strategy")
            .def("pre_run", &wc::Strategy::pre_run)
            .def("pre_quit", &wc::Strategy::pre_quit)
            .def("on_switch_day", &wc::Strategy::on_switch_day)
            .def("on_quote", &wc::Strategy::on_quote)
            .def("on_entrust", &wc::Strategy::on_entrust)
            .def("on_transaction", &wc::Strategy::on_transaction)
            .def("on_order", &wc::Strategy::on_order)
            .def("on_trade", &wc::Strategy::on_trade)
            .def("on_algo_order_status", &wc::Strategy::on_algo_order_status);

    py::class_<wc::Calendar>(m, "Calendar")
            .def(py::init<const std::string &>())
            .def("current_day", &wc::Calendar::get_current_trading_day)
            .def("calculate_trading_day", &wc::Calendar::calculate_trading_day);
}
