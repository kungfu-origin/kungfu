//
// Created by qlu on 2019/2/26.
//

#ifndef KUNGFU_SERIALIZE_H
#define KUNGFU_SERIALIZE_H

#include <nlohmann/json.hpp>
#include "xtp_api_struct.h"

namespace kungfu
{
    namespace xtp
    {
        inline void to_json(nlohmann::json& j, const XTPOrderInfo& order_info)
        {
            j["order_xtp_id"] = order_info.order_xtp_id;
            j["order_client_id"] = order_info.order_client_id;
            j["order_cancel_client_id"] = order_info.order_cancel_client_id;
            j["order_cancel_xtp_id"] = order_info.order_cancel_xtp_id;
            j["ticker"] = std::string(order_info.ticker);
            j["market"] = order_info.market;
            j["price"] = order_info.price;
            j["quantity"] = order_info.quantity;
            j["price_type"] = order_info.price_type;
            j["side"] = order_info.side;
            j["business_type"] = order_info.business_type;
            j["qty_traded"] = order_info.qty_traded;
            j["qty_left"] = order_info.qty_left;
            j["insert_time"] = order_info.insert_time;
            j["update_time"] = order_info.update_time;
            j["cancel_time"] = order_info.cancel_time;
            j["trade_amount"] = order_info.trade_amount;
            j["order_local_id"] = std::string(order_info.order_local_id);
            j["order_status"] = order_info.order_status;
            j["order_submit_status"] = order_info.order_submit_status;
            j["order_type"] = std::string(1, order_info.order_type);
        }

        inline void to_json(nlohmann::json& j, const XTPTradeReport& trade_info)
        {
            j["order_xtp_id"] = trade_info.order_xtp_id;
            j["order_client_id"] = trade_info.order_client_id;
            j["ticker"] = trade_info.ticker;
            j["market"] = trade_info.market;
            j["local_order_id"] = trade_info.local_order_id;
            j["exec_id"] = std::string(trade_info.exec_id);
            j["price"] = trade_info.price;
            j["quantity"] = trade_info.quantity;
            j["trade_time"] = trade_info.trade_time;
            j["trade_amount"] = trade_info.trade_amount;
            j["report_index"] = trade_info.report_index;
            j["order_exch_id"] = std::string(trade_info.order_exch_id);
            j["trade_type"] = std::string(1, trade_info.trade_type);
            j["business_type"] = trade_info.business_type;
            j["side"] = trade_info.side;
            j["position_effect"] = trade_info.position_effect;
        }

        inline void to_json(nlohmann::json& j, const XTPOrderInsertInfo& order_info)
        {
            j["order_xtp_id"] = order_info.order_xtp_id;
            j["order_client_id"] = order_info.order_client_id;
            j["ticker"] = order_info.ticker;
            j["market"] = order_info.market;
            j["price"] = order_info.price;
            j["stop_price"] = order_info.stop_price;
            j["quantity"] = order_info.quantity;
            j["price_type"] = order_info.price_type;
            j["side"] = order_info.side;
            j["position_effect"] = order_info.position_effect;
        }

        inline void to_json(nlohmann::json& j, const XTPQueryAssetRsp& asset)
        {
            j["total_asset"] = asset.total_asset;
            j["buying_power"] = asset.buying_power;
            j["security_asset"] = asset.security_asset;
            j["fund_buy_amount"] = asset.fund_buy_amount;
            j["fund_buy_fee"] = asset.fund_buy_fee;
            j["fund_sell_amount"] = asset.fund_buy_amount;
            j["fund_sell_fee"] = asset.fund_sell_fee;
            j["withholding_amount"] = asset.withholding_amount;
            j["account_type"] = asset.account_type;

            j["frozen_margin"] = asset.frozen_margin;
            j["frozen_exec_cash"] = asset.frozen_exec_cash;
            j["frozen_exec_fee"] = asset.frozen_exec_fee;
            j["pay_later"] = asset.pay_later;
            j["preadva_pay"] = asset.preadva_pay;
            j["orig_banlance"] = asset.orig_banlance;
            j["banlance"] = asset.banlance;
            j["deposit_withdraw"] = asset.deposit_withdraw;
            j["trade_netting"] = asset.trade_netting;
            j["captial_asset"] = asset.captial_asset;
            j["force_freeze_amount"] = asset.force_freeze_amount;
            j["preferred_amount"] = asset.preferred_amount;
        }

