//
// Created by qlu on 2019/2/20.
//

#ifndef KUNGFU_CAST_H
#define KUNGFU_CAST_H

#include <string>
#include <cmath>
#include <cinttypes>
#include <nlohmann/json.hpp>

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/reqrsp.h>

#include <kungfu/wingchun/util/business_helper.h>

#define FORMAT_DOUBLE(x) rounded(x, 4)

namespace kungfu {namespace journal
{

    inline void from_json(const nlohmann::json& j, kungfu::journal::Instrument& instrument)
    {
        strcpy(instrument.exchange_id, j["exchange_id"].get<std::string>().c_str());
        strcpy(instrument.instrument_id, j["instrument_id"].get<std::string>().c_str());
        if (j.find("instrument_type") != j.end())
        {
            instrument.instrument_type = j["instrument_type"].get<std::string>()[0];
        }
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::FutureInstrument& instrument)
    {
        j["exchange_id"] = std::string(instrument.exchange_id);
        j["instrument_id"] = std::string(instrument.instrument_id);
        j["instrument_type"] = std::string(1, instrument.instrument_type);
        j["product_id"] = std::string(instrument.product_id);
        j["contract_multiplier"] = instrument.contract_multiplier;
        j["price_tick"] = instrument.price_tick;
        j["open_date"] = std::string(instrument.open_date);
        j["create_date"] = std::string(instrument.create_date);
        j["expire_date"] = std::string(instrument.expire_date);
        j["delivery_year"] = instrument.delivery_year;
        j["delivery_month"] = instrument.delivery_month;
        j["long_margin_ratio"] = instrument.long_margin_ratio;
        j["short_margin_ratio"] = instrument.short_margin_ratio;
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::Instrument& instrument)
    {
        j["exchange_id"] = std::string(instrument.exchange_id);
        j["instrument_id"] = std::string(instrument.instrument_id);
        if (instrument.instrument_type == kungfu::InstrumentTypeStock || instrument.instrument_type == kungfu::InstrumentTypeFuture || instrument.instrument_type == kungfu::InstrumentTypeBond)
        {
            j["instrument_type"] = std::string(1, instrument.instrument_type);
        }
        else
        {
            j["instrument_type"] = std::string(1, InstrumentTypeUnknown);
        }
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::Quote& quote)
    {
        j["trading_day"] = std::string(quote.trading_day);
        j["rcv_time"] = quote.rcv_time;
        j["data_time"] = quote.data_time;
        j["instrument_id"] = std::string(quote.instrument_id);
        j["exchange_id"] = std::string(quote.exchange_id);
        j["instrument_type"] = std::string(1, quote.instrument_type);

        j["pre_close_price"] = FORMAT_DOUBLE(quote.pre_close_price);
        j["pre_settlement_price"] = FORMAT_DOUBLE(quote.pre_settlement_price);

        j["last_price"] = FORMAT_DOUBLE(quote.last_price);
        j["volume"] = quote.volume;
        j["turnover"] = FORMAT_DOUBLE(quote.turnover);

        j["pre_open_interest"] = FORMAT_DOUBLE(quote.pre_open_interest);
        j["open_interest"] = FORMAT_DOUBLE(quote.open_interest);

        j["open_price"] = FORMAT_DOUBLE(quote.open_price);
        j["high_price"] = FORMAT_DOUBLE(quote.high_price);
        j["low_price"] = FORMAT_DOUBLE(quote.low_price);

        j["upper_limit_price"] = FORMAT_DOUBLE(quote.upper_limit_price);
        j["lower_limit_price"] = FORMAT_DOUBLE(quote.lower_limit_price);

        j["close_price"] = FORMAT_DOUBLE(quote.close_price);
        j["settlement_price"] = FORMAT_DOUBLE(quote.settlement_price);

        j["bid_price"] = std::vector<double>(quote.bid_price, std::end(quote.bid_price));
        j["ask_price"] = std::vector<double>(quote.ask_price, std::end(quote.ask_price));

        j["bid_volume"] = std::vector<int64_t >(quote.bid_volume, std::end(quote.bid_volume));
        j["ask_volume"] = std::vector<int64_t >(quote.ask_volume, std::end(quote.ask_volume));
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::OrderInput& input)
    {
        j["order_id"] = input.order_id;
        j["instrument_id"] = std::string(input.instrument_id);
        j["exchange_id"] = std::string(input.exchange_id);
        j["account_id"] = std::string(input.account_id);
        j["client_id"] = std::string(input.client_id);
        if (input.instrument_type != char(0))
        {
            j["instrument_type"] = std::string(1, input.instrument_type);
        }
        j["volume"] = input.volume;
        j["limit_price"] = FORMAT_DOUBLE(input.limit_price);
        j["frozen_price"] = FORMAT_DOUBLE(input.frozen_price);
        if (input.side != char(0))
        {
            j["side"] = std::string(1, input.side);
        }

        if (input.offset != char(0))
        {
            j["offset"] = std::string(1, input.offset);
        }

        if (input.price_type != char(0))
        {
            j["price_type"] = std::string(1, input.price_type);
        }

        if (input.volume_condition != char(0))
        {
            j["volume_condition"] = std::string(1, input.volume_condition);
        }

        if (input.time_condition != char(0))
        {
            j["time_condition"] = std::string(1, input.time_condition);
        }
        j["parent_id"] = input.parent_id;
    }

