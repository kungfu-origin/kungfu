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
#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/watcher.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/strategy/runner.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace py = pybind11;

using namespace kungfu::yijinjing;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::msg::data;

class PyCalendar: public Calendar
{
public:
    const std::string get_current_trading_day() const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, Calendar, current_trading_day);
    }

    const std::string get_next_trading_day(const std::string& trading_day, int32_t delta_days) override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, Calendar, get_next_trading_day, trading_day, delta_days);
    }

    bool is_open(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) override
    {
        PYBIND11_OVERLOAD_PURE(bool, Calendar, is_open, tp, exchange_id);
    }

    const std::chrono::system_clock::time_point get_open_time(const std::string& trading_day, const std::string& exchange_id) override
    {
        PYBIND11_OVERLOAD_PURE(const std::chrono::system_clock::time_point, Calendar, get_open_time, trading_day, exchange_id);
    }

    const std::chrono::system_clock::time_point get_close_time(const std::string& trading_day, const std::string& exchange_id) override
    {
        PYBIND11_OVERLOAD_PURE(const std::chrono::system_clock::time_point, Calendar, get_close_time, trading_day, exchange_id);
    }

    const std::chrono::system_clock::time_point next_open(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) override
    {
        PYBIND11_OVERLOAD_PURE(const std::chrono::system_clock::time_point, Calendar, next_open, tp, exchange_id);
    }

    const std::chrono::system_clock::time_point next_close(const std::chrono::system_clock::time_point tp, const std::string& exchange_id) override
    {
        PYBIND11_OVERLOAD_PURE(const std::chrono::system_clock::time_point, Calendar, next_close, tp, exchange_id);
    }

};


class PyWatcher : public Watcher
{
public:
    using Watcher::Watcher;

    std::string handle_request(const std::string &msg) override
    {PYBIND11_OVERLOAD_PURE(std::string, Watcher, handle_request, msg) }

    void on_quote(event_ptr event, const Quote& quote) override
    {PYBIND11_OVERLOAD_PURE(void, Watcher, on_quote, event, quote) }

    void on_order(event_ptr event, const Order& order) override
    {PYBIND11_OVERLOAD_PURE(void, Watcher, on_order, event, order) }

    void on_trade(event_ptr event, const Trade& trade) override
    {PYBIND11_OVERLOAD_PURE(void, Watcher, on_trade, event, trade) }

    void on_assets(const AccountInfo& account_info, const std::vector<Position>& positions) override
    {PYBIND11_OVERLOAD_PURE(void, Watcher, on_assets, account_info, positions) }
};

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

    void on_quote(strategy::Context_ptr context, const Quote &quote) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_quote, context, quote); }

    void on_entrust(strategy::Context_ptr context, const Entrust &entrust) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_entrust, context, entrust); }

    void on_transaction(strategy::Context_ptr context, const Transaction &transaction) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_transaction, context, transaction); }

    void on_order(strategy::Context_ptr context, const Order &order) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_order, context, order); }

    void on_trade(strategy::Context_ptr context, const Trade &trade) override
    {PYBIND11_OVERLOAD(void, strategy::Strategy, on_trade, context, trade); }

    void on_algo_order_status(strategy::Context_ptr context, uint64_t order_id, const std::string &algo_type, const std::string &event_msg) override
    {
        PYBIND11_OVERLOAD(void, strategy::Strategy, on_algo_order_status, context, order_id, algo_type, event_msg);
    }
};

