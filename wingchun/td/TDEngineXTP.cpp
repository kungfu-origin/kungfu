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
// Created by cjiang on 17/11/21.
//

#include "TDEngineXTP.h"
#include "longfist/xtp.h"
#include "longfist/LFUtils.h"
#include "TypeConvert.hpp"

USING_WC_NAMESPACE

#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))

TDEngineXTP::TDEngineXTP(): ITDEngine(SOURCE_XTP), api(nullptr), front_port(-1), client_id(-1)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.XTP");
}

void TDEngineXTP::init()
{
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
}

void TDEngineXTP::pre_load(const json& j_config)
{
    // put td level here
    front_ip = j_config["Ip"].get<string>();
    front_port = j_config["Port"].get<int>();
    account_key = j_config["Key"].get<string>();
    client_id = j_config["ClientId"].get<int>();
}

void TDEngineXTP::resize_accounts(int account_num)
{
    account_units.resize(account_num);
}

TradeAccount TDEngineXTP::load_account(int idx, const json& j_config)
{
    // internal load
    string user_id = j_config[WC_CONFIG_KEY_USER_ID].get<string>();
    string password = j_config[WC_CONFIG_KEY_PASSWORD].get<string>();

    AccountUnitXTP& unit = account_units[idx];
    unit.session_id = -1;
    unit.logged_in = false;
    // set up
    TradeAccount account = {};
    strncpy(account.InvestorID, user_id.c_str(), 16);
    strncpy(account.UserID, user_id.c_str(), 16);
    strncpy(account.Password, password.c_str(), 21);
    return account;
}

void TDEngineXTP::connect(long timeout_nsec)
{
    if (api == nullptr)
    {
        api = XTP::API::TraderApi::CreateTraderApi(client_id, KUNGFU_RUNTIME_FOLDER);
        if (!api)
        {
            throw std::runtime_error("XTP_MD failed to create api");
        }
        api->SubscribePublicTopic(XTP_TERT_QUICK);//只传送登录后公有流（订单响应、成交回报）的内容
        api->SetSoftwareVersion("1.1.0");
        api->SetSoftwareKey(account_key.c_str());
        api->RegisterSpi(this);
    }
    for (size_t idx = 0; idx < account_units.size(); idx ++)
    {
        if (!account_units[idx].logged_in)
        {
            long session_id = api->Login(front_ip.c_str(), front_port, accounts[idx].UserID, accounts[idx].Password, XTP_PROTOCOL_TCP);
            if (session_id > 0)
            {
                account_units[idx].session_id = session_id;
                account_units[idx].logged_in = true;
                KF_LOG_INFO(logger, "[Login] login succeed!" << " (user_id)" << accounts[idx].UserID
                                                             << " (session)" << session_id
                                                             << " (idx)" << idx);
            }
            else
            {
                XTPRI* error_info = api->GetApiLastError();
                int err_id = error_info->error_id;
                KF_LOG_ERROR(logger, "[request] login failed! (errId)" << err_id << " (errMsg)" << GBK2UTF8(error_info->error_msg));
                if (err_id == 10210101)
                {
                    client_id += 1;
                    KF_LOG_INFO(logger, "due to client_id, will auto-retry with new client_id:" << client_id);
                    logout();
                    release_api();
                    connect(timeout_nsec);
                }
            }
        }
    }
}

void TDEngineXTP::login(long timeout_nsec)
{
    connect(timeout_nsec);
}

void TDEngineXTP::logout()
{
    if (api == nullptr)
        return;

    for (size_t idx = 0; idx < account_units.size(); idx++)
    {
        if (account_units[idx].session_id > 0)
        {
            long session_id = account_units[idx].session_id;
            if (api->Logout(session_id) == 0)
            {
                account_units[idx].session_id = -1;
                KF_LOG_INFO(logger, "[Logout] logout succeed!" << " (user_id)" << accounts[idx].UserID
                                                               << " (session)" << session_id
                                                               << " (idx)" << idx);
            }
            else
            {
                XTPRI* error_info = api->GetApiLastError();
                int err_id = error_info->error_id;
                KF_LOG_ERROR(logger, "[request] logout failed! (errId)" << err_id << " (errMsg)" << GBK2UTF8(error_info->error_msg));
            }
            account_units[idx].logged_in = false;
        }
    }
}

void TDEngineXTP::release_api()
{
    logout();
    if (api != nullptr)
    {
        api->Release();
        api = nullptr;
    }
}

bool TDEngineXTP::is_logged_in() const
{
    for (auto& unit: account_units)
    {
        if (!unit.logged_in)
            return false;
    }
    return true;
}

bool TDEngineXTP::is_connected() const
{
    return is_logged_in();
}

/**
 * req functions
 */
void TDEngineXTP::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    KF_LOG_DEBUG(logger, "[req_pos]" << " (Iid)" << data->InvestorID);

    if (!account_units[account_index].logged_in || api->QueryPosition(nullptr, account_units[account_index].session_id, requestId) != 0)
    {
        KF_LOG_ERROR(logger, "[request] investor position failed!" << " (rid)" << requestId
                                                                   << " (idx)" << account_index);
    }
}

