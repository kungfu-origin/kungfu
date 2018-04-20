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
 * MDEngineXTP: XTP's market data engine adapter.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Nov, 2017
 */

#ifndef WINGCHUN_MDENGINEXTP_H
#define WINGCHUN_MDENGINEXTP_H

#include "IMDEngine.h"
#include "xtp_quote_api.h"
#include "longfist/LFConstants.h"

WC_NAMESPACE_START

class MDEngineXTP: public IMDEngine, public XTP::API::QuoteSpi
{
public:
    /** load internal information from config json */
    virtual void load(const json& j_config);
    virtual void connect(long timeout_nsec);
    virtual void login(long timeout_nsec);
    virtual void logout();
    virtual void release_api();
    virtual void subscribeMarketData(const vector<string>& instruments, const vector<string>& markets);
    virtual bool is_connected() const { return connected; };
    virtual bool is_logged_in() const { return logged_in; };
    virtual string name() const { return "MDEngineXTP"; };

public:
    MDEngineXTP();

private:
    /** xtp api */
    XTP::API::QuoteApi* api;
    /** ---- internal information --- */
    /** xtp needs user to specify unique client_id
     * for each session within same account */
    int client_id;
    string user_id;
    string password;
    string front_ip;
    int front_port;
    /// @brief udp_buffer_size > 0 then use udp, otherwise use tcp
    int udp_buffer_size;
    /** xtp's internal logger level */
    int gateway_log_level;
    // internal flags
    bool connected;
    bool logged_in;
    int  reqId;
    bool to_dump_static_info;

public:
    // SPI
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param reason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnDisconnected(int reason);

    ///订阅行情应答
    virtual void OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last);

    ///深度行情通知
    virtual void OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count);

    ///查询可交易合约的应答
    ///@param ticker_info 可交易合约信息
    ///@param error_info 查询可交易合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
    ///@param is_last 是否此次查询可交易合约的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
    virtual void OnQueryAllTickers(XTPQSI* ticker_info, XTPRI *error_info, bool is_last);

    ///错误应答
    virtual void OnError(XTPRI *error_info,bool is_last);
};

DECLARE_PTR(MDEngineXTP);

WC_NAMESPACE_END

#endif //WINGCHUN_MDENGINEXTP_H