    inline void from_json(const nlohmann::json& j, kungfu::journal::OrderInput& input)
    {
        input = {};
        input.order_id = j["order_id"].get<uint64_t>();
        strncpy(input.instrument_id, j["instrument_id"].get<std::string>().c_str(), INSTRUMENT_ID_LEN);
        strncpy(input.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);
        strncpy(input.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
        strncpy(input.client_id, j["client_id"].get<std::string>().c_str(), CLIENT_ID_LEN);

        if (j.find("instrument_type") != j.end())
        {
            input.instrument_type = j["instrument_type"].get<std::string>()[0];
        }
        input.limit_price = j["limit_price"].get<double>();
        input.frozen_price = j["frozen_price"].get<double>();

        input.volume = j["volume"].get<int64_t>();

        if (j.find("side") != j.end())
        {
            input.side = j["side"].get<std::string>()[0];
        }

        if (j.find("offset") != j.end())
        {
            input.offset = j["offset"].get<std::string>()[0];
        }

        if (j.find("price_type") != j.end())
        {
            input.price_type = j["price_type"].get<std::string>()[0];
        }

        if (j.find("volume_condition") != j.end())
        {
            input.volume_condition = j["volume_condition"].get<std::string>()[0];
        }
        if (j.find("time_condition") != j.end())
        {
            input.time_condition = j["time_condition"].get<std::string>()[0];
        }
        if (j.find("parent_id") != j.end())
        {
            input.parent_id = j["parent_id"].get<uint64_t>();
        }
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::OrderInputRsp& input_rsp)
    {
        j["order_id"] = input_rsp.order_id;
        j["error_id"] = input_rsp.error_id;
        j["error_msg"] = input_rsp.error_msg;
    }

    inline void to_json(nlohmann::json& j, const kungfu::journal::OrderAction& action)
    {
        j["order_id"] = action.order_id;
        j["order_action_id"] = action.order_action_id;
        j["action_flag"] = std::string(1, action.action_flag);
        j["price"] = FORMAT_DOUBLE(action.price);
        j["volume"] = action.volume;
    }

    inline void from_json(const nlohmann::json& j, kungfu::journal::OrderAction& action)
    {
        action = {};
        if (j.find("order_id") != j.end())
        {
            action.order_id = j["order_id"].get<uint64_t>();
        }
        if (j.find("order_action_id") != j.end())
        {
            action.order_action_id = j["order_action_id"].get<uint64_t>();
        }
        if (j.find("action_flag") != j.end())
        {
            action.action_flag = j["action_flag"].get<std::string>()[0];
        }
        if (j.find("price") != j.end())
        {
            action.price = j["price"].get<double>();
        }
        if (j.find("volume") != j.end())
        {
            action.volume = j["volume"].get<int64_t>();
        }
    }

    inline void to_json(nlohmann::json& j, const journal::OrderActionRsp& action_rsp)
    {
        j["order_id"] = action_rsp.order_id;
        j["order_action_id"] = action_rsp.order_action_id;
        j["error_id"] = action_rsp.error_id;
        j["error_msg"] = action_rsp.error_msg;
    }

    inline void to_json(nlohmann::json& j, const journal::Order& order)
    {
        j["rcv_time"] = order.rcv_time;
        j["order_id"] = std::to_string(order.order_id);
        j["insert_time"] = order.insert_time;
        j["update_time"] = order.update_time;
        j["trading_day"] = std::string(order.trading_day);

        j["instrument_id"] = std::string(order.instrument_id);
        j["exchange_id"] = std::string(order.exchange_id);

        j["account_id"] = std::string(order.account_id);
        j["client_id"] = std::string(order.client_id);

        j["instrument_type"] = std::string(1, order.instrument_type);

        j["limit_price"] = FORMAT_DOUBLE(order.limit_price);
        j["frozen_price"] = FORMAT_DOUBLE(order.frozen_price);

        j["volume"] = order.volume;
        j["volume_traded"] = order.volume_traded;
        j["volume_left"] = order.volume_left;

        j["tax"] = FORMAT_DOUBLE(order.tax);
        j["commission"] = FORMAT_DOUBLE(order.commission);

        j["error_id"] = order.error_id;
        j["error_msg"] = std::string(order.error_msg);

        j["status"] = std::string(1, order.status);

        j["parent_id"] = order.parent_id;

        j["side"] = std::string(1, order.side);
        j["offset"] = std::string(1, order.offset);
        j["price_type"] = std::string(1, order.price_type);
        j["volume_condition"] = std::string(1, order.volume_condition);
        j["time_condition"] = std::string(1, order.time_condition);

    }

    inline void to_json(nlohmann::json& j, const journal::Trade& trade)
    {
        j["rcv_time"] = trade.rcv_time;

        j["id"] = trade.id;
        j["order_id"] = std::to_string(trade.order_id);

        j["trade_time"] = trade.trade_time;
        j["instrument_id"] = std::string(trade.instrument_id);
        j["exchange_id"] = std::string(trade.exchange_id);
        j["account_id"] = std::string(trade.account_id);
        j["client_id"] = std::string(trade.client_id);

        j["instrument_type"] = std::string(1, trade.instrument_type);

        j["side"] = std::string(1, trade.side);
        j["offset"] = std::string(1, trade.offset);

        j["price"] = FORMAT_DOUBLE(trade.price);
        j["volume"] = trade.volume;

        j["tax"] = FORMAT_DOUBLE(trade.tax);
        j["commission"] = FORMAT_DOUBLE(trade.commission);

    }

    inline void to_json(nlohmann::json& j, const journal::Entrust& entrust)
    {
        j["source_id"] = std::string(entrust.source_id);
        j["trading_day"] = std::string(entrust.trading_day);
        j["rcv_time"] = entrust.rcv_time;
        j["data_time"] = entrust.data_time;
        j["instrument_id"] = std::string(entrust.instrument_id);
        j["exchange_id"] = std::string(entrust.exchange_id);
        j["instrument_type"] = std::string(1, entrust.instrument_type);
        j["price"] = FORMAT_DOUBLE(entrust.price);
        j["volume"] = entrust.volume;
        j["side"] = std::string(1, entrust.side);
        j["price_type"] = std::string(1, entrust.price_type);
        j["main_seq"] = entrust.main_seq;
        j["seq"] = entrust.seq;
    }

    inline void to_json(nlohmann::json& j, const journal::Transaction& transaction)
    {
        j["source_id"] = std::string(transaction.source_id);
        j["trading_day"] = std::string(transaction.trading_day);
        j["rcv_time"] = transaction.rcv_time;
        j["data_time"] = transaction.data_time;
        j["instrument_id"] = std::string(transaction.instrument_id);
        j["exchange_id"] = std::string(transaction.exchange_id);
        j["instrument_type"] = std::string(1, transaction.instrument_type);
        j["price"] = FORMAT_DOUBLE(transaction.price);
        j["volume"] = transaction.volume;
        j["bid_no"] = transaction.bid_no;
        j["ask_no"] = transaction.ask_no;
        j["exec_type"] = std::string(1, transaction.exec_type);
        j["bs_flag"] = std::string(1, transaction.bs_flag);
        j["main_seq"] = transaction.main_seq;
        j["seq"] = transaction.seq;
    }

    template<typename T> std::string to_string(const T& ori)
    {
        nlohmann::json j;
        to_json(j, ori);
        return j.dump();
    }
}}

namespace kungfu {namespace flying
{
    inline void to_json(nlohmann::json& j, const kungfu::flying::AccountInfo& account)
    {
        j["rcv_time"] = account.rcv_time;
        j["update_time"] = account.update_time;
        j["trading_day"] = std::string(account.trading_day);
        j["account_id"] = std::string(account.account_id);
        j["type"] = std::string(1, account.account_type);
        j["broker_id"] = std::string(account.broker_id);
        j["source_id"] = std::string(account.source_id);
        j["initial_equity"] = FORMAT_DOUBLE(account.initial_equity);
        j["static_equity"] = FORMAT_DOUBLE(account.static_equity);
        j["dynamic_equity"] = FORMAT_DOUBLE(account.dynamic_equity);
        j["accumulated_pnl"] = FORMAT_DOUBLE(account.accumulated_pnl);
        j["accumulated_pnl_ratio"] = FORMAT_DOUBLE(account.accumulated_pnl_ratio);
        j["intraday_pnl"] = FORMAT_DOUBLE(account.intraday_pnl);
        j["intraday_pnl_ratio"] = FORMAT_DOUBLE(account.intraday_pnl_ratio);
        j["avail"] = FORMAT_DOUBLE(account.avail);
        j["market_value"] = FORMAT_DOUBLE(account.market_value);
        j["margin"] = FORMAT_DOUBLE(account.margin);
        j["accumulated_fee"] = FORMAT_DOUBLE(account.accumulated_fee);
        j["intraday_fee"] = FORMAT_DOUBLE(account.intraday_fee);
        j["frozen_cash"] = FORMAT_DOUBLE(account.frozen_cash);
        j["frozen_margin"] = FORMAT_DOUBLE(account.frozen_margin);
        j["frozen_fee"] = FORMAT_DOUBLE(account.frozen_fee);
        j["position_pnl"] = FORMAT_DOUBLE(account.position_pnl);
        j["close_pnl"] = FORMAT_DOUBLE(account.close_pnl);
    }

