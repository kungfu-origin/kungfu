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
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace py = pybind11;

using namespace kungfu::wingchun;

class PyStrategy : public strategy::Strategy
{
public:
    using strategy::Strategy::Strategy; // Inherit constructors

    void pre_start(strategy::Context_ptr context) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, pre_start, context); }

    void post_start(strategy::Context_ptr context) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, post_start, context); }

    void pre_stop(strategy::Context_ptr context) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, pre_stop, context); }

    void post_stop(strategy::Context_ptr context) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, post_stop, context); }

    void on_switch_day(strategy::Context_ptr context, const std::string &next_trading_day) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_switch_day, context, next_trading_day); }

    void on_quote(strategy::Context_ptr context, const msg::data::Quote &quote) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_quote, context, quote); }

    void on_entrust(strategy::Context_ptr context, const msg::data::Entrust &entrust) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_entrust, context, entrust); }

    void on_transaction(strategy::Context_ptr context, const msg::data::Transaction &transaction) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_transaction, context, transaction); }

    void on_order(strategy::Context_ptr context, const msg::data::Order &order) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_order, context, order); }

    void on_trade(strategy::Context_ptr context, const msg::data::Trade &trade) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_trade, context, trade); }

    void on_algo_order_status(strategy::Context_ptr context, uint64_t order_id, const std::string &algo_type, const std::string &event_msg) override
    {
        PYBIND11_OVERLOAD(void, strategy::Strategy, on_algo_order_status, context, order_id, algo_type, event_msg);
    }
};

