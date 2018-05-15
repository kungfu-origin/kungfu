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

#include "MDEngineXTP.h"
#include "TypeConvert.hpp"
#include "Timer.h"
#include "EngineUtil.hpp"
#include "longfist/xtp.h"
#include "longfist/LFUtils.h"

USING_WC_NAMESPACE

#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))

MDEngineXTP::MDEngineXTP(): IMDEngine(SOURCE_XTP),
    api(nullptr), client_id(1), front_port(-1), udp_buffer_size(0), gateway_log_level(0),
    connected(false), logged_in(false), reqId(0), to_dump_static_info(false)
{
    logger = yijinjing::KfLog::getLogger("MdEngine.XTP");
}

void MDEngineXTP::load(const json& j_config)
{
    client_id = j_config["ClientId"].get<int>();
    user_id = j_config[WC_CONFIG_KEY_USER_ID].get<string>();
    password = j_config[WC_CONFIG_KEY_PASSWORD].get<string>();
    front_ip = j_config["Ip"].get<string>();
    front_port = j_config["Port"].get<int>();
    udp_buffer_size = j_config.value("UdpBufferSize", 0);
    if(udp_buffer_size > 0)
    {
        KF_LOG_INFO(logger, "[Protocol] UDP (buffer_size)" << udp_buffer_size);
    }
    else
    {
        KF_LOG_INFO(logger, "[Protocol] TCP");
    }
    to_dump_static_info = j_config.value("DumpStatic", false);

    string log_level_string = j_config.value("GatewayLogLevel", "DEBUG");
    if (log_level_string.compare("FATAL") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_FATAL;
        KF_LOG_INFO(logger, "[GatewayLogLevel] FATAL");
    }
    else if (log_level_string.compare("ERROR") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_ERROR;
        KF_LOG_INFO(logger, "[GatewayLogLevel] ERROR");
    }
    else if (log_level_string.compare("WARNING") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_WARNING;
        KF_LOG_INFO(logger, "[GatewayLogLevel] WARNING");
    }
    else if (log_level_string.compare("INFO") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_INFO;
        KF_LOG_INFO(logger, "[GatewayLogLevel] INFO");
    }
    else if (log_level_string.compare("DEBUG") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_DEBUG;
        KF_LOG_INFO(logger, "[GatewayLogLevel] DEBUG");
    }
    else if (log_level_string.compare("TRACE") == 0)
    {
        gateway_log_level = XTP_LOG_LEVEL_TRACE;
        KF_LOG_INFO(logger, "[GatewayLogLevel] TRACE");
    }
    else
    {
        gateway_log_level = XTP_LOG_LEVEL_DEBUG;
        KF_LOG_INFO(logger, "[GatewayLogLevel] DEBUG (default)");
    }
}

void MDEngineXTP::connect(long timeout_nsec)
{
    // xtp is using sync api, no need to set timeout limit.
    if (api == nullptr)
    {
        api = XTP::API::QuoteApi::CreateQuoteApi(client_id, KUNGFU_RUNTIME_FOLDER, (XTP_LOG_LEVEL)gateway_log_level);
        if (!api)
        {
            throw std::runtime_error("XTP_MD failed to create api");
        }
        api->RegisterSpi(this);
    }
    if (!connected)
    {
        XTP_PROTOCOL_TYPE protocol_type = udp_buffer_size > 0 ? XTP_PROTOCOL_UDP : XTP_PROTOCOL_TCP;
        if(XTP_PROTOCOL_UDP == protocol_type)
        {
            api->SetUDPBufferSize(udp_buffer_size);//设置UDP接收缓冲区大小，单位为MB
        }
        int res = api->Login(front_ip.c_str(), front_port, user_id.c_str(), password.c_str(), protocol_type);
        if (res != 0)
        {
            XTPRI* error_info = api->GetApiLastError();
            int err_id = error_info->error_id;
            KF_LOG_ERROR(logger, "[request] login failed! (errId)" << err_id << " (errMsg)" << GBK2UTF8(error_info->error_msg));
            if (err_id == 10210101)
            {
                client_id += 1;
                KF_LOG_INFO(logger, "due to client_id, will auto-retry with new client_id:" << client_id);
                release_api();
                connect(timeout_nsec);
            }
        }
        else
        {
            connected = true;
            logged_in = true;
            KF_LOG_INFO(logger, "[Login] login succeed! (user_id)" << user_id << " (client_id)" << client_id);

            if (to_dump_static_info)
            {
                int res1 = api->QueryAllTickers(XTP_EXCHANGE_SH);
                int res2 = api->QueryAllTickers(XTP_EXCHANGE_SZ);
                if (res1 == 0 && res2 == 0)
                {
                    KF_LOG_INFO(logger, "[QueryAllTickers] success");
                }
                else
                {
                    KF_LOG_ERROR(logger, "[QueryAllTickers] failed (SH)" << res1 << " (SZ)" << res2);
                }
            }
        }
    }
}

void MDEngineXTP::login(long timeout_nsec)
{
    if (!logged_in)
    {
        connect(timeout_nsec);
    }
}