    inline void to_json(nlohmann::json& j, const kungfu::flying::Position& position)
    {
        j["rcv_time"] = position.rcv_time;
        j["update_time"] = position.update_time;
        j["instrument_id"] = std::string(position.instrument_id);
        j["instrument_type"] = std::string(1, position.instrument_type);
        j["exchange_id"] = std::string(position.exchange_id);
        j["account_id"] = std::string(position.account_id);
        j["client_id"] = std::string(position.client_id);
        j["direction"] = std::string(1, position.direction);
        j["volume"] = position.volume;
        j["yesterday_volume"] = position.yesterday_volume;
        j["frozen_total"] = position.frozen_total;
        j["frozen_yesterday"] = position.frozen_yesterday;
        j["last_price"] = FORMAT_DOUBLE(position.last_price);
        j["open_price"] = FORMAT_DOUBLE(position.open_price);
        j["cost_price"] = FORMAT_DOUBLE(position.cost_price);
        j["close_price"] = FORMAT_DOUBLE(position.close_price);
        j["pre_close_price"] = FORMAT_DOUBLE(position.pre_close_price);
        j["settlement_price"] = FORMAT_DOUBLE(position.settlement_price);
        j["pre_settlement_price"] = FORMAT_DOUBLE(position.pre_settlement_price);
        j["margin"] = FORMAT_DOUBLE(position.margin);
        j["position_pnl"] = FORMAT_DOUBLE(position.position_pnl);
        j["close_pnl"] = FORMAT_DOUBLE(position.close_pnl);
        j["realized_pnl"] = FORMAT_DOUBLE(position.realized_pnl);
        j["unrealized_pnl"] = FORMAT_DOUBLE(position.unrealized_pnl);
        j["open_date"] = std::string(position.open_date);
        j["expire_date"] = std::string(position.expire_date);
    }

    inline void to_json(nlohmann::json& j, const kungfu::flying::PortfolioInfo& pnl)
    {
        j["trading_day"] = std::string(pnl.trading_day);
        j["update_time"] = pnl.update_time;
        j["initial_equity"] = FORMAT_DOUBLE(pnl.initial_equity);
        j["static_equity"] = FORMAT_DOUBLE(pnl.static_equity);
        j["dynamic_equity"] = FORMAT_DOUBLE(pnl.dynamic_equity);
        j["accumulated_pnl"] = FORMAT_DOUBLE(pnl.accumulated_pnl);
        j["accumulated_pnl_ratio"] = FORMAT_DOUBLE(pnl.accumulated_pnl_ratio);
        j["intraday_pnl"] = FORMAT_DOUBLE(pnl.intraday_pnl);
        j["intraday_pnl_ratio"] = FORMAT_DOUBLE(pnl.intraday_pnl_ratio);
    }

    inline void to_json(nlohmann::json& j, const kungfu::flying::AlgoOrderInput& input)
    {
        j["order_id"] = input.order_id;
        j["client_id"] = input.client_id;
        j["algo_type"] = input.algo_type;
        j["input"] = input.input;
    }

    inline void from_json(const nlohmann::json& j, kungfu::flying::AlgoOrderInput& input)
    {
        input.order_id = j["order_id"];
        input.client_id = j["client_id"];
        input.algo_type = j["algo_type"];
        input.input = j["input"];
    }