void TDEngineXTP::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    KF_LOG_DEBUG(logger, "[req_pos]" << " (Iid)" << data->InvestorID);

    if (!account_units[account_index].logged_in || api->QueryAsset(account_units[account_index].session_id, requestId) != 0)
    {
        KF_LOG_ERROR(logger, "[request] account info failed!" << " (rid)" << requestId
                                                              << " (idx)" << account_index);
    }
}

void TDEngineXTP::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time)
{
    struct XTPOrderInsertInfo req = parseTo(*data);
    KF_LOG_DEBUG(logger, "[req_order_insert]" << " (rid)" << requestId
                                              << " (Tid)" << req.ticker
                                              << " (OrderRef)" << req.order_client_id);
    if (account_units[account_index].logged_in)
    {
        long xtp_id = api->InsertOrder(&req, account_units[account_index].session_id);
        if (xtp_id == 0)
        {
            XTPRI* error_info = api->GetApiLastError();
            int err_id = error_info->error_id;
            char* err_msg = (char*)GBK2UTF8(error_info->error_msg).c_str();
            send_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER, 1, requestId, err_id, err_msg);
            KF_LOG_ERROR(logger, "[request] order insert failed!" << " (rid)" << requestId << " (errId)" << err_id << " (errMsg)" << err_msg);
        }
        else
        {
            xtp_ids[requestId] = xtp_id;
            send_writer->write_frame(&req, sizeof(XTPOrderInsertInfo), source_id, MSG_TYPE_LF_ORDER_XTP, 1/*ISLAST*/, requestId);
        }
    }
}

void TDEngineXTP::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time)
{
    int order_id = data->KfOrderID;
    if (account_units[account_index].logged_in && xtp_ids.find(order_id) != xtp_ids.end())
    {
        long xtp_order_id = xtp_ids[order_id];
        KF_LOG_DEBUG(logger, "[req_order_action]" << " (rid)" << requestId
                                                  << " (Iid)" << order_id
                                                  << " (xtp_id)" << xtp_order_id);
        if (api->CancelOrder(xtp_order_id, account_units[account_index].session_id) == 0)
        {
            XTPRI* error_info = api->GetApiLastError();
            int err_id = error_info->error_id;
            char* err_msg = (char*)GBK2UTF8(error_info->error_msg).c_str();
            send_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION, 1, requestId, err_id, err_msg);
            KF_LOG_ERROR(logger, "[request] order cancel failed!" << " (rid)" << requestId << " (errId)" << err_id << " (errMsg)" << err_msg);
        }
    }
}

/*
 * SPI functions
 */
void TDEngineXTP::OnDisconnected(uint64_t session_id, int reason)
{
    KF_LOG_INFO(logger, "[OnDisconnected] (session_id)" << session_id << " (reason)" << reason);
    for (auto& unit: account_units)
    {
        if (unit.session_id == session_id)
            unit.logged_in = false;
    }
}

void TDEngineXTP::OnError(XTPRI *error_info)
{
    int err_id = error_info->error_id;
    char* err_msg = (char*)GBK2UTF8(error_info->error_msg).c_str();
    KF_LOG_ERROR(logger, "[OnError] " << " (errId)" << err_id << " (errMsg)" << err_msg);
}

void TDEngineXTP::OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id)
{
    auto rtn_order = parseFrom(*order_info);
    if (error_info == nullptr || error_info->error_id == 0)
    {
        on_rtn_order(&rtn_order);
        raw_writer->write_frame(order_info, sizeof(XTPOrderInfo),
                                source_id, MSG_TYPE_LF_RTN_ORDER_XTP,
                                1/*islast*/, -1);
    }
    else
    {
        int local_id = std::stoi(string(rtn_order.OrderRef));
        int rid = td_helper->get_order_id(local_id);
        LFInputOrderField data = {};
        memcpy(data.InstrumentID, order_info->ticker, 16);
        on_rsp_order_insert(&data, rid, error_info->error_id, error_info->error_msg);
    }
}

void TDEngineXTP::OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id)
{
    auto rtn_trade = parseFrom(*trade_info);
    on_rtn_trade(&rtn_trade);
    raw_writer->write_frame(trade_info, sizeof(XTPTradeReport),
                            source_id, MSG_TYPE_LF_RTN_TRADE_XTP, 1/*islast*/, -1/*invalidRid*/);
}

void TDEngineXTP::OnCancelOrderError(XTPOrderCancelInfo *cancel_info, XTPRI *error_info, uint64_t session_id)
{
    OnError(error_info);
}

void TDEngineXTP::OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
{
    if (error_info == nullptr || error_info->error_id == 0)
    {
        auto pos = parseFrom(*position);
        on_rsp_position(&pos, is_last, request_id);
    }
    else
    {
        LFRspPositionField pos = {};
        on_rsp_position(&pos, is_last, request_id, error_info->error_id, error_info->error_msg);
    }
}

BOOST_PYTHON_MODULE(libxtptd)
{
    using namespace boost::python;
    class_<TDEngineXTP, boost::shared_ptr<TDEngineXTP> >("Engine")
    .def(init<>())
    .def("init", &TDEngineXTP::initialize)
    .def("start", &TDEngineXTP::start)
    .def("stop", &TDEngineXTP::stop)
    .def("logout", &TDEngineXTP::logout)
    .def("wait_for_stop", &TDEngineXTP::wait_for_stop);
}