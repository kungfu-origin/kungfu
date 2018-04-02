/*****************************************************************************
 * Copyright [2017] [taurus.ai]
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

//
// Created by cjiang on 17/8/16.
//

#ifndef LONGFIST_TRANSFER_M_H
#define LONGFIST_TRANSFER_M_H

#include "LFConstants.h"
/*
 * 150006320 -> '15:00:06'
 */
inline void UpdateTimeFromMilSecInt(char* time_str, int time_int)
{
    time_int /= 1000;
    sprintf(time_str, "%02d:%02d:%02d", time_int / 10000, time_int / 100 % 100, time_int % 100);
}

/*
 * 150006320 -> 320
 */
inline void UpdateMillisecFromMilSecInt(int& mil_sec, int time_int)
{
    mil_sec = time_int % 1000;
}

/*
 * YYYYMMDDHHMMSSsss -> 'YYYYMMDD'
 */
inline void TradingDayFromLong(char* trading_day, long datetime)
{
    sprintf(trading_day, "%d", (int)(datetime / 1000000000));
}

/*
 * YYYYMMDDHHMMSSsss -> 'DD:HH:MM'
 */
inline void UpdateTimeFromLong(char* update_time, long datetime)
{
    int time_int = datetime % 1000000000;
    time_int /= 1000;
    sprintf(update_time, "%02d:%02d:%02d", time_int / 10000, time_int / 100 % 100, time_int % 100);
}

/*
 * YYYYMMDDHHMMSSsss -> sss
 */
inline void MillisecFromLong(int& mil_sec, long datetime)
{
    mil_sec = datetime % 1000;
}

#ifdef KUNGFU_LF_XTP
/**************************************************************/
/*                          XTP                               */
/**************************************************************/
inline void FromXtpExchangeId(char* exchangeId, XTP_EXCHANGE_TYPE exId)
{
    if (exId == XTP_EXCHANGE_SH)
        strcpy(exchangeId, EXCHANGE_SSE);
    else if (exId == XTP_EXCHANGE_SZ)
        strcpy(exchangeId, EXCHANGE_SZE);
    else
        strcpy(exchangeId, "None");
}

inline void ToXtpExchangeId(XTP_EXCHANGE_TYPE& exId, const char* exchangeId)
{
    if (!strcmp(exchangeId, EXCHANGE_SSE))
        exId = XTP_EXCHANGE_SH;
    else if (!strcmp(exchangeId, EXCHANGE_SZE))
        exId = XTP_EXCHANGE_SZ;
    else
        exId = XTP_EXCHANGE_UNKNOWN;
}

inline void FromXtpMarket(char* exchangeId, XTP_MARKET_TYPE marketId)
{
    if (marketId == XTP_MKT_SH_A)
        strcpy(exchangeId, EXCHANGE_SSE);
    else if (marketId == XTP_MKT_SZ_A)
        strcpy(exchangeId, EXCHANGE_SZE);
    else
        strcpy(exchangeId, "None");
}

inline void ToXtpMarket(XTP_MARKET_TYPE& marketId, const char* exchangeId)
{
    if (!strcmp(exchangeId, EXCHANGE_SSE))
        marketId = XTP_MKT_SH_A;
    else if (!strcmp(exchangeId, EXCHANGE_SZE))
        marketId = XTP_MKT_SZ_A;
    else
        marketId = XTP_MKT_UNKNOWN;
}

inline void FromXtpPosiDirection(char& posi_direction, XTP_MARKET_TYPE marketId)
{
    posi_direction = LF_CHAR_Net;
}

inline void FromXtpPriceType(char& order_price_type, XTP_PRICE_TYPE xtp_type)
{
    if (xtp_type == XTP_PRICE_LIMIT)
        order_price_type = LF_CHAR_LimitPrice;
    else
        order_price_type = LF_CHAR_BestPrice;
}

inline void ToXtpPriceType(XTP_PRICE_TYPE& xtp_type, char order_price_type)
{
    if (order_price_type == LF_CHAR_LimitPrice)
        xtp_type = XTP_PRICE_LIMIT;
    else
        xtp_type = XTP_PRICE_BEST5_OR_CANCEL;
}

inline void FromXtpDirection(char& direction, XTP_SIDE_TYPE xtp_side)
{
    if (xtp_side == XTP_SIDE_BUY || xtp_side == XTP_SIDE_BUY_OPEN || xtp_side == XTP_SIDE_BUY_CLOSE)
        direction = LF_CHAR_Buy;
    else if (xtp_side == XTP_SIDE_SELL || xtp_side == XTP_SIDE_SELL_OPEN || xtp_side == XTP_SIDE_SELL_CLOSE)
        direction = LF_CHAR_Sell;
    else
        direction = '\0';
}

inline void ToXtpDirection(XTP_SIDE_TYPE& xtp_side, char direction)
{
    if (direction == LF_CHAR_Buy)
        xtp_side = XTP_SIDE_BUY;
    else if (direction == LF_CHAR_Sell)
        xtp_side = XTP_SIDE_SELL;
    else
        xtp_side = XTP_SIDE_UNKNOWN;
}

inline void FromXtpOrderStatus(char& orderStatus, byte ordStatus)
{
    if (ordStatus == XTP_ORDER_STATUS_INIT)
        orderStatus = LF_CHAR_NotTouched;
    else if (ordStatus == XTP_ORDER_STATUS_UNKNOWN)
        orderStatus = LF_CHAR_Unknown;
    else if (ordStatus == XTP_ORDER_STATUS_PARTTRADEDQUEUEING)
        orderStatus = LF_CHAR_PartTradedQueueing;
    else if (ordStatus == XTP_ORDER_STATUS_NOTRADEQUEUEING)
        orderStatus = LF_CHAR_NoTradeQueueing;
    else if (ordStatus == XTP_ORDER_STATUS_PARTTRADEDNOTQUEUEING)
        orderStatus = LF_CHAR_PartTradedNotQueueing;
    else if (ordStatus == XTP_ORDER_STATUS_CANCELED)
        orderStatus = LF_CHAR_Canceled;
    else if (ordStatus == XTP_ORDER_STATUS_ALLTRADED)
        orderStatus = LF_CHAR_AllTraded;
    else if (ordStatus == XTP_ORDER_STATUS_REJECTED)
        orderStatus = LF_CHAR_Error;
    else
        orderStatus = LF_CHAR_Error;
}

#endif

#endif //LONGFIST_TRANSFER_M_H