        inline void to_json(nlohmann::json& j, const XTPQueryStkPositionRsp& pos)
        {
            j["ticker"] = std::string(pos.ticker);
            j["ticker_name"] = std::string(pos.ticker_name);
            j["market"] = pos.market;
            j["total_qty"] = pos.total_qty;
            j["sellable_qty"] = pos.sellable_qty;
            j["avg_price"] = pos.avg_price;
            j["unrealized_pnl"] = pos.unrealized_pnl;
            j["yesterday_position"] = pos.yesterday_position;
            j["purchase_redeemable_qty"] = pos.purchase_redeemable_qty;
            j["position_direction"] = pos.position_direction;

        }

        inline void to_json(nlohmann::json& j, const XTPMD& md)
        {
            j["exchange_id"] = md.exchange_id;
            j["ticker"] = std::string(md.ticker);
            j["last_price"] = md.last_price;
            j["pre_close_price"] = md.pre_close_price;
            j["open_price"] = md.open_price;
            j["high_price"] = md.high_price;
            j["low_price"] = md.low_price;
            j["close_price"] = md.close_price;

            j["pre_total_long_positon"] = md.pre_total_long_positon;
            j["total_long_positon"] = md.total_long_positon;
            j["pre_settl_price"] = md.pre_settl_price;
            j["settl_price"] = md.settl_price;

            j["upper_limit_price"] = md.upper_limit_price;
            j["lower_limit_price"] = md.lower_limit_price;

            j["pre_delta"] = md.pre_delta;
            j["curr_delta"] = md.curr_delta;

            j["data_time"] = md.data_time;

            j["qty"] = md.qty;
            j["turnover"] = md.turnover;
            j["avg_price"] = md.avg_price;

            j["bid"] = std::vector<double>(md.bid, std::end(md.bid));
            j["ask"] = std::vector<double>(md.ask, std::end(md.ask));

            j["bid_qty"] = std::vector<double>(md.bid_qty, std::end(md.bid_qty));
            j["ask_qty"] = std::vector<double>(md.ask_qty, std::end(md.ask_qty));

            j["trades_count"] = md.trades_count;
            j["ticker_status"] = std::string(md.ticker_status);
            j["data_type"] = md.data_type;
        }

        inline void to_json(nlohmann::json& j, const XTPOrderCancelInfo& cancel_info)
        {
            j["order_cancel_xtp_id"] = cancel_info.order_cancel_xtp_id;
            j["order_xtp_id"] = cancel_info.order_xtp_id;
        }

        inline void to_json(nlohmann::json& j, const XTPTickByTickStruct& tick)
        {
            j["exchange_id"] = tick.exchange_id;
            j["ticker"] = std::string(tick.ticker);
            j["seq"] = tick.seq;
            j["data_time"] = tick.data_time;
            j["type"] = tick.type;
            if (tick.type == XTP_TBT_ENTRUST)
            {
                j["channel_no"] = tick.entrust.channel_no;
                j["seq"] = tick.entrust.seq;
                j["price"] = tick.entrust.price;
                j["qty"] = tick.entrust.qty;
                j["side"] = std::string(1, tick.entrust.side);
                j["ord_type"] = std::string(1, tick.entrust.ord_type);
            }
            else if (tick.type == XTP_TBT_TRADE)
            {
                j["channel_no"] = tick.trade.channel_no;
                j["seq"] = tick.trade.seq;
                j["price"] = tick.trade.price;
                j["qty"] = tick.trade.qty;
                j["money"] = tick.trade.money;
                j["bid_no"] = tick.trade.bid_no;
                j["ask_no"] = tick.trade.ask_no;
                j["trade_flag"] = std::string(1, tick.trade.trade_flag);
            }
        }

        template<typename T> std::string to_string(const T& ori)
        {
            nlohmann::json j;
            to_json(j, ori);
            return j.dump();
        }

    }
}


#endif //KUNGFU_SERIALIZE_H
