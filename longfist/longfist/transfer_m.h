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

/*
 * 150006320 -> '15:00:00'
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

#ifdef KUNGFU_LF_OES
inline void FromOesMktId(char* exchangeId, uint8 mktId)
{
    if (mktId == OES_MKT_ID_SH_A)
        strcpy(exchangeId, "SSE");
    else if (mktId == OES_MKT_ID_SZ_A)
        strcpy(exchangeId, "SZE");
    else
        strcpy(exchangeId, "None");
}

inline void ToOesMktId(uint8& mktId, const char* exchangeId)
{
    if (!strcmp(exchangeId, "SSE"))
        mktId = OES_MKT_ID_SH_A;
    else if (!strcmp(exchangeId, "SZE"))
        mktId = OES_MKT_ID_SZ_A;
    else
        mktId = -1;
}

inline void FromOesBsType(char& direction, uint8 bsType)
{
    if (bsType == 1)
        direction = LF_CHAR_Buy;
    else if (bsType == 2)
        direction = LF_CHAR_Sell;
}

inline void ToOesBsType(uint8& bsType, char direction)
{
    if (direction == LF_CHAR_Buy)
        bsType = 1;
    else if (direction == LF_CHAR_Sell)
        bsType = 2;
    else
        bsType = 0;
}

inline void FromOesOrdType(char& orderPriceType, uint8 ordType)
{
    if (ordType == OES_ORD_TYPE_LMT)
        orderPriceType = LF_CHAR_LimitPrice;
    else if (ordType == OES_ORD_TYPE_MTL_BEST_5 ||
             ordType == OES_ORD_TYPE_MTL_BEST ||
             ordType == OES_ORD_TYPE_FAK_BEST_5 ||
             ordType == OES_ORD_TYPE_MTL_SAMEPARTY_BEST)
        orderPriceType = LF_CHAR_BestPrice;
    else
        orderPriceType = LF_CHAR_AnyPrice;
}

inline void ToOesOrdType(uint8& ordType, char orderPriceType)
{
    if (orderPriceType == LF_CHAR_LimitPrice)
        ordType = OES_ORD_TYPE_LMT;
    else
        ordType = OES_ORD_TYPE_MTL_BEST;
}

inline void FromOesOrderStatus(char& orderStatus, uint8 ordStatus)
{
    if (ordStatus == OES_ORD_STATUS_NORMAL)
        orderStatus = LF_CHAR_NotTouched;
    else if (ordStatus == OES_ORD_STATUS_DECLARING)
        orderStatus = LF_CHAR_AcceptedNoReply;
    else if (ordStatus == OES_ORD_STATUS_DECLARED)
        orderStatus = LF_CHAR_NoTradeQueueing;
    else if (ordStatus == OES_ORD_STATUS_CANCEL_DONE)
        orderStatus = LF_CHAR_Canceled;
    else if (ordStatus == OES_ORD_STATUS_PARTIALLY_FILLED)
        orderStatus = LF_CHAR_PartTradedQueueing;
    else if (ordStatus == OES_ORD_STATUS_PARTIALLY_CANCELED)
        orderStatus = LF_CHAR_PartTradedNotQueueing;
    else if (ordStatus == OES_ORD_STATUS_CANCELED)
        orderStatus = LF_CHAR_Canceled;
    else if (ordStatus == OES_ORD_STATUS_FILLED)
        orderStatus = LF_CHAR_AllTraded;
    else
        orderStatus = LF_CHAR_Error;
}
#endif

#endif //LONGFIST_TRANSFER_M_H
