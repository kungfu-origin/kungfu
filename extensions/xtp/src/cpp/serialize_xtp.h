#ifndef KUNGFU_SERIALIZE_XTP_H
#define KUNGFU_SERIALIZE_XTP_H
#include <nlohmann/json.hpp>
#include <xtp_api_struct.h>
namespace nlohmann {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPQueryOrderRsp, order_xtp_id, order_client_id, order_cancel_client_id,
                                   order_cancel_xtp_id, ticker, market, price, quantity, price_type, side,
                                   position_effect, reserved1, reserved2, business_type, qty_traded, qty_left,
                                   insert_time, update_time, cancel_time, trade_amount, order_local_id, order_status,
                                   order_submit_status, order_type);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPOrderInsertInfo, order_xtp_id, order_client_id, ticker, market, price, stop_price,
                                   quantity, price_type, side, position_effect, reserved1, reserved2, business_type);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPTradeReport, order_xtp_id, order_client_id, ticker, market, local_order_id,
                                   exec_id, price, quantity, trade_time, trade_amount, report_index, order_exch_id,
                                   trade_type, side, position_effect, reserved1, reserved2, business_type, branch_pbu);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPOrderCancelInfo, order_cancel_xtp_id, order_xtp_id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPQueryStkPositionRsp, ticker, ticker_name, market, total_qty, sellable_qty,
                                   avg_price, unrealized_pnl, yesterday_position, purchase_redeemable_qty,
                                   position_direction, position_security_type, executable_option, lockable_position,
                                   executable_underlying, locked_position, usable_locked_position, profit_price,
                                   buy_cost, profit_cost, unknown);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPQueryAssetRsp, total_asset, buying_power, security_asset, fund_buy_amount,
                                   fund_buy_fee, fund_sell_amount, fund_sell_fee, withholding_amount, account_type,
                                   frozen_margin, frozen_exec_cash, frozen_exec_fee, pay_later, preadva_pay,
                                   orig_banlance, banlance, deposit_withdraw, trade_netting, captial_asset,
                                   force_freeze_amount, preferred_amount, repay_stock_aval_banlance,
                                   fund_order_data_charges, fund_cancel_data_charges, exchange_cur_risk_degree,
                                   company_cur_risk_degree, unknown);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPMarketDataStruct, exchange_id, ticker, last_price, pre_close_price, open_price,
                                   high_price, low_price, close_price, pre_total_long_positon, total_long_positon,
                                   pre_settl_price, settl_price, upper_limit_price, lower_limit_price, pre_delta,
                                   curr_delta, data_time, qty, turnover, avg_price, bid, ask, bid_qty, ask_qty,
                                   trades_count, ticker_status);
} // namespace nlohmann
namespace kungfu::wingchun::xtp {
template <typename T> std::string to_string(const T &ori) {
  nlohmann::json j;
  to_json(j, ori);
  return j.dump();
}
} // namespace kungfu::wingchun::xtp
#endif