void MDEngineXTP::logout()
{
    if (logged_in)
    {
        int res = api->Logout();
        if (res != 0)
        {
            XTPRI* error_info = api->GetApiLastError();
            KF_LOG_ERROR(logger, "[request] logout failed!" << " (errId)" << error_info->error_id
                                                            << " (errMsg)" << GBK2UTF8(error_info->error_msg));
        }
        else
        {
            KF_LOG_INFO(logger, "[Logout] logout succeed! (user_id)" << user_id << " (client_id)" << client_id);
        }
    }
    connected = false;
    logged_in = false;
}

void MDEngineXTP::release_api()
{
    if (api != nullptr)
    {
        api->Release();
        api = nullptr;
    }
}

void MDEngineXTP::subscribeMarketData(const vector<string>& instruments, const vector<string>& markets)
{
    map<byte, vector<string> > ticker_map;
    for (size_t i = 0; i < markets.size(); i++)
    {
        const string& market = markets[i];
        XTP_EXCHANGE_TYPE exId = XTP_EXCHANGE_UNKNOWN;
        if (market.size() > 0)
        {
            ToXtpExchangeId(exId, market.c_str());
        }
        else
        {
            short lf_exchange_id = EngineUtil::getExchangeIdFromStockTicker(instruments[i].c_str());
            if (lf_exchange_id == EXCHANGE_ID_SSE)
                exId = XTP_EXCHANGE_SH;
            else if (lf_exchange_id == EXCHANGE_ID_SZE)
                exId = XTP_EXCHANGE_SZ;
        }
        if (exId != XTP_EXCHANGE_UNKNOWN)
        {
            ticker_map[exId].push_back(instruments[i]);
        }
    }
    for (auto iter: ticker_map)
    {
        vector<string>& tickers = iter.second;
        int nCount = tickers.size();
        char* *insts = new char*[nCount];
        for (size_t i = 0; i < tickers.size(); i++)
        {
            insts[i] = (char*)tickers[i].c_str();
        }
        api->SubscribeMarketData(insts, nCount, (XTP_EXCHANGE_TYPE)iter.first);
        delete[] insts;
    }
}

/*
 * SPI functions
 */
void MDEngineXTP::OnDisconnected(int reason)
{
    KF_LOG_INFO(logger, "[OnDisconnected] reason=" << reason);
    connected = false;
    logged_in = false;
}

void MDEngineXTP::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    if (error_info != nullptr && error_info->error_id != 0)
    {
        KF_LOG_ERROR(logger, "[OnSubMarketData]" << " (errID)" << error_info->error_id
                                                 << " (errMsg)" << GBK2UTF8(error_info->error_msg)
                                                 << " (Tid)" << ((ticker != nullptr) ?
                                                                 ticker->ticker : "null"));
    }
}

void MDEngineXTP::OnError(XTPRI *error_info,bool is_last)
{
    if (error_info != nullptr && error_info->error_id != 0)
    {
        KF_LOG_ERROR(logger, "[OnError]" << " (errID)" << error_info->error_id
                                         << " (errMsg)" << GBK2UTF8(error_info->error_msg)
                                         << " (isLast)" << is_last);
    }
}

void MDEngineXTP::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
    auto data = parseFrom(*market_data);
    on_market_data(&data);
    // if need to write raw data...
    // raw_writer->write_frame(pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField),
    //                         source_id, MSG_TYPE_LF_MD_CTP, 1/*islast*/, -1/*invalidRid*/);
}

void MDEngineXTP::OnQueryAllTickers(XTPQSI* ticker_info, XTPRI *error_info, bool is_last)
{
    if (error_info != nullptr && error_info->error_id != 0)
    {
        KF_LOG_ERROR(logger, "[OnQueryAllTickers] FAILED" << " (errID)" << error_info->error_id
                                                          << " (errMsg)" << GBK2UTF8(error_info->error_msg)
                                                          << " (isLast)" << is_last);
    }
    else
    {
        KF_LOG_INFO(logger, "[OnQueryAllTickers] (exchange_id)"<<ticker_info->exchange_id
                                                               << " (ticker)" <<ticker_info->ticker
                                                               << " (ticker_name)" <<ticker_info->ticker_name
                                                               << " (ticker_type)" <<ticker_info->ticker_type
                                                               << " (pre_close_price)" <<ticker_info->pre_close_price
                                                               << " (upper_limit_price)" <<ticker_info->upper_limit_price
                                                               << " (lower_limit_price)" <<ticker_info->lower_limit_price
                                                               << " (buy_qty_unit)" <<ticker_info->buy_qty_unit
                                                               << " (sell_qty_unit)" <<ticker_info->sell_qty_unit);
    }
}

BOOST_PYTHON_MODULE(libxtpmd)
{
    using namespace boost::python;
    class_<MDEngineXTP, boost::shared_ptr<MDEngineXTP> >("Engine")
    .def(init<>())
    .def("init", &MDEngineXTP::initialize)
    .def("start", &MDEngineXTP::start)
    .def("stop", &MDEngineXTP::stop)
    .def("logout", &MDEngineXTP::logout)
    .def("wait_for_stop", &MDEngineXTP::wait_for_stop);
}