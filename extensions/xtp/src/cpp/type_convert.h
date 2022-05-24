//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_XTP_EXT_TYPE_CONVERT_H
#define KUNGFU_XTP_EXT_TYPE_CONVERT_H

#include <cstdio>
#include <cstring>
#include <ctime>
#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/yijinjing/time.h>
#include <nlohmann/json.hpp>
#include <xtp_api_struct.h>

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;

namespace kungfu::wingchun::xtp {

template <typename T> inline void set_offset(T &t) {
  switch (t.side) {
  case Side::Buy:
    t.offset = Offset::Open;
    break;
  case Side::Sell:
    t.offset = Offset::Close;
    break;
  default:
    SPDLOG_ERROR("Invalidated kf_side : {} ", t.side);
    break;
  }
}

inline void to_json(nlohmann::json &j, const XTPOrderInfo &order_info) {
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

inline void to_json(nlohmann::json &j, const XTPTradeReport &trade_info) {
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

inline void to_json(nlohmann::json &j, const XTPOrderInsertInfo &order_info) {
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

inline void to_json(nlohmann::json &j, const XTPQueryAssetRsp &asset) {
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

inline void to_json(nlohmann::json &j, const XTPQueryStkPositionRsp &pos) {
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

inline void to_json(nlohmann::json &j, const XTPMD &md) {
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

  j["bid_qty"] = std::vector<int64_t>(md.bid_qty, std::end(md.bid_qty));
  j["ask_qty"] = std::vector<int64_t>(md.ask_qty, std::end(md.ask_qty));

  j["trades_count"] = md.trades_count;
  j["ticker_status"] = std::string(md.ticker_status);
  j["data_type"] = md.data_type;
}

inline void to_json(nlohmann::json &j, const XTPOrderCancelInfo &cancel_info) {
  j["order_cancel_xtp_id"] = cancel_info.order_cancel_xtp_id;
  j["order_xtp_id"] = cancel_info.order_xtp_id;
}

inline void to_json(nlohmann::json &j, const XTPTickByTickStruct &tick) {
  j["exchange_id"] = tick.exchange_id;
  j["ticker"] = std::string(tick.ticker);
  j["seq"] = tick.seq;
  j["data_time"] = tick.data_time;
  j["type"] = tick.type;
  if (tick.type == XTP_TBT_ENTRUST) {
    j["channel_no"] = tick.entrust.channel_no;
    j["seq"] = tick.entrust.seq;
    j["price"] = tick.entrust.price;
    j["qty"] = tick.entrust.qty;
    j["side"] = std::string(1, tick.entrust.side);
    j["ord_type"] = std::string(1, tick.entrust.ord_type);
  } else if (tick.type == XTP_TBT_TRADE) {
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

template <typename T> std::string to_string(const T &ori) {
  nlohmann::json j;
  to_json(j, ori);
  return j.dump();
}

inline XTP_PROTOCOL_TYPE get_xtp_protocol_type(const std::string &p) {
  if (p == "udp") {
    return XTP_PROTOCOL_UDP;
  } else {
    return XTP_PROTOCOL_TCP;
  }
}

inline int64_t nsec_from_xtp_timestamp(int64_t xtp_time) {
  std::tm result = {};
  result.tm_year = xtp_time / (int64_t)1e13 - 1900;
  result.tm_mon = xtp_time % (int64_t)1e13 / (int64_t)1e11 - 1;
  result.tm_mday = xtp_time % (int64_t)1e11 / (int64_t)1e9;
  result.tm_hour = xtp_time % (int64_t)1e9 / (int64_t)1e7;
  result.tm_min = xtp_time % (int)1e7 / (int)1e5;
  result.tm_sec = xtp_time % (int)1e5 / (int)1e3;
  int milli_sec = xtp_time % (int)1e3;
  std::time_t parsed_time = std::mktime(&result);
  int64_t nano = parsed_time * kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND;
  return parsed_time * kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND +
         milli_sec * kungfu::yijinjing::time_unit::NANOSECONDS_PER_MILLISECOND;
}

inline void from_xtp(const XTP_MARKET_TYPE &xtp_market_type, char *exchange_id) {
  if (xtp_market_type == XTP_MKT_SH_A) {
    strcpy(exchange_id, "SSE");
  } else if (xtp_market_type == XTP_MKT_SZ_A) {
    strcpy(exchange_id, "SZE");
  }
}

inline void to_xtp(XTP_MARKET_TYPE &xtp_market_type, const char *exchange_id) {
  if (!strcmp(exchange_id, "SSE")) {
    xtp_market_type = XTP_MKT_SH_A;
  } else if (!strcmp(exchange_id, "SZE")) {
    xtp_market_type = XTP_MKT_SZ_A;
  } else {
    xtp_market_type = XTP_MKT_UNKNOWN;
  }
}

inline std::string exchange_id_from_xtp(const XTP_EXCHANGE_TYPE ex) {
  if (ex == XTP_EXCHANGE_SH) {
    return EXCHANGE_SSE;
  } else if (ex == XTP_EXCHANGE_SZ) {
    return EXCHANGE_SZE;
  } else {
    return "Unknown";
  }
}

inline void from_xtp(const XTP_EXCHANGE_TYPE &xtp_exchange_type, char *exchange_id) {
  if (xtp_exchange_type == XTP_EXCHANGE_SH) {
    strcpy(exchange_id, "SSE");
  } else if (xtp_exchange_type == XTP_EXCHANGE_SZ) {
    strcpy(exchange_id, "SZE");
  }
}

inline void to_xtp(XTP_EXCHANGE_TYPE &xtp_exchange_type, char *exchange_id) {
  if (strcmp(exchange_id, "SSE") == 0) {
    xtp_exchange_type = XTP_EXCHANGE_SH;
  } else if (strcmp(exchange_id, "SZE") == 0) {
    xtp_exchange_type = XTP_EXCHANGE_SZ;
  } else {
    xtp_exchange_type = XTP_EXCHANGE_UNKNOWN;
  }
}

inline void from_xtp(const XTP_PRICE_TYPE &xtp_price_type, const XTP_MARKET_TYPE &xtp_exchange_type,
                     PriceType &price_type) {
  if (xtp_price_type == XTP_PRICE_LIMIT)
    price_type = PriceType::Limit;
  else if (xtp_price_type == XTP_PRICE_BEST5_OR_CANCEL)
    price_type = PriceType::FakBest5;
  else if (xtp_exchange_type == XTP_MKT_SH_A) {
    if (xtp_price_type == XTP_PRICE_BEST5_OR_LIMIT)
      price_type = PriceType::ReverseBest;
  } else if (xtp_exchange_type == XTP_MKT_SZ_A) {
    if (xtp_price_type == XTP_PRICE_BEST_OR_CANCEL)
      price_type = PriceType::Fak;
    else if (xtp_price_type == XTP_PRICE_FORWARD_BEST)
      price_type = PriceType::ForwardBest;
    else if (xtp_price_type == XTP_PRICE_REVERSE_BEST_LIMIT)
      price_type = PriceType::ReverseBest;
  } else
    price_type = PriceType::UnKnown;
}

inline void to_xtp(XTP_PRICE_TYPE &xtp_price_type, const PriceType &price_type, const char *exchange) {
  if (price_type == PriceType::Limit)
    xtp_price_type = XTP_PRICE_LIMIT;
  else if ((price_type == PriceType::Any) || (price_type == PriceType::FakBest5))
    xtp_price_type = XTP_PRICE_BEST5_OR_CANCEL;
  else if (strcmp(exchange, EXCHANGE_SSE) == 0) {
    if (price_type == PriceType::ReverseBest)
      xtp_price_type = XTP_PRICE_BEST5_OR_LIMIT;
  } else if (strcmp(exchange, EXCHANGE_SZE) == 0) {
    if (price_type == PriceType::Fak)
      xtp_price_type = XTP_PRICE_BEST_OR_CANCEL;
    else if (price_type == PriceType::ForwardBest)
      xtp_price_type = XTP_PRICE_FORWARD_BEST;
    else if (price_type == PriceType::ReverseBest)
      xtp_price_type = XTP_PRICE_REVERSE_BEST_LIMIT;
  } else
    xtp_price_type = XTP_PRICE_TYPE_UNKNOWN;
}

inline void from_xtp(const XTP_ORDER_STATUS_TYPE &xtp_order_status, OrderStatus &status) {
  if (xtp_order_status == XTP_ORDER_STATUS_INIT || xtp_order_status == XTP_ORDER_STATUS_NOTRADEQUEUEING) {
    status = OrderStatus::Pending;
  } else if (xtp_order_status == XTP_ORDER_STATUS_ALLTRADED) {
    status = OrderStatus::Filled;
  } else if (xtp_order_status == XTP_ORDER_STATUS_CANCELED) {
    status = OrderStatus::Cancelled;
  } else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDQUEUEING) {
    status = OrderStatus::PartialFilledActive;
  } else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDNOTQUEUEING) {
    status = OrderStatus::PartialFilledNotActive;
  } else if (xtp_order_status == XTP_ORDER_STATUS_REJECTED) {
    status = OrderStatus::Error;
  } else {
    status = OrderStatus::Unknown;
  }
}

inline void from_xtp(XTPQSI *ticker_info, Instrument &quote) {
  strcpy(quote.instrument_id, ticker_info->ticker);
  if (ticker_info->exchange_id == 1) {
    quote.exchange_id = EXCHANGE_SSE;
  } else if (ticker_info->exchange_id == 2) {
    quote.exchange_id = EXCHANGE_SZE;
  } else {
    quote.exchange_id = "false_id";
  }
  // strcpy(quote.product_id, ticker_info->ticker_name);
  memcpy(quote.product_id, ticker_info->ticker_name, 32); // commit by JC
  quote.instrument_type = get_instrument_type(quote.exchange_id, quote.instrument_id);
  quote.is_trading = true;
  quote.price_tick = ticker_info->price_tick;
}

inline void from_xtp(const XTP_SIDE_TYPE &xtp_side, Side &side) {
  if (xtp_side == XTP_SIDE_BUY) {
    side = Side::Buy;
  } else if (xtp_side == XTP_SIDE_SELL) {
    side = Side::Sell;
  }
}

inline void to_xtp(XTP_SIDE_TYPE &xtp_side, const Side &side) {
  if (side == Side::Buy) {
    xtp_side = XTP_SIDE_BUY;
  } else if (side == Side::Sell) {
    xtp_side = XTP_SIDE_SELL;
  }
}

inline void to_xtp(XTPMarketDataStruct &des, const Quote &ori) {
  // TODO
}

inline void from_xtp(const XTPMarketDataStruct &ori, Quote &des) {
  strcpy(des.source_id, SOURCE_XTP);
  des.data_time = nsec_from_xtp_timestamp(ori.data_time);
  strcpy(des.trading_day, yijinjing::time::strftime(des.data_time, KUNGFU_TRADING_DAY_FORMAT).c_str());
  strcpy(des.instrument_id, ori.ticker);
  from_xtp(ori.exchange_id, des.exchange_id);

  des.instrument_type = ori.data_type != XTP_MARKETDATA_OPTION ? get_instrument_type(des.exchange_id, des.instrument_id)
                                                               : InstrumentType::StockOption;

  des.last_price = ori.last_price;
  des.pre_settlement_price = ori.pre_settl_price;
  des.pre_close_price = ori.pre_close_price;
  des.open_price = ori.open_price;
  des.high_price = ori.high_price;
  des.low_price = ori.low_price;
  des.volume = ori.qty;
  des.turnover = ori.turnover;
  des.close_price = ori.close_price;
  des.settlement_price = ori.settl_price;
  des.upper_limit_price = ori.upper_limit_price;
  des.lower_limit_price = ori.lower_limit_price;

  memcpy(des.ask_price, ori.ask, sizeof(des.ask_price));
  memcpy(des.ask_volume, ori.ask_qty, sizeof(des.ask_price));
  memcpy(des.bid_price, ori.bid, sizeof(des.ask_price));
  memcpy(des.bid_volume, ori.bid_qty, sizeof(des.ask_price));
}

inline void from_xtp(const XTPMarketDataStruct &ori, int64_t *bid1_qty, int32_t bid1_count, int32_t max_bid1_count,
                     int64_t *ask1_qty, int32_t ask1_count, int32_t max_ask1_count, TopOfBook &des) {
  strcpy(des.source_id, SOURCE_XTP);
  des.data_time = nsec_from_xtp_timestamp(ori.data_time);
  strcpy(des.trading_day, yijinjing::time::strftime(des.data_time, KUNGFU_TRADING_DAY_FORMAT).c_str());
  strcpy(des.instrument_id, ori.ticker);
  from_xtp(ori.exchange_id, des.exchange_id);
  if (ori.data_type == XTP_MARKETDATA_OPTION) {
    des.instrument_type = InstrumentType::StockOption;
  } else {
    des.instrument_type = get_instrument_type(des.exchange_id, des.instrument_id);
  }
  des.last_price = ori.last_price;
  des.bid_price1 = ori.bid[0];
  des.bid_volume1 = ori.bid_qty[0];
  des.ask_price1 = ori.ask[0];
  des.ask_volume1 = ori.ask_qty[0];

  des.bid_count1 = bid1_count;
  des.max_bid_count1 = max_bid1_count;
  des.ask_count1 = ask1_count;
  des.max_ask_count1 = max_ask1_count;
  memcpy(des.bid_qty1, bid1_qty, bid1_count);
  memcpy(des.ask_qty1, ask1_qty, ask1_count);
}

inline void to_xtp(XTPOrderInsertInfo &des, const OrderInput &ori) {
  strcpy(des.ticker, ori.instrument_id);
  to_xtp(des.market, ori.exchange_id);
  des.price = ori.limit_price;
  des.quantity = ori.volume;
  to_xtp(des.side, ori.side);
  to_xtp(des.price_type, ori.price_type, ori.exchange_id);
  des.business_type = XTP_BUSINESS_TYPE_CASH;
}

inline void from_xtp(const XTPOrderInsertInfo &ori, OrderInput &des) {
  // TODO
}

inline void from_xtp(const XTPOrderInfo &ori, Order &des) {
  strcpy(des.instrument_id, ori.ticker);
  from_xtp(ori.market, des.exchange_id);
  from_xtp(ori.price_type, ori.market, des.price_type);
  des.volume = ori.quantity;
  des.volume_traded = ori.qty_traded;
  des.volume_left = ori.qty_left;
  des.limit_price = ori.price;
  from_xtp(ori.order_status, des.status);
  from_xtp(ori.side, des.side);
  //  des.offset = Offset::Open;
  set_offset(des);
  if (ori.business_type == XTP_BUSINESS_TYPE_CASH) {
    des.instrument_type = InstrumentType::Stock;
  }
  if (ori.update_time > 0) {
    des.update_time = nsec_from_xtp_timestamp(ori.update_time);
  }
}

inline void from_xtp(const XTPQueryOrderRsp &ori, HistoryOrder &des) {
  strcpy(des.instrument_id, ori.ticker);
  from_xtp(ori.market, des.exchange_id);
  from_xtp(ori.price_type, ori.market, des.price_type);
  des.volume = ori.quantity;
  des.volume_traded = ori.qty_traded;
  des.volume_left = ori.qty_left;
  des.limit_price = ori.price;
  from_xtp(ori.order_status, des.status);
  from_xtp(ori.side, des.side);
  //  des.offset = Offset::Open;
  set_offset(des);
  if (ori.business_type == XTP_BUSINESS_TYPE_CASH) {
    des.instrument_type = InstrumentType::Stock;
  }
  if (ori.update_time > 0) {
    des.update_time = nsec_from_xtp_timestamp(ori.update_time);
  }
}

inline void from_xtp(const XTPTradeReport &ori, Trade &des) {
  strcpy(des.instrument_id, ori.ticker);
  des.volume = ori.quantity;
  des.price = ori.price;
  from_xtp(ori.market, des.exchange_id);
  from_xtp(ori.side, des.side);
  //  des.offset = Offset::Open;
  set_offset(des);
  if (ori.business_type == XTP_BUSINESS_TYPE_CASH) {
    des.instrument_type = InstrumentType::Stock;
  }
  des.trade_time = nsec_from_xtp_timestamp(ori.trade_time);
}

inline void from_xtp(const XTPQueryTradeRsp &ori, HistoryTrade &des) {
  strcpy(des.instrument_id, ori.ticker);
  des.volume = ori.quantity;
  des.price = ori.price;
  from_xtp(ori.market, des.exchange_id);
  from_xtp(ori.side, des.side);
  //  des.offset = Offset::Open;
  set_offset(des);
  if (ori.business_type == XTP_BUSINESS_TYPE_CASH) {
    des.instrument_type = InstrumentType::Stock;
  }
  des.trade_time = nsec_from_xtp_timestamp(ori.trade_time);
}

inline void from_xtp(const XTPQueryStkPositionRsp &ori, Position &des) {
  strcpy(des.instrument_id, ori.ticker);
  from_xtp(ori.market, des.exchange_id);
  des.volume = ori.total_qty;
  des.yesterday_volume = ori.sellable_qty;
  des.avg_open_price = ori.avg_price;
  des.position_cost_price = ori.avg_price;
}

inline void from_xtp(const XTPQueryAssetRsp &ori, Asset &des) {
  strcpy(des.source_id, SOURCE_XTP);
  des.avail = ori.buying_power;
}

inline void from_xtp(const XTPTickByTickStruct &ori, Entrust &des) {
  from_xtp(ori.exchange_id, des.exchange_id);
  strcpy(des.instrument_id, ori.ticker);
  des.data_time = nsec_from_xtp_timestamp(ori.data_time);

  des.price = ori.entrust.price;
  des.volume = ori.entrust.qty;
  des.main_seq = ori.entrust.channel_no;
  des.seq = ori.entrust.seq;

  if (ori.entrust.ord_type == '1') {
    des.price_type = PriceType::Any;
  } else if (ori.entrust.ord_type == '2') {
    des.price_type = PriceType::Limit;
  } else if (ori.entrust.ord_type == 'U') {
    des.price_type = PriceType::ForwardBest;
  }
}

inline void from_xtp(const XTPTickByTickStruct &ori, Transaction &des) {
  strcpy(des.source_id, SOURCE_XTP);

  from_xtp(ori.exchange_id, des.exchange_id);
  strcpy(des.instrument_id, ori.ticker);
  des.data_time = nsec_from_xtp_timestamp(ori.data_time);

  des.main_seq = ori.trade.channel_no;
  des.seq = ori.trade.seq;

  des.price = ori.trade.price;
  des.volume = ori.trade.qty;

  des.bid_no = ori.trade.bid_no;
  des.ask_no = ori.trade.ask_no;

  switch (ori.trade.trade_flag) {
  case 'B': {
    des.bs_flag = BsFlag::Buy;
    des.exec_type = ExecType::Trade;
    break;
  }
  case 'S': {
    des.bs_flag = BsFlag::Sell;
    des.exec_type = ExecType::Trade;
    break;
  }
  case 'N': {
    des.bs_flag = BsFlag::Unknown;
    des.exec_type = ExecType::Trade;
    break;
  }
  case '4': {
    des.exec_type = ExecType::Cancel;
    break;
  }
  case 'F': {
    des.exec_type = ExecType::Trade;
    break;
  }
  default: {
    break;
  }
  }
}
} // namespace kungfu::wingchun::xtp
#endif // KUNGFU_XTP_EXT_TYPE_CONVERT_H