PYBIND11_MODULE(pywingchun, m)
{
    py::class_<msg::data::Instrument>(m, "Instrument")
            .def_readonly("instrument_type", &msg::data::Instrument::instrument_type)
            .def_property_readonly("instrument_id", &msg::data::Instrument::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Instrument::get_exchange_id)
            .def("__repr__",
                 [](const msg::data::Instrument &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::FutureInstrument>(m, "FutureInstrument")
            .def_readonly("contract_multiplier", &msg::data::FutureInstrument::contract_multiplier)
            .def_readonly("price_tick", &msg::data::FutureInstrument::price_tick)
            .def_readonly("delivery_year", &msg::data::FutureInstrument::delivery_year)
            .def_readonly("delivery_month", &msg::data::FutureInstrument::delivery_month)
            .def_readonly("is_trading", &msg::data::FutureInstrument::is_trading)
            .def_readonly("long_margin_ratio", &msg::data::FutureInstrument::long_margin_ratio)
            .def_readonly("short_margin_ratio", &msg::data::FutureInstrument::short_margin_ratio)
            .def_property_readonly("product_id", &msg::data::FutureInstrument::get_product_id)
            .def_property_readonly("open_date", &msg::data::FutureInstrument::get_open_date)
            .def_property_readonly("create_date", &msg::data::FutureInstrument::get_create_date)
            .def_property_readonly("expire_date", &msg::data::FutureInstrument::get_expire_date)
            .def("__repr__",
                 [](const msg::data::FutureInstrument &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::Quote>(m, "Quote")
            .def_property_readonly("source_id", &msg::data::Quote::get_source_id)
            .def_property_readonly("trading_day", &msg::data::Quote::get_trading_day)
            .def_readonly("rcv_time", &msg::data::Quote::rcv_time)
            .def_readonly("data_time", &msg::data::Quote::data_time)
            .def_property_readonly("instrument_id", &msg::data::Quote::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Quote::get_exchange_id)
            .def_readonly("instrument_type", &msg::data::Quote::instrument_type)
            .def_readonly("pre_close_price", &msg::data::Quote::pre_close_price)
            .def_readonly("pre_settlement_price", &msg::data::Quote::pre_settlement_price)
            .def_readonly("last_price", &msg::data::Quote::last_price)
            .def_readonly("volume", &msg::data::Quote::volume)
            .def_readonly("turnover", &msg::data::Quote::turnover)
            .def_readonly("pre_open_interest", &msg::data::Quote::pre_open_interest)
            .def_readonly("open_interest", &msg::data::Quote::open_interest)
            .def_readonly("open_price", &msg::data::Quote::open_price)
            .def_readonly("high_price", &msg::data::Quote::high_price)
            .def_readonly("low_price", &msg::data::Quote::low_price)
            .def_readonly("upper_limit_price", &msg::data::Quote::upper_limit_price)
            .def_readonly("lower_limit_price", &msg::data::Quote::lower_limit_price)
            .def_readonly("close_price", &msg::data::Quote::close_price)
            .def_readonly("settlement_price", &msg::data::Quote::settlement_price)
            .def_property_readonly("bid_price", &msg::data::Quote::get_bid_price)
            .def_property_readonly("ask_price", &msg::data::Quote::get_ask_price)
            .def_property_readonly("bid_volume", &msg::data::Quote::get_bid_volume)
            .def_property_readonly("ask_volume", &msg::data::Quote::get_ask_volume)
            .def("__repr__",
                 [](const msg::data::Quote &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::Entrust>(m, "Entrust")
            .def_property_readonly("source_id", &msg::data::Entrust::get_source_id)
            .def_property_readonly("trading_day", &msg::data::Entrust::get_trading_day)
            .def_readonly("rcv_time", &msg::data::Entrust::rcv_time)
            .def_readonly("data_time", &msg::data::Entrust::data_time)
            .def_property_readonly("instrument_id", &msg::data::Entrust::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Entrust::get_exchange_id)
            .def_readonly("instrument_type", &msg::data::Entrust::instrument_type)
            .def_readonly("price", &msg::data::Entrust::price)
            .def_readonly("volume", &msg::data::Entrust::volume)
            .def_readonly("side", &msg::data::Entrust::side)
            .def_readonly("price_type", &msg::data::Entrust::price_type)
            .def_readonly("main_seq", &msg::data::Entrust::main_seq)
            .def_readonly("seq", &msg::data::Entrust::seq)
            .def("__repr__",
                 [](const msg::data::Entrust &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::Transaction>(m, "Transaction")
            .def_property_readonly("source_id", &msg::data::Transaction::get_source_id)
            .def_property_readonly("trading_day", &msg::data::Transaction::get_trading_day)
            .def_readonly("rcv_time", &msg::data::Transaction::rcv_time)
            .def_readonly("data_time", &msg::data::Transaction::data_time)
            .def_property_readonly("instrument_id", &msg::data::Transaction::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Transaction::get_exchange_id)
            .def_readonly("instrument_type", &msg::data::Transaction::instrument_type)
            .def_readonly("price", &msg::data::Transaction::price)
            .def_readonly("volume", &msg::data::Transaction::volume)
            .def_readonly("bid_no", &msg::data::Transaction::bid_no)
            .def_readonly("ask_no", &msg::data::Transaction::ask_no)
            .def_readonly("exec_type", &msg::data::Transaction::exec_type)
            .def_readonly("bs_flag", &msg::data::Transaction::bs_flag)
            .def_readonly("main_seq", &msg::data::Transaction::main_seq)
            .def_readonly("seq", &msg::data::Transaction::seq)
            .def("__repr__",
                 [](const msg::data::Transaction &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::OrderInput>(m, "OrderInput")
            .def_readonly("order_id", &msg::data::OrderInput::order_id)
            .def_readonly("instrument_type", &msg::data::OrderInput::instrument_type)
            .def_readonly("limit_price", &msg::data::OrderInput::limit_price)
            .def_readonly("frozen_price", &msg::data::OrderInput::frozen_price)
            .def_readonly("volume", &msg::data::OrderInput::volume)
            .def_readonly("side", &msg::data::OrderInput::side)
            .def_readonly("offset", &msg::data::OrderInput::offset)
            .def_readonly("price_type", &msg::data::OrderInput::price_type)
            .def_readonly("volume_condition", &msg::data::OrderInput::volume_condition)
            .def_readonly("time_condition", &msg::data::OrderInput::time_condition)
            .def_readonly("parent_id", &msg::data::OrderInput::parent_id)
            .def_property_readonly("instrument_id", &msg::data::OrderInput::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::OrderInput::get_exchange_id)
            .def_property_readonly("account_id", &msg::data::OrderInput::get_account_id)
            .def("__repr__",
                 [](const msg::data::OrderInput &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::Order>(m, "Order")
            .def_readonly("rcv_time", &msg::data::Order::rcv_time)
            .def_readonly("order_id", &msg::data::Order::order_id)
            .def_readonly("insert_time", &msg::data::Order::insert_time)
            .def_readonly("update_time", &msg::data::Order::update_time)
            .def_readonly("instrument_type", &msg::data::Order::instrument_type)
            .def_readonly("limit_price", &msg::data::Order::limit_price)
            .def_readonly("frozen_price", &msg::data::Order::frozen_price)
            .def_readonly("volume", &msg::data::Order::volume)
            .def_readonly("volume_traded", &msg::data::Order::volume_traded)
            .def_readonly("volume_left", &msg::data::Order::volume_left)
            .def_readonly("tax", &msg::data::Order::tax)
            .def_readonly("commission", &msg::data::Order::commission)
            .def_readonly("status", &msg::data::Order::status)
            .def_readonly("error_id", &msg::data::Order::error_id)
            .def_readonly("side", &msg::data::Order::side)
            .def_readonly("offset", &msg::data::Order::offset)
            .def_readonly("price_type", &msg::data::Order::price_type)
            .def_readonly("volume_condition", &msg::data::Order::volume_condition)
            .def_readonly("time_condition", &msg::data::Order::time_condition)
            .def_property_readonly("trading_day", &msg::data::Order::get_trading_day)
            .def_property_readonly("instrument_id", &msg::data::Order::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Order::get_exchange_id)
            .def_property_readonly("account_id", &msg::data::Order::get_account_id)
            .def_property_readonly("client_id", &msg::data::Order::get_client_id)
            .def_property_readonly("error_msg", &msg::data::Order::get_error_msg)
            .def("__repr__",
                 [](const msg::data::Order &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::OrderAction>(m, "OrderAction")
            .def_readonly("order_id", &msg::data::OrderAction::order_id)
            .def_readonly("order_action_id", &msg::data::OrderAction::order_action_id)
            .def_readonly("action_flag", &msg::data::OrderAction::action_flag)
            .def_readonly("price", &msg::data::OrderAction::price)
            .def_readonly("volume", &msg::data::OrderAction::volume)
            .def("__repr__",
                 [](const msg::data::OrderAction &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::Trade>(m, "Trade")
            .def_readonly("rcv_time", &msg::data::Trade::rcv_time)
            .def_readonly("id", &msg::data::Trade::id)
            .def_readonly("order_id", &msg::data::Trade::order_id)
            .def_readonly("parent_order_id", &msg::data::Trade::parent_order_id)
            .def_readonly("trade_time", &msg::data::Trade::trade_time)
            .def_readonly("instrument_type", &msg::data::Trade::instrument_type)
            .def_readonly("side", &msg::data::Trade::side)
            .def_readonly("offset", &msg::data::Trade::offset)
            .def_readonly("price", &msg::data::Trade::price)
            .def_readonly("volume", &msg::data::Trade::volume)
            .def_readonly("tax", &msg::data::Trade::tax)
            .def_readonly("commission", &msg::data::Trade::commission)
            .def_property_readonly("instrument_id", &msg::data::Trade::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Trade::get_exchange_id)
            .def_property_readonly("account_id", &msg::data::Trade::get_account_id)
            .def_property_readonly("client_id", &msg::data::Trade::get_client_id)
            .def("__repr__",
                 [](const msg::data::Trade &a)
                 {
                     return msg::data::to_string(a);
                 }
            );
    py::class_<msg::data::AccountInfo>(m, "AccountInfo")
            .def_readonly("rcv_time", &msg::data::AccountInfo::rcv_time)
            .def_readonly("update_time", &msg::data::AccountInfo::update_time)
            .def_readonly("initial_equity", &msg::data::AccountInfo::initial_equity)
            .def_readonly("static_equity", &msg::data::AccountInfo::static_equity)
            .def_readonly("dynamic_equity", &msg::data::AccountInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &msg::data::AccountInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &msg::data::AccountInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &msg::data::AccountInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &msg::data::AccountInfo::intraday_pnl_ratio)
            .def_readonly("avail", &msg::data::AccountInfo::avail)
            .def_readonly("market_value", &msg::data::AccountInfo::market_value)
            .def_readonly("margin", &msg::data::AccountInfo::margin)
            .def_readonly("accumulated_fee", &msg::data::AccountInfo::accumulated_fee)
            .def_readonly("intraday_fee", &msg::data::AccountInfo::intraday_fee)
            .def_readonly("frozen_cash", &msg::data::AccountInfo::frozen_cash)
            .def_readonly("frozen_margin", &msg::data::AccountInfo::frozen_margin)
            .def_readonly("frozen_fee", &msg::data::AccountInfo::frozen_fee)
            .def_readonly("position_pnl", &msg::data::AccountInfo::position_pnl)
            .def_readonly("close_pnl", &msg::data::AccountInfo::close_pnl)
            .def_property_readonly("trading_day", &msg::data::AccountInfo::get_trading_day)
            .def_property_readonly("account_id", &msg::data::AccountInfo::get_account_id)
            .def_property_readonly("broker_id", &msg::data::AccountInfo::get_broker_id)
            .def_property_readonly("source_id", &msg::data::AccountInfo::get_source_id)
            .def("__repr__",
                 [](const msg::data::AccountInfo &a)
                 {
                     return msg::nanomsg::to_string(a);
                 }
            );
    py::class_<msg::data::Position>(m, "Position")
            .def_readonly("rcv_time", &msg::data::Position::rcv_time)
            .def_readonly("update_time", &msg::data::Position::update_time)
            .def_readonly("instrument_type", &msg::data::Position::instrument_type)
            .def_readonly("direction", &msg::data::Position::direction)
            .def_readonly("volume", &msg::data::Position::volume)
            .def_readonly("yesterday_volume", &msg::data::Position::yesterday_volume)
            .def_readonly("frozen_total", &msg::data::Position::frozen_total)
            .def_readonly("frozen_yesterday", &msg::data::Position::frozen_yesterday)
            .def_readonly("last_price", &msg::data::Position::last_price)
            .def_readonly("open_price", &msg::data::Position::open_price)
            .def_readonly("cost_price", &msg::data::Position::cost_price)
            .def_readonly("close_price", &msg::data::Position::close_price)
            .def_readonly("pre_close_price", &msg::data::Position::pre_close_price)
            .def_readonly("settlement_price", &msg::data::Position::settlement_price)
            .def_readonly("pre_settlement_price", &msg::data::Position::pre_settlement_price)
            .def_readonly("margin", &msg::data::Position::margin)
            .def_readonly("position_pnl", &msg::data::Position::position_pnl)
            .def_readonly("close_pnl", &msg::data::Position::close_pnl)
            .def_readonly("realized_pnl", &msg::data::Position::realized_pnl)
            .def_readonly("unrealized_pnl", &msg::data::Position::unrealized_pnl)
            .def_property_readonly("trading_day", &msg::data::Position::get_trading_day)
            .def_property_readonly("instrument_id", &msg::data::Position::get_instrument_id)
            .def_property_readonly("exchange_id", &msg::data::Position::get_exchange_id)
            .def_property_readonly("account_id", &msg::data::Position::get_account_id)
            .def_property_readonly("client_id", &msg::data::Position::get_client_id)
            .def_property_readonly("open_date", &msg::data::Position::get_open_date)
            .def_property_readonly("expire_date", &msg::data::Position::get_expire_date)
            .def("__repr__",
                 [](const msg::data::Position &a)
                 {
                     return msg::nanomsg::to_string(a);
                 }
            );
    py::class_<msg::data::PortfolioInfo>(m, "PortfolioInfo")
            .def_readonly("update_time", &msg::data::PortfolioInfo::update_time)
            .def_readonly("initial_equity", &msg::data::PortfolioInfo::initial_equity)
            .def_readonly("static_equity", &msg::data::PortfolioInfo::static_equity)
            .def_readonly("dynamic_equity", &msg::data::PortfolioInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &msg::data::PortfolioInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &msg::data::PortfolioInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &msg::data::PortfolioInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &msg::data::PortfolioInfo::intraday_pnl_ratio)
            .def_property_readonly("trading_day", &msg::data::PortfolioInfo::get_trading_day)
            .def_property_readonly("client_id", &msg::data::PortfolioInfo::get_client_id)
            .def("__repr__",
                 [](const msg::data::PortfolioInfo &a)
                 {
                     return msg::nanomsg::to_string(a);
                 }
            );

    py::class_<strategy::Runner>(m, "Runner")
            .def(py::init<bool, kungfu::yijinjing::data::locator_ptr, const std::string &, const std::string &>())
            .def("run", &strategy::Runner::run)
            .def("add_strategy", &strategy::Runner::add_strategy)
            ;

    py::class_<strategy::Context, std::shared_ptr<strategy::Context>>(m, "Context")
            .def("now", &strategy::Context::now)
            .def("add_account", &strategy::Context::add_account)
            .def("subscribe", &strategy::Context::subscribe)
            .def("insert_market_order", &strategy::Context::insert_market_order)
            .def("insert_limit_order", &strategy::Context::insert_limit_order)
            .def("insert_fak_order", &strategy::Context::insert_fak_order)
            .def("insert_fok_order", &strategy::Context::insert_fok_order)
            .def("cancel_order", &strategy::Context::cancel_order)
            ;

    py::class_<strategy::Strategy, PyStrategy, strategy::Strategy_ptr>(m, "Strategy")
            .def(py::init())
            .def("pre_start", &strategy::Strategy::pre_start)
            .def("post_start", &strategy::Strategy::post_start)
            .def("pre_stop", &strategy::Strategy::pre_stop)
            .def("post_stop", &strategy::Strategy::post_stop)
            .def("on_switch_day", &strategy::Strategy::on_switch_day)
            .def("on_quote", &strategy::Strategy::on_quote)
            .def("on_entrust", &strategy::Strategy::on_entrust)
            .def("on_transaction", &strategy::Strategy::on_transaction)
            .def("on_order", &strategy::Strategy::on_order)
            .def("on_trade", &strategy::Strategy::on_trade)
            .def("on_algo_order_status", &strategy::Strategy::on_algo_order_status);

    py::class_<Calendar>(m, "Calendar")
            .def(py::init<const std::string &>())
            .def("current_trading_day", &Calendar::get_current_trading_day)
            .def("calculate_trading_day", &Calendar::calculate_trading_day);
}