    inline void to_json(nlohmann::json& j, const kungfu::flying::AlgoOrderStatus& status)
    {
        j["order_id"] = status.order_id;
        j["algo_type"] = status.algo_type;
        j["status"] = status.status;
    }

    inline void from_json(const nlohmann::json& j, kungfu::flying::AlgoOrderStatus& status)
    {
        status.order_id = j["order_id"];
        status.algo_type = j["algo_type"];
        status.status = j["status"];
    }

    inline void to_json(nlohmann::json& j, const kungfu::flying::AlgoOrderAction& action)
    {
        j["order_id"] = action.order_id;
        j["order_action_id"] = action.order_action_id;
        j["action"] = action.action;
    }

    inline void from_json(const nlohmann::json& j, kungfu::flying::AlgoOrderAction& action)
    {
        action.order_id = j["order_id"];
        action.order_action_id = j["order_action_id"];
        action.action = j["action"];
    }

    template<typename T> std::string to_string(const T& ori)
    {
        nlohmann::json j;
        to_json(j, ori);
        return j.dump();
    }
}}

namespace kungfu
{
    inline void to_json(nlohmann::json& j, const LoginRequest& login_req)
    {
        j["sender"] = login_req.sender;
        j["recipient"] = login_req.recipient;
    }

    inline void from_json(const nlohmann::json& j, LoginRequest& login_req)
    {
        login_req.sender = j["sender"];
        login_req.recipient = j["recipient"];
    }

    inline void to_json(nlohmann::json& j, const SubscribeRequest& sub_req)
    {
        j["sender"] = sub_req.sender;
        j["recipient"] = sub_req.recipient;
        j["is_level2"] = sub_req.is_level2;
        j["instruments"] = nlohmann::json::array();
        for (const auto& instrument : sub_req.instruments)
        {
            j["instruments"].push_back(instrument);
        }
    }

    inline void from_json(const nlohmann::json& j, SubscribeRequest& sub_req)
    {
        sub_req.sender = j["sender"];
        sub_req.recipient = j["recipient"];
        sub_req.is_level2 = j["is_level2"].get<bool>();
        sub_req.instruments.clear();
        for (const auto& instrument : j["instruments"])
        {
            sub_req.instruments.push_back(instrument);
        }
    }

    inline void to_json(nlohmann::json& j, const SubscribeRsp& rsp)
    {
        j["sender"] = rsp.sender;
    }
}

#endif //KUNGFU_CAST_H
