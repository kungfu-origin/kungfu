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

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/strategy/strategy_util.h>
#include <kungfu/wingchun/calendar/calendar_service.h>
#include <kungfu/wingchun/util/journal_printer.h>

namespace py = pybind11;
namespace kf = kungfu;
namespace kfj = kungfu::journal;
namespace kff = kungfu::flying;

class PyStrategy : public kf::Strategy {
public:
    using kf::Strategy::Strategy; // Inherit constructors
    void pre_run() override { PYBIND11_OVERLOAD(void, kf::Strategy, pre_run, ); }
    void on_switch_day(const std::string& next_trading_day) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_switch_day, next_trading_day); }
    void on_quote(const kfj::Quote& quote) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_quote, quote); }
    void on_entrust(const kfj::Entrust& entrust) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_entrust, entrust); }
    void on_transaction(const kfj::Transaction& transaction) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_transaction, transaction); }
    void on_order(const kfj::Order& order) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_order, order); }
    void on_trade(const kfj::Trade& trade) override { PYBIND11_OVERLOAD(void, kf::Strategy, on_trade, trade); }
    void on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& event_msg) override {
        PYBIND11_OVERLOAD(void, kf::Strategy, on_algo_order_status, order_id, algo_type, event_msg);
    }
};

PYBIND11_MODULE(pywingchun, m)
{
    py::class_<kfj::Instrument>(m, "Instrument")
    .def_readonly("instrument_type", &kfj::Instrument::instrument_type)
    .def_property_readonly("instrument_id", &kfj::Instrument::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Instrument::get_exchange_id)
    .def("__repr__",
        [](const kfj::Instrument &a) {
            return kfj::to_string(a);
        }
    );
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
    .def("__repr__",
        [](const kfj::FutureInstrument &a) {
            return kfj::to_string(a);
        }
    );
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
    .def("__repr__",
        [](const kfj::Quote &a) {
            return kfj::to_string(a);
        }
    );
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
    .def("__repr__",
        [](const kfj::Entrust &a) {
            return kfj::to_string(a);
        }
    );
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
    .def("__repr__",
        [](const kfj::Transaction &a) {
            return kfj::to_string(a);
        }
    );
    py::class_<kfj::OrderInput>(m, "OrderInput")
    .def_readonly("order_id", &kfj::OrderInput::order_id)
    .def_readonly("instrument_type", &kfj::OrderInput::instrument_type)
    .def_readonly("limit_price", &kfj::OrderInput::limit_price)
    .def_readonly("frozen_price", &kfj::OrderInput::frozen_price)
    .def_readonly("volume", &kfj::OrderInput::volume)
    .def_readonly("side", &kfj::OrderInput::side)
    .def_readonly("offset", &kfj::OrderInput::offset)
    .def_readonly("price_type", &kfj::OrderInput::price_type)
    .def_readonly("volume_condition", &kfj::OrderInput::volume_condition)
    .def_readonly("time_condition", &kfj::OrderInput::time_condition)
    .def_readonly("parent_id", &kfj::OrderInput::parent_id)
    .def_property_readonly("instrument_id", &kfj::OrderInput::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::OrderInput::get_exchange_id)
    .def_property_readonly("account_id", &kfj::OrderInput::get_account_id)
    .def_property_readonly("client_id", &kfj::OrderInput::get_client_id)
    .def("__repr__",
        [](const kfj::OrderInput &a) {
            return kfj::to_string(a);
        }
    );
    py::class_<kfj::Order>(m, "Order")
    .def_readonly("rcv_time", &kfj::Order::rcv_time)
    .def_readonly("order_id", &kfj::Order::order_id)
    .def_readonly("insert_time", &kfj::Order::insert_time)
    .def_readonly("update_time", &kfj::Order::update_time)
    .def_readonly("instrument_type", &kfj::Order::instrument_type)
    .def_readonly("limit_price", &kfj::Order::limit_price)
    .def_readonly("frozen_price", &kfj::Order::frozen_price)
    .def_readonly("volume", &kfj::Order::volume)
    .def_readonly("volume_traded", &kfj::Order::volume_traded)
    .def_readonly("volume_left", &kfj::Order::volume_left)
    .def_readonly("tax", &kfj::Order::tax)
    .def_readonly("commission", &kfj::Order::commission)
    .def_readonly("status", &kfj::Order::status)
    .def_readonly("error_id", &kfj::Order::error_id)
    .def_readonly("side", &kfj::Order::side)
    .def_readonly("offset", &kfj::Order::offset)
    .def_readonly("price_type", &kfj::Order::price_type)
    .def_readonly("volume_condition", &kfj::Order::volume_condition)
    .def_readonly("time_condition", &kfj::Order::time_condition)
    .def_property_readonly("trading_day", &kfj::Order::get_trading_day)
    .def_property_readonly("instrument_id", &kfj::Order::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Order::get_exchange_id)
    .def_property_readonly("account_id", &kfj::Order::get_account_id)
    .def_property_readonly("client_id", &kfj::Order::get_client_id)
    .def_property_readonly("error_msg", &kfj::Order::get_error_msg)
    .def("__repr__",
        [](const kfj::Order &a) {
            return kfj::to_string(a);
        }
    );
    py::class_<kfj::OrderAction>(m, "OrderAction")
    .def_readonly("order_id", &kfj::OrderAction::order_id)
    .def_readonly("order_action_id", &kfj::OrderAction::order_action_id)
    .def_readonly("action_flag", &kfj::OrderAction::action_flag)
    .def_readonly("price", &kfj::OrderAction::price)
    .def_readonly("volume", &kfj::OrderAction::volume)
    .def("__repr__",
        [](const kfj::OrderAction &a) {
            return kfj::to_string(a);
        }
    );
    py::class_<kfj::Trade>(m, "Trade")
    .def_readonly("rcv_time", &kfj::Trade::rcv_time)
    .def_readonly("id", &kfj::Trade::id)
    .def_readonly("order_id", &kfj::Trade::order_id)
    .def_readonly("parent_order_id", &kfj::Trade::parent_order_id)
    .def_readonly("trade_time", &kfj::Trade::trade_time)
    .def_readonly("instrument_type", &kfj::Trade::instrument_type)
    .def_readonly("side", &kfj::Trade::side)
    .def_readonly("offset", &kfj::Trade::offset)
    .def_readonly("price", &kfj::Trade::price)
    .def_readonly("volume", &kfj::Trade::volume)
    .def_readonly("tax", &kfj::Trade::tax)
    .def_readonly("commission", &kfj::Trade::commission)
    .def_property_readonly("instrument_id", &kfj::Trade::get_instrument_id)
    .def_property_readonly("exchange_id", &kfj::Trade::get_exchange_id)
    .def_property_readonly("account_id", &kfj::Trade::get_account_id)
    .def_property_readonly("client_id", &kfj::Trade::get_client_id)
    .def("__repr__",
        [](const kfj::Trade &a) {
            return kfj::to_string(a);
        }
    );
    py::class_<kff::AccountInfo>(m, "AccountInfo")
    .def_readonly("rcv_time", &kff::AccountInfo::rcv_time)
    .def_readonly("update_time", &kff::AccountInfo::update_time)
    .def_readonly("account_type", &kff::AccountInfo::account_type)
    .def_readonly("initial_equity", &kff::AccountInfo::initial_equity)
    .def_readonly("static_equity", &kff::AccountInfo::static_equity)
    .def_readonly("dynamic_equity", &kff::AccountInfo::dynamic_equity)
    .def_readonly("accumulated_pnl", &kff::AccountInfo::accumulated_pnl)
    .def_readonly("accumulated_pnl_ratio", &kff::AccountInfo::accumulated_pnl_ratio)
    .def_readonly("intraday_pnl", &kff::AccountInfo::intraday_pnl)
    .def_readonly("intraday_pnl_ratio", &kff::AccountInfo::intraday_pnl_ratio)
    .def_readonly("avail", &kff::AccountInfo::avail)
    .def_readonly("market_value", &kff::AccountInfo::market_value)
    .def_readonly("margin", &kff::AccountInfo::margin)
    .def_readonly("accumulated_fee", &kff::AccountInfo::accumulated_fee)
    .def_readonly("intraday_fee", &kff::AccountInfo::intraday_fee)
    .def_readonly("frozen_cash", &kff::AccountInfo::frozen_cash)
    .def_readonly("frozen_margin", &kff::AccountInfo::frozen_margin)
    .def_readonly("frozen_fee", &kff::AccountInfo::frozen_fee)
    .def_readonly("position_pnl", &kff::AccountInfo::position_pnl)
    .def_readonly("close_pnl", &kff::AccountInfo::close_pnl)
    .def_property_readonly("trading_day", &kff::AccountInfo::get_trading_day)
    .def_property_readonly("account_id", &kff::AccountInfo::get_account_id)
    .def_property_readonly("broker_id", &kff::AccountInfo::get_broker_id)
    .def_property_readonly("source_id", &kff::AccountInfo::get_source_id)
    .def("__repr__",
        [](const kff::AccountInfo &a) {
            return kff::to_string(a);
        }
    );
    py::class_<kff::Position>(m, "Position")
    .def_readonly("rcv_time", &kff::Position::rcv_time)
    .def_readonly("update_time", &kff::Position::update_time)
    .def_readonly("instrument_type", &kff::Position::instrument_type)
    .def_readonly("direction", &kff::Position::direction)
    .def_readonly("volume", &kff::Position::volume)
    .def_readonly("yesterday_volume", &kff::Position::yesterday_volume)
    .def_readonly("frozen_total", &kff::Position::frozen_total)
    .def_readonly("frozen_yesterday", &kff::Position::frozen_yesterday)
    .def_readonly("last_price", &kff::Position::last_price)
    .def_readonly("open_price", &kff::Position::open_price)
    .def_readonly("cost_price", &kff::Position::cost_price)
    .def_readonly("close_price", &kff::Position::close_price)
    .def_readonly("pre_close_price", &kff::Position::pre_close_price)
    .def_readonly("settlement_price", &kff::Position::settlement_price)
    .def_readonly("pre_settlement_price", &kff::Position::pre_settlement_price)
    .def_readonly("margin", &kff::Position::margin)
    .def_readonly("position_pnl", &kff::Position::position_pnl)
    .def_readonly("close_pnl", &kff::Position::close_pnl)
    .def_readonly("realized_pnl", &kff::Position::realized_pnl)
    .def_readonly("unrealized_pnl", &kff::Position::unrealized_pnl)
    .def_property_readonly("trading_day", &kff::Position::get_trading_day)
    .def_property_readonly("instrument_id", &kff::Position::get_instrument_id)
    .def_property_readonly("exchange_id", &kff::Position::get_exchange_id)
    .def_property_readonly("account_id", &kff::Position::get_account_id)
    .def_property_readonly("client_id", &kff::Position::get_client_id)
    .def_property_readonly("open_date", &kff::Position::get_open_date)
    .def_property_readonly("expire_date", &kff::Position::get_expire_date)
    .def("__repr__",
        [](const kff::Position &a) {
            return kff::to_string(a);
        }
    );
    py::class_<kff::PortfolioInfo>(m, "PortfolioInfo")
    .def_readonly("update_time", &kff::PortfolioInfo::update_time)
    .def_readonly("initial_equity", &kff::PortfolioInfo::initial_equity)
    .def_readonly("static_equity", &kff::PortfolioInfo::static_equity)
    .def_readonly("dynamic_equity", &kff::PortfolioInfo::dynamic_equity)
    .def_readonly("accumulated_pnl", &kff::PortfolioInfo::accumulated_pnl)
    .def_readonly("accumulated_pnl_ratio", &kff::PortfolioInfo::accumulated_pnl_ratio)
    .def_readonly("intraday_pnl", &kff::PortfolioInfo::intraday_pnl)
    .def_readonly("intraday_pnl_ratio", &kff::PortfolioInfo::intraday_pnl_ratio)
    .def_property_readonly("trading_day", &kff::PortfolioInfo::get_trading_day)
    .def_property_readonly("client_id", &kff::PortfolioInfo::get_client_id)
    .def("__repr__",
        [](const kff::PortfolioInfo &a) {
            return kff::to_string(a);
        }
    );

    py::class_<kf::Strategy, PyStrategy>(m, "Strategy")
    .def(py::init<const std::string&>())
    .def_property_readonly("name", &kf::Strategy::get_name)
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
    .def("on_algo_order_status", &kf::Strategy::on_algo_order_status);

    py::class_<kf::CalendarService>(m, "CalendarService")
    .def(py::init())
    .def("current_day", &kf::CalendarService::get_current_day)
    .def("calculate_trading_day", &kf::CalendarService::calculate_trading_day)
    ;

    py::class_<kungfu::JournalPrinter>(m, "JournalPrinter")
    .def(py::init<const std::string&, const std::string&, int, bool, bool, bool, bool>())
    .def("run", &kungfu::JournalPrinter::run, py::arg("start_time"), py::arg("end_time"), py::arg("keep"))
    ;
}
