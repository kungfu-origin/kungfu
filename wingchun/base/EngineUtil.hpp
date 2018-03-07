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

/**
 * EngineUtil
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 * Some useful / common util functions.
 * Some definitions
 */

#ifndef WINGCHUN_ENGINEUTIL_H
#define WINGCHUN_ENGINEUTIL_H

#include "WC_DECLARE.h"
#include "longfist/LFConstants.h"
#include "longfist/LFUtils.h"

/*                                                                    +-------------+
 *                                                          (Fail) -> | ConnectFail |
 *                                                        /           +-------------+
 * +------+                                              /               |
 * | Idle | - (page engine control / auto) -> connecting    <--  (auto retry) $RECONNECT_UPPER_LIMIT
 * +------+                                            |
 *                                                   (Success)                            +-----------+
 *                                                     |                        (Fail) -> | LoginFail |
 *                                                     v                       /          +-----------+
 *                                                 Connected - (auto login) ->
 *                                                                             \             +----------+
 *                                                                              (Success) -> | LoginSuc |
 *                                                                                           +----------+
 */
//////////////////////////////////////////
/// (string) WingchunEngineStatusMsg
//////////////////////////////////////////
#define WC_ENGINE_STATUS_IDLE           "Idle"
#define WC_ENGINE_STATUS_CONNECTING     "Connecting"
#define WC_ENGINE_STATUS_CONNECTED      "Connected"
#define WC_ENGINE_STATUS_CONNECT_FAIL   "ConnectFail"
#define WC_ENGINE_STATUS_LOGIN_SUCCESS  "LoginSuc"
#define WC_ENGINE_STATUS_LOGIN_FAIL     "LoginFail"
#define WC_ENGINE_STATUS_STOPPED        "Stopped"
#define WC_ENGINE_STATUS_STOP_FAIL      "StopFail"

//////////////////////////////////////////
/// (string) WingchunConfigKey
//////////////////////////////////////////
#define WC_CONFIG_KEY_PASSWORD          "Password"
#define WC_CONFIG_KEY_AUTH_CODE         "AuthCode"
#define WC_CONFIG_KEY_NEED_AUTH         "NeedAuth"
#define WC_CONFIG_KEY_NEED_SETTLE_CONFIRM "NeedSettleConfirm"
#define WC_CONFIG_KEY_BROKER_ID         "BrokerId"
#define WC_CONFIG_KEY_USER_ID           "UserId"
#define WC_CONFIG_KEY_EXCHANGE_ID       "ExchangeId"
#define WC_CONFIG_KEY_INVESTOR_ID       "InvestorId"
#define WC_CONFIG_KEY_FRONT_URI         "FrontUri"
#define WC_CONFIG_KEY_FRONT_URI_Q       "FrontUriQ"
#define WC_CONFIG_KEY_USER_PRODUCT_INFO "UserProductInfo"
#define WC_CONFIG_KEY_MCAST_ADDRESS     "McastAddress"
#define WC_CONFIG_KEY_NIC               "Nic"
#define WC_CONFIG_KEY_PROTOCOL_INFO     "ProtocolInfo"


WC_NAMESPACE_START

class EngineUtil
{
public:
    static inline void splitTicker(const char* ticker, char* commodityNo, char* contractNo);
    static const char* gbkErrorMsg2utf8(const char* errorMsg);
    static short getExchangeIdFromStockTicker(const char* ticker);
    static string getExchangeNameFromStockTicker(const char* ticker);
};

inline void EngineUtil::splitTicker(const char* ticker, char* commodityNo, char* contractNo)
{
    size_t i = 0;
    while (i < strlen(ticker))
    {
        if (ticker[i] <= '9' && ticker[i] >= '0')
            break;
        i++;
    }
    strncpy(commodityNo, ticker, i);
    strcpy(contractNo, &ticker[i]);
};

inline short EngineUtil::getExchangeIdFromStockTicker(const char* ticker)
{
    switch (ticker[0])
    {
        case '0': // 000 - A stock; 002 - 中小板; 080 - 配股; 031 - 权证
        case '1': // funds of sz
        case '2': // 200 - B stock
        case '3': // 300 - 创业板
            return EXCHANGE_ID_SZE;
        case '5': // fund of sh, etc
        case '6': // 600/601 - A stock
        case '7': // 730 - new stock; 700 - 配股
        case '9': // 900 - B stock
            return EXCHANGE_ID_SSE;
        default: // 4,8 新三板
            return -1;
    }
}

inline string EngineUtil::getExchangeNameFromStockTicker(const char* ticker)
{
    short exchange_id = getExchangeIdFromStockTicker(ticker);
    return getExchangeName(exchange_id);
}

WC_NAMESPACE_END

#endif //WINGCHUN_ENGINEUTIL_H