PYBIND11_MODULE(pywingchun, m)
{
    auto m_utils =  m.def_submodule("utils");
    m_utils.def("get_symbol_id", &kungfu::wingchun::strategy::get_symbol_id);
    m_utils.def("is_valid_price", &kungfu::wingchun::is_valid_price);

    auto m_constants = m.def_submodule("constants");

    py::enum_<kungfu::wingchun::InstrumentType>(m_constants, "InstrumentType", py::arithmetic())
        .value("Unknown", kungfu::wingchun::InstrumentType::Unknown)
        .value("Stock", kungfu::wingchun::InstrumentType::Stock)
        .value("Future", kungfu::wingchun::InstrumentType::Future)
        .value("Bond", kungfu::wingchun::InstrumentType::Bond)
        .value("StockOption", kungfu::wingchun::InstrumentType::StockOption)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::ExecType>(m_constants, "ExecType", py::arithmetic())
        .value("Unknown", kungfu::wingchun::ExecType::Unknown)
        .value("Cancel", kungfu::wingchun::ExecType::Cancel)
        .value("Trade", kungfu::wingchun::ExecType::Trade)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::Side>(m_constants, "Side", py::arithmetic())
        .value("Buy", kungfu::wingchun::Side::Buy)
        .value("Sell", kungfu::wingchun::Side::Sell)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::Offset>(m_constants, "Offset", py::arithmetic())
        .value("Open", kungfu::wingchun::Offset::Open)
        .value("Close", kungfu::wingchun::Offset::Close)
        .value("CloseToday", kungfu::wingchun::Offset::CloseToday)
        .value("CloseYesterday", kungfu::wingchun::Offset::CloseYesterday)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::OrderStatus>(m_constants, "OrderStatus", py::arithmetic())
        .value("Unknown", kungfu::wingchun::OrderStatus::Unknown)
        .value("Submitted", kungfu::wingchun::OrderStatus::Submitted)
        .value("Pending", kungfu::wingchun::OrderStatus::Pending)
        .value("Cancelled", kungfu::wingchun::OrderStatus::Cancelled)
        .value("Error", kungfu::wingchun::OrderStatus::Error)
        .value("Filled", kungfu::wingchun::OrderStatus::Filled)
        .value("PartialFilledNotActive", kungfu::wingchun::OrderStatus::PartialFilledNotActive)
        .value("PartialFilledActive", kungfu::wingchun::OrderStatus::PartialFilledActive)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::Direction>(m_constants, "Direction", py::arithmetic())
        .value("Long", kungfu::wingchun::Direction::Long)
        .value("Short", kungfu::wingchun::Direction::Short)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::PriceType>(m_constants, "PriceType", py::arithmetic())
        .value("Any", kungfu::wingchun::PriceType::Any)
        .value("Best", kungfu::wingchun::PriceType::Best)
        .value("Best5", kungfu::wingchun::PriceType::Best5)
        .value("Limit", kungfu::wingchun::PriceType::Limit)
        .value("ForwardBest", kungfu::wingchun::PriceType::ForwardBest)
        .value("ReverseBest", kungfu::wingchun::PriceType::ReverseBest)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::VolumeCondition>(m_constants, "VolumeCondition", py::arithmetic())
        .value("Any", kungfu::wingchun::VolumeCondition::Any)
        .value("Min", kungfu::wingchun::VolumeCondition::Min)
        .value("All", kungfu::wingchun::VolumeCondition::All)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::TimeCondition>(m_constants, "TimeCondition", py::arithmetic())
        .value("IOC", kungfu::wingchun::TimeCondition::IOC)
        .value("GFD", kungfu::wingchun::TimeCondition::GFD)
        .value("GTC", kungfu::wingchun::TimeCondition::GTC)
        .export_values()
        ;

    py::enum_<kungfu::wingchun::msg::type::MsgType>(m_constants, "MsgType", py::arithmetic())
        .value("Error", kungfu::wingchun::msg::type::MsgType::Error)
        .value("Quote", kungfu::wingchun::msg::type::MsgType::Quote)
        .value("Entrust", kungfu::wingchun::msg::type::MsgType::Entrust)
        .value("Transaction", kungfu::wingchun::msg::type::MsgType::Transaction)
        .value("OrderInput", kungfu::wingchun::msg::type::MsgType::OrderInput)
        .value("OrderAction", kungfu::wingchun::msg::type::MsgType::OrderAction)
        .value("Order", kungfu::wingchun::msg::type::MsgType::Order)
        .value("Trade", kungfu::wingchun::msg::type::MsgType::Trade)
        .value("Position", kungfu::wingchun::msg::type::MsgType::Position)
        .value("AccountInfo", kungfu::wingchun::msg::type::MsgType::AccountInfo)
        .value("AccountInfoByMin", kungfu::wingchun::msg::type::MsgType::AccountInfoByMin)
        .value("PortfolioByMin", kungfu::wingchun::msg::type::MsgType::PortfolioByMin)
        .value("PositionDetail", kungfu::wingchun::msg::type::MsgType::PositionDetail)
        .value("SubPortfolioInfo", kungfu::wingchun::msg::type::MsgType::SubPortfolioInfo)
        .value("ReqLogin", kungfu::wingchun::msg::type::MsgType::ReqLogin)
        .value("Subscribe", kungfu::wingchun::msg::type::MsgType::Subscribe)
        .value("ReqOrderInput", kungfu::wingchun::msg::type::MsgType::ReqOrderInput)
        .value("ReqOrderAction", kungfu::wingchun::msg::type::MsgType::ReqOrderAction)
        .value("RspLogin", kungfu::wingchun::msg::type::MsgType::RspLogin)
        .value("RspSubscribe", kungfu::wingchun::msg::type::MsgType::RspSubscribe)
        .value("RspOrderInput", kungfu::wingchun::msg::type::MsgType::RspOrderInput)
        .value("GatewayState", kungfu::wingchun::msg::type::MsgType::GatewayState)
        .value("AlgoOrderInput", kungfu::wingchun::msg::type::MsgType::AlgoOrderInput)
        .value("AlgoOrderStatus", kungfu::wingchun::msg::type::MsgType::AlgoOrderStatus)
        .value("AlgoOrderAction", kungfu::wingchun::msg::type::MsgType::AlgoOrderAction)
        .value("SwitchDay", kungfu::wingchun::msg::type::MsgType::SwitchDay)
        .value("RspTradingDay", kungfu::wingchun::msg::type::MsgType::RspTradingDay)
        .value("PositionEnd", kungfu::wingchun::msg::type::MsgType::PositionEnd)
        .value("PositionDetailEnd", kungfu::wingchun::msg::type::MsgType::PositionDetailEnd)
        .export_values()
        ;

    py::class_<Instrument>(m, "Instrument")
            .def_readonly("instrument_type", &Instrument::instrument_type)
            .def_property_readonly("instrument_id", &Instrument::get_instrument_id)
            .def_property_readonly("exchange_id", &Instrument::get_exchange_id)
            .def("__repr__",
                 [](const Instrument &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<FutureInstrument>(m, "FutureInstrument")
            .def_readonly("contract_multiplier", &FutureInstrument::contract_multiplier)
            .def_readonly("price_tick", &FutureInstrument::price_tick)
            .def_readonly("delivery_year", &FutureInstrument::delivery_year)
            .def_readonly("delivery_month", &FutureInstrument::delivery_month)
            .def_readonly("is_trading", &FutureInstrument::is_trading)
            .def_readonly("long_margin_ratio", &FutureInstrument::long_margin_ratio)
            .def_readonly("short_margin_ratio", &FutureInstrument::short_margin_ratio)
            .def_property_readonly("product_id", &FutureInstrument::get_product_id)
            .def_property_readonly("open_date", &FutureInstrument::get_open_date)
            .def_property_readonly("create_date", &FutureInstrument::get_create_date)
            .def_property_readonly("expire_date", &FutureInstrument::get_expire_date)
            .def("__repr__",
                 [](const FutureInstrument &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<Quote>(m, "Quote")
            .def_property_readonly("source_id", &Quote::get_source_id)
            .def_property_readonly("trading_day", &Quote::get_trading_day)
            .def_readonly("data_time", &Quote::data_time)
            .def_property_readonly("instrument_id", &Quote::get_instrument_id)
            .def_property_readonly("exchange_id", &Quote::get_exchange_id)
            .def_readonly("instrument_type", &Quote::instrument_type)
            .def_readonly("pre_close_price", &Quote::pre_close_price)
            .def_readonly("pre_settlement_price", &Quote::pre_settlement_price)
            .def_readonly("last_price", &Quote::last_price)
            .def_readonly("volume", &Quote::volume)
            .def_readonly("turnover", &Quote::turnover)
            .def_readonly("pre_open_interest", &Quote::pre_open_interest)
            .def_readonly("open_interest", &Quote::open_interest)
            .def_readonly("open_price", &Quote::open_price)
            .def_readonly("high_price", &Quote::high_price)
            .def_readonly("low_price", &Quote::low_price)
            .def_readonly("upper_limit_price", &Quote::upper_limit_price)
            .def_readonly("lower_limit_price", &Quote::lower_limit_price)
            .def_readonly("close_price", &Quote::close_price)
            .def_readonly("settlement_price", &Quote::settlement_price)
            .def_property_readonly("bid_price", &Quote::get_bid_price)
            .def_property_readonly("ask_price", &Quote::get_ask_price)
            .def_property_readonly("bid_volume", &Quote::get_bid_volume)
            .def_property_readonly("ask_volume", &Quote::get_ask_volume)
            .def("__repr__",
                 [](const Quote &a)
                 {
                     return to_string(a);
                 }
            );

    py::class_<Entrust>(m, "Entrust")
            .def_property_readonly("source_id", &Entrust::get_source_id)
            .def_property_readonly("trading_day", &Entrust::get_trading_day)
            .def_readonly("data_time", &Entrust::data_time)
            .def_property_readonly("instrument_id", &Entrust::get_instrument_id)
            .def_property_readonly("exchange_id", &Entrust::get_exchange_id)
            .def_readonly("instrument_type", &Entrust::instrument_type)
            .def_readonly("price", &Entrust::price)
            .def_readonly("volume", &Entrust::volume)
            .def_readonly("side", &Entrust::side)
            .def_readonly("price_type", &Entrust::price_type)
            .def_readonly("main_seq", &Entrust::main_seq)
            .def_readonly("seq", &Entrust::seq)
            .def("__repr__",
                 [](const Entrust &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<Transaction>(m, "Transaction")
            .def_property_readonly("source_id", &Transaction::get_source_id)
            .def_property_readonly("trading_day", &Transaction::get_trading_day)
            .def_readonly("data_time", &Transaction::data_time)
            .def_property_readonly("instrument_id", &Transaction::get_instrument_id)
            .def_property_readonly("exchange_id", &Transaction::get_exchange_id)
            .def_readonly("instrument_type", &Transaction::instrument_type)
            .def_readonly("price", &Transaction::price)
            .def_readonly("volume", &Transaction::volume)
            .def_readonly("bid_no", &Transaction::bid_no)
            .def_readonly("ask_no", &Transaction::ask_no)
            .def_readonly("exec_type", &Transaction::exec_type)
            .def_readonly("bs_flag", &Transaction::bs_flag)
            .def_readonly("main_seq", &Transaction::main_seq)
            .def_readonly("seq", &Transaction::seq)
            .def("__repr__",
                 [](const Transaction &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<OrderInput>(m, "OrderInput")
            .def_readonly("order_id", &OrderInput::order_id)
            .def_readonly("instrument_type", &OrderInput::instrument_type)
            .def_readonly("limit_price", &OrderInput::limit_price)
            .def_readonly("frozen_price", &OrderInput::frozen_price)
            .def_readonly("volume", &OrderInput::volume)
            .def_readonly("side", &OrderInput::side)
            .def_readonly("offset", &OrderInput::offset)
            .def_readonly("price_type", &OrderInput::price_type)
            .def_readonly("volume_condition", &OrderInput::volume_condition)
            .def_readonly("time_condition", &OrderInput::time_condition)
            .def_readonly("parent_id", &OrderInput::parent_id)
            .def_property_readonly("instrument_id", &OrderInput::get_instrument_id)
            .def_property_readonly("exchange_id", &OrderInput::get_exchange_id)
            .def_property_readonly("account_id", &OrderInput::get_account_id)
            .def("__repr__",
                 [](const OrderInput &a)
                 {
                     return to_string(a);
                 }
            );

    py::class_<Order>(m, "Order")
            .def_readonly("order_id", &Order::order_id)
            .def_readonly("insert_time", &Order::insert_time)
            .def_readonly("update_time", &Order::update_time)
            .def_readonly("instrument_type", &Order::instrument_type)
            .def_readonly("limit_price", &Order::limit_price)
            .def_readonly("frozen_price", &Order::frozen_price)
            .def_readonly("volume", &Order::volume)
            .def_readonly("volume_traded", &Order::volume_traded)
            .def_readonly("volume_left", &Order::volume_left)
            .def_readonly("tax", &Order::tax)
            .def_readonly("commission", &Order::commission)
            .def_readonly("status", &Order::status)
            .def_readonly("error_id", &Order::error_id)
            .def_readonly("side", &Order::side)
            .def_readonly("offset", &Order::offset)
            .def_readonly("price_type", &Order::price_type)
            .def_readonly("volume_condition", &Order::volume_condition)
            .def_readonly("time_condition", &Order::time_condition)
            .def_property_readonly("trading_day", &Order::get_trading_day)
            .def_property_readonly("instrument_id", &Order::get_instrument_id)
            .def_property_readonly("exchange_id", &Order::get_exchange_id)
            .def_property_readonly("account_id", &Order::get_account_id)
            .def_property_readonly("client_id", &Order::get_client_id)
            .def_property_readonly("error_msg", &Order::get_error_msg)
            .def("__repr__",
                 [](const Order &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<OrderAction>(m, "OrderAction")
            .def_readonly("order_id", &OrderAction::order_id)
            .def_readonly("order_action_id", &OrderAction::order_action_id)
            .def_readonly("action_flag", &OrderAction::action_flag)
            .def_readonly("price", &OrderAction::price)
            .def_readonly("volume", &OrderAction::volume)
            .def("__repr__",
                 [](const OrderAction &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<Trade>(m, "Trade")
            .def_readonly("trade_id", &Trade::trade_id)
            .def_readonly("order_id", &Trade::order_id)
            .def_readonly("parent_order_id", &Trade::parent_order_id)
            .def_readonly("trade_time", &Trade::trade_time)
            .def_readonly("instrument_type", &Trade::instrument_type)
            .def_readonly("side", &Trade::side)
            .def_readonly("offset", &Trade::offset)
            .def_readonly("price", &Trade::price)
            .def_readonly("volume", &Trade::volume)
            .def_readonly("tax", &Trade::tax)
            .def_readonly("commission", &Trade::commission)
            .def_property_readonly("instrument_id", &Trade::get_instrument_id)
            .def_property_readonly("exchange_id", &Trade::get_exchange_id)
            .def_property_readonly("account_id", &Trade::get_account_id)
            .def_property_readonly("client_id", &Trade::get_client_id)
            .def("__repr__",
                 [](const Trade &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<AccountInfo>(m, "AccountInfo")
            .def_readonly("update_time", &AccountInfo::update_time)
            .def_readonly("initial_equity", &AccountInfo::initial_equity)
            .def_readonly("static_equity", &AccountInfo::static_equity)
            .def_readonly("dynamic_equity", &AccountInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &AccountInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &AccountInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &AccountInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &AccountInfo::intraday_pnl_ratio)
            .def_readonly("avail", &AccountInfo::avail)
            .def_readonly("market_value", &AccountInfo::market_value)
            .def_readonly("margin", &AccountInfo::margin)
            .def_readonly("accumulated_fee", &AccountInfo::accumulated_fee)
            .def_readonly("intraday_fee", &AccountInfo::intraday_fee)
            .def_readonly("frozen_cash", &AccountInfo::frozen_cash)
            .def_readonly("frozen_margin", &AccountInfo::frozen_margin)
            .def_readonly("frozen_fee", &AccountInfo::frozen_fee)
            .def_readonly("position_pnl", &AccountInfo::position_pnl)
            .def_readonly("close_pnl", &AccountInfo::close_pnl)
            .def_property_readonly("trading_day", &AccountInfo::get_trading_day)
            .def_property_readonly("account_id", &AccountInfo::get_account_id)
            .def_property_readonly("broker_id", &AccountInfo::get_broker_id)
            .def_property_readonly("source_id", &AccountInfo::get_source_id)
            .def("__repr__",
                 [](const AccountInfo &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<Position>(m, "Position")
            .def_readonly("update_time", &Position::update_time)
            .def_readonly("instrument_type", &Position::instrument_type)
            .def_readonly("direction", &Position::direction)
            .def_readonly("volume", &Position::volume)
            .def_readonly("yesterday_volume", &Position::yesterday_volume)
            .def_readonly("frozen_total", &Position::frozen_total)
            .def_readonly("frozen_yesterday", &Position::frozen_yesterday)
            .def_readonly("last_price", &Position::last_price)
            .def_readonly("open_price", &Position::open_price)
            .def_readonly("cost_price", &Position::cost_price)
            .def_readonly("close_price", &Position::close_price)
            .def_readonly("pre_close_price", &Position::pre_close_price)
            .def_readonly("settlement_price", &Position::settlement_price)
            .def_readonly("pre_settlement_price", &Position::pre_settlement_price)
            .def_readonly("margin", &Position::margin)
            .def_readonly("position_pnl", &Position::position_pnl)
            .def_readonly("close_pnl", &Position::close_pnl)
            .def_readonly("realized_pnl", &Position::realized_pnl)
            .def_readonly("unrealized_pnl", &Position::unrealized_pnl)
            .def_property_readonly("trading_day", &Position::get_trading_day)
            .def_property_readonly("instrument_id", &Position::get_instrument_id)
            .def_property_readonly("exchange_id", &Position::get_exchange_id)
            .def_property_readonly("account_id", &Position::get_account_id)
            .def_property_readonly("client_id", &Position::get_client_id)
            .def_property_readonly("open_date", &Position::get_open_date)
            .def_property_readonly("expire_date", &Position::get_expire_date)
            .def("__repr__",
                 [](const Position &a)
                 {
                     return to_string(a);
                 }
            );
    py::class_<PortfolioInfo>(m, "PortfolioInfo")
            .def_readonly("update_time", &PortfolioInfo::update_time)
            .def_readonly("initial_equity", &PortfolioInfo::initial_equity)
            .def_readonly("static_equity", &PortfolioInfo::static_equity)
            .def_readonly("dynamic_equity", &PortfolioInfo::dynamic_equity)
            .def_readonly("accumulated_pnl", &PortfolioInfo::accumulated_pnl)
            .def_readonly("accumulated_pnl_ratio", &PortfolioInfo::accumulated_pnl_ratio)
            .def_readonly("intraday_pnl", &PortfolioInfo::intraday_pnl)
            .def_readonly("intraday_pnl_ratio", &PortfolioInfo::intraday_pnl_ratio)
            .def_property_readonly("trading_day", &PortfolioInfo::get_trading_day)
            .def_property_readonly("client_id", &PortfolioInfo::get_client_id)
            .def("__repr__",
                 [](const PortfolioInfo &a)
                 {
                     return to_string(a);
                 }
            );

    py::class_<Watcher, PyWatcher>(m, "Watcher")
            .def(py::init<data::locator_ptr, data::mode, bool>())
            .def_property_readonly("io_device", &Watcher::get_io_device)
            .def("get_location", &Watcher::get_location)
            .def("publish", &Watcher::publish)
            .def("handle_request", &Watcher::handle_request)
            .def("on_quote", &Watcher::on_quote)
            .def("on_order", &Watcher::on_order)
            .def("on_trade", &Watcher::on_trade)
            .def("on_assets", &Watcher::on_assets)
            .def("set_begin_time", &Watcher::set_begin_time)
            .def("set_end_time", &Watcher::set_end_time)
            .def("run", &Watcher::run);

    py::class_<strategy::Runner, kungfu::practice::apprentice, std::shared_ptr<strategy::Runner>>(m, "Runner")
            .def(py::init<kungfu::yijinjing::data::locator_ptr, const std::string &, const std::string &, data::mode, bool>())
            .def("set_begin_time", &strategy::Runner::set_begin_time)
            .def("set_end_time", &strategy::Runner::set_end_time)
            .def("run", &strategy::Runner::run)
            .def("add_strategy", &strategy::Runner::add_strategy);

    py::class_<strategy::Context, std::shared_ptr<strategy::Context>>(m, "Context")
            .def("now", &strategy::Context::now)
            .def("add_account", &strategy::Context::add_account)
            .def("subscribe", &strategy::Context::subscribe)
            .def("insert_market_order", &strategy::Context::insert_market_order)
            .def("insert_limit_order", &strategy::Context::insert_limit_order)
            .def("insert_fak_order", &strategy::Context::insert_fak_order)
            .def("insert_fok_order", &strategy::Context::insert_fok_order)
            .def("cancel_order", &strategy::Context::cancel_order);

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

    py::class_<Calendar, std::shared_ptr<Calendar>, PyCalendar>(m, "Calendar")
            .def(py::init())
            .def("get_current_trading_day", &Calendar::get_current_trading_day)
            .def("get_next_trading_day", &Calendar::get_next_trading_day)
            .def("is_open", &Calendar::is_open)
            .def("get_open_time", &Calendar::get_open_time)
            .def("get_close_time", &Calendar::get_close_time)
            .def("next_open", &Calendar::next_open)
            .def("next_close", &Calendar::next_close)
            ;
}
