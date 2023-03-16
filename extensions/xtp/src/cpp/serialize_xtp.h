//
// Created by qlu on 2019/1/10.
//
#ifndef KUNGFU_SERIALIZE_XTP_H
#define KUNGFU_SERIALIZE_XTP_H
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(XTPTradeReport, order_xtp_id, order_client_id, ticker, market, local_order_id,
                                   exec_id, price, quantity, trade_time, trade_amount, report_index, order_exch_id,
                                   trade_type, side, position_effect, reserved1, reserved2, business_type, branch_pbu);
} // namespace nlohmann
namespace kungfu::wingchun::xtp {
template <typename T> std::string to_string(const T &ori) {
  nlohmann::json j;
  to_json(j, ori);
  return j.dump();
}
} // namespace kungfu::wingchun::xtp
#endif // KUNGFU_SERIALIZE_XTP_H
