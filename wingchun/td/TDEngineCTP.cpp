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
// Created by cjiang on 17/4/11.
//

#include "TDEngineCTP.h"
#include "longfist/ctp.h"
#include "longfist/LFUtils.h"
#include "TypeConvert.hpp"
#include <boost/algorithm/string.hpp>

USING_WC_NAMESPACE

TDEngineCTP::TDEngineCTP(): ITDEngine(SOURCE_CTP),need_settleConfirm(true), need_authenticate(false), curAccountIdx(-1)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.CTP");
    KF_LOG_INFO(logger, "[ATTENTION] default to confirm settlement and no authentication!");
}

void TDEngineCTP::init()
{
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
}

void TDEngineCTP::pre_load(const json& j_config)
{
    front_uri = j_config[WC_CONFIG_KEY_FRONT_URI].get<string>();
    if (j_config.find(WC_CONFIG_KEY_NEED_SETTLE_CONFIRM) != j_config.end()
        && !j_config[WC_CONFIG_KEY_NEED_SETTLE_CONFIRM].get<bool>())
    {
        need_settleConfirm = false;
        KF_LOG_INFO(logger, "[ATTENTION] no need to confirm settlement!");
    }
    if (j_config.find(WC_CONFIG_KEY_NEED_AUTH) != j_config.end()
        && j_config[WC_CONFIG_KEY_NEED_AUTH].get<bool>())
    {
        need_authenticate = true;
        KF_LOG_INFO(logger, "[ATTENTION] need to authenticate code!");
    }
}

void TDEngineCTP::resize_accounts(int account_num)
{
    account_units.resize(account_num);
}

TradeAccount TDEngineCTP::load_account(int idx, const json& j_config)
{
    // internal load
    string broker_id = j_config[WC_CONFIG_KEY_BROKER_ID].get<string>();
    string user_id = j_config[WC_CONFIG_KEY_USER_ID].get<string>();
    string investor_id = j_config[WC_CONFIG_KEY_INVESTOR_ID].get<string>();
    string password = j_config[WC_CONFIG_KEY_PASSWORD].get<string>();

    AccountUnitCTP& unit = account_units[idx];
    unit.api = nullptr;
    unit.front_id = -1;
    unit.session_id = -1;
    unit.initialized = false;
    unit.connected = false;
    unit.authenticated = false;
    unit.logged_in = false;
    unit.settle_confirmed = false;
    if (need_authenticate)
        unit.auth_code = j_config[WC_CONFIG_KEY_AUTH_CODE].get<string>();

    // set up
    TradeAccount account = {};
    strncpy(account.BrokerID, broker_id.c_str(), 19);
    strncpy(account.InvestorID, investor_id.c_str(), 19);
    strncpy(account.UserID, user_id.c_str(), 16);
    strncpy(account.Password, password.c_str(), 21);
    return account;
}

void TDEngineCTP::connect(long timeout_nsec)
{
    for (size_t idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitCTP& unit = account_units[idx];
        if (unit.api == nullptr)
        {
            CThostFtdcTraderApi* api = CThostFtdcTraderApi::CreateFtdcTraderApi();
            if (!api)
            {
                throw std::runtime_error("CTP_TD failed to create api");
            }
            api->RegisterSpi(this);
            unit.api = api;
        }
        if (!unit.connected)
        {
            curAccountIdx = idx;
            unit.api->RegisterFront((char*)front_uri.c_str());
            unit.api->SubscribePublicTopic(THOST_TERT_QUICK); // need check
            unit.api->SubscribePrivateTopic(THOST_TERT_QUICK); // need check
            if (!unit.initialized)
            {
                unit.api->Init();
                unit.initialized = true;
            }
            long start_time = yijinjing::getNanoTime();
            while (!unit.connected && yijinjing::getNanoTime() - start_time < timeout_nsec)
            {}
        }
    }
}

void TDEngineCTP::login(long timeout_nsec)
{
    for (size_t idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitCTP& unit = account_units[idx];
        TradeAccount& account = accounts[idx];
        // authenticate
        if (need_authenticate && !unit.authenticated)
        {
            struct CThostFtdcReqAuthenticateField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.UserID, account.UserID);
            strcpy(req.AuthCode, unit.auth_code.c_str());
            unit.auth_rid = request_id;
            if (unit.api->ReqAuthenticate(&req, request_id++))
            {
                KF_LOG_ERROR(logger, "[request] auth failed!" << " (Bid)" << req.BrokerID
                                                              << " (Uid)" << req.UserID
                                                              << " (Auth)" << req.AuthCode);
            }
            long start_time = yijinjing::getNanoTime();
            while (!unit.authenticated && yijinjing::getNanoTime() - start_time < timeout_nsec)
            {}
        }
        // login
        if (!unit.logged_in)
        {
            struct CThostFtdcReqUserLoginField req = {};
            strcpy(req.TradingDay, "");
            strcpy(req.UserID, account.UserID);
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.Password, account.Password);
            unit.login_rid = request_id;
            if (unit.api->ReqUserLogin(&req, request_id++))
            {
                KF_LOG_ERROR(logger, "[request] login failed!" << " (Bid)" << req.BrokerID
                                                               << " (Uid)" << req.UserID);
            }
            long start_time = yijinjing::getNanoTime();
            while (!unit.logged_in && yijinjing::getNanoTime() - start_time < timeout_nsec)
            {}
        }
        // confirm settlement
        if (need_settleConfirm && !unit.settle_confirmed)
        {
            struct CThostFtdcSettlementInfoConfirmField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.InvestorID, account.InvestorID);
            unit.settle_rid = request_id;
            if (unit.api->ReqSettlementInfoConfirm(&req, request_id++))
            {
                KF_LOG_ERROR(logger, "[request] settlement info failed!" << " (Bid)" << req.BrokerID
                                                                         << " (Iid)" << req.InvestorID);
            }
            long start_time = yijinjing::getNanoTime();
            while (!unit.settle_confirmed && yijinjing::getNanoTime() - start_time < timeout_nsec)
            {}
        }
    }
}

void TDEngineCTP::logout()
{
    for (size_t idx = 0; idx < account_units.size(); idx++)
    {
        AccountUnitCTP& unit = account_units[idx];
        TradeAccount& account = accounts[idx];
        if (unit.logged_in)
        {
            CThostFtdcUserLogoutField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.UserID, account.UserID);
            unit.login_rid = request_id;
            if (unit.api->ReqUserLogout(&req, request_id++))
            {
                KF_LOG_ERROR(logger, "[request] logout failed!" << " (Bid)" << req.BrokerID
                                                                << " (Uid)" << req.UserID);
            }
        }
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
    }
}

void TDEngineCTP::release_api()
{
    for (auto& unit: account_units)
    {
        if (unit.api != nullptr)
        {
            unit.api->Release();
            unit.api = nullptr;
        }
        unit.initialized = false;
        unit.connected = false;
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
        unit.api = nullptr;
    }
}

bool TDEngineCTP::is_logged_in() const
{
    for (auto& unit: account_units)
    {
        if (!unit.logged_in || (need_settleConfirm && !unit.settle_confirmed))
            return false;
    }
    return true;
}

bool TDEngineCTP::is_connected() const
{
    for (auto& unit: account_units)
    {
        if (!unit.connected)
            return false;
    }
    return true;
}

/**
 * req functions
 */
void TDEngineCTP::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    struct CThostFtdcQryInvestorPositionField req = parseTo(*data);
    KF_LOG_DEBUG(logger, "[req_pos]" << " (Bid)" << req.BrokerID
                                     << " (Iid)" << req.InvestorID
                                     << " (Tid)" << req.InstrumentID);

    if (account_units[account_index].api->ReqQryInvestorPosition(&req, requestId))
    {
        KF_LOG_ERROR(logger, "[request] investor position failed!" << " (rid)" << requestId
                                                                   << " (idx)" << account_index);
    }
    send_writer->write_frame(&req, sizeof(CThostFtdcQryInvestorPositionField), source_id, MSG_TYPE_LF_QRY_POS_CTP, 1/*ISLAST*/, requestId);
}

void TDEngineCTP::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    struct CThostFtdcQryTradingAccountField req = parseTo(*data);
    KF_LOG_DEBUG(logger, "[req_account]" << " (Bid)" << req.BrokerID
                                         << " (Iid)" << req.InvestorID);

    if (account_units[account_index].api->ReqQryTradingAccount(&req, requestId))
    {
        KF_LOG_ERROR(logger, "[request] account info failed!" << " (rid)" << requestId
                                                              << " (idx)" << account_index);
    }
    send_writer->write_frame(&req, sizeof(CThostFtdcQryTradingAccountField), source_id, MSG_TYPE_LF_QRY_ACCOUNT_CTP, 1/*ISLAST*/, requestId);
}

void TDEngineCTP::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time)
{
    struct CThostFtdcInputOrderField req = parseTo(*data);
    req.RequestID = requestId;
    req.IsAutoSuspend = 0;
    req.UserForceClose = 0;
    KF_LOG_DEBUG(logger, "[req_order_insert]" << " (rid)" << requestId
                                              << " (Iid)" << req.InvestorID
                                              << " (Tid)" << req.InstrumentID
                                              << " (OrderRef)" << req.OrderRef);

    if (account_units[account_index].api->ReqOrderInsert(&req, requestId))
    {
        KF_LOG_ERROR(logger, "[request] order insert failed!" << " (rid)" << requestId);
    }
    send_writer->write_frame(&req, sizeof(CThostFtdcInputOrderField), source_id, MSG_TYPE_LF_ORDER_CTP, 1/*ISLAST*/, requestId);
}

void TDEngineCTP::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time)
{
    struct CThostFtdcInputOrderActionField req = parseTo(*data);
    req.OrderActionRef = local_id ++;
    auto& unit = account_units[account_index];
    req.FrontID = unit.front_id;
    req.SessionID = unit.session_id;
    KF_LOG_DEBUG(logger, "[req_order_action]" << " (rid)" << requestId
                                              << " (Iid)" << req.InvestorID
                                              << " (OrderRef)" << req.OrderRef
                                              << " (OrderActionRef)" << req.OrderActionRef);

    if (unit.api->ReqOrderAction(&req, requestId))
    {
        KF_LOG_ERROR(logger, "[request] order action failed!" << " (rid)" << requestId);
    }

    send_writer->write_frame(&req, sizeof(CThostFtdcInputOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_CTP, 1/*ISLAST*/, requestId);
}

/*
 * SPI functions
 */
void TDEngineCTP::OnFrontConnected()
{
    KF_LOG_INFO(logger, "[OnFrontConnected] (idx)" << curAccountIdx);
    account_units[curAccountIdx].connected = true;
}

void TDEngineCTP::OnFrontDisconnected(int nReason)
{
    KF_LOG_INFO(logger, "[OnFrontDisconnected] reason=" << nReason);
    for (auto& unit: account_units)
    {
        unit.connected = false;
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
    }
}

#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))

void TDEngineCTP::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        KF_LOG_ERROR(logger, "[OnRspAuthenticate]" << " (errId)" << pRspInfo->ErrorID
                                                   << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        KF_LOG_INFO(logger, "[OnRspAuthenticate]" << " (userId)" <<  pRspAuthenticateField->UserID
                                                  << " (brokerId)" << pRspAuthenticateField->BrokerID
                                                  << " (product)" << pRspAuthenticateField->UserProductInfo
                                                  << " (rid)" << nRequestID);
        for (auto& unit: account_units)
        {
            if (unit.auth_rid == nRequestID)
                unit.authenticated = true;
        }
    }
}

void TDEngineCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                 int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        KF_LOG_ERROR(logger, "[OnRspUserLogin]" << " (errId)" << pRspInfo->ErrorID
                                                << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        KF_LOG_INFO(logger, "[OnRspUserLogin]" << " (Bid)" << pRspUserLogin->BrokerID
                                               << " (Uid)" << pRspUserLogin->UserID
                                               << " (maxRef)" << pRspUserLogin->MaxOrderRef
                                               << " (Fid)" << pRspUserLogin->FrontID
                                               << " (Sid)" << pRspUserLogin->SessionID);
        for (auto& unit: account_units)
        {
            if (unit.login_rid == nRequestID)
            {
                unit.front_id = pRspUserLogin->FrontID;
                unit.session_id = pRspUserLogin->SessionID;
                unit.logged_in = true;
            }
        }
        int max_ref = atoi(pRspUserLogin->MaxOrderRef) + 1;
        local_id = (max_ref > local_id) ? max_ref: local_id;
    }
}

void TDEngineCTP::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        KF_LOG_ERROR(logger, "[OnRspSettlementInfoConfirm]" << " (errId)" << pRspInfo->ErrorID
                                                            << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        KF_LOG_INFO(logger, "[OnRspSettlementInfoConfirm]" << " (brokerID)" << pSettlementInfoConfirm->BrokerID
                                                           << " (investorID)" << pSettlementInfoConfirm->InvestorID
                                                           << " (confirmDate)" << pSettlementInfoConfirm->ConfirmDate
                                                           << " (confirmTime)" << pSettlementInfoConfirm->ConfirmTime);
        for (auto& unit: account_units)
        {
            if (unit.settle_rid == nRequestID)
            {
                unit.settle_confirmed = true;
            }
        }
    }
}

void TDEngineCTP::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                  int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID == 0)
    {
        KF_LOG_ERROR(logger, "[OnRspUserLogout]" << " (errId)" << pRspInfo->ErrorID
                                                 << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        KF_LOG_INFO(logger, "[OnRspUserLogout]" << " (brokerId)" << pUserLogout->BrokerID
                                                << " (userId)" << pUserLogout->UserID);
        for (auto& unit: account_units)
        {
            if (unit.login_rid == nRequestID)
            {
                unit.logged_in = false;
                unit.authenticated = false;
                unit.settle_confirmed = false;
            }
        }
    }
}

void TDEngineCTP::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
                                    int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    auto data = parseFrom(*pInputOrder);
    on_rsp_order_insert(&data, nRequestID, errorId, errorMsg);
    raw_writer->write_error_frame(pInputOrder, sizeof(CThostFtdcInputOrderField), source_id, MSG_TYPE_LF_ORDER_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void TDEngineCTP::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    auto data = parseFrom(*pInputOrderAction);
    on_rsp_order_action(&data, nRequestID, errorId, errorMsg);
    raw_writer->write_error_frame(pInputOrderAction, sizeof(CThostFtdcInputOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void TDEngineCTP::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo,
                                     int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    CThostFtdcInvestorPositionField emptyCtp = {};
    if (pInvestorPosition == nullptr)
        pInvestorPosition = &emptyCtp;
    auto pos = parseFrom(*pInvestorPosition);
    on_rsp_position(&pos, bIsLast, nRequestID, errorId, errorMsg);
    raw_writer->write_error_frame(pInvestorPosition, sizeof(CThostFtdcInvestorPositionField), source_id, MSG_TYPE_LF_RSP_POS_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void TDEngineCTP::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    auto rtn_order = parseFrom(*pOrder);
    on_rtn_order(&rtn_order);
    raw_writer->write_frame(pOrder, sizeof(CThostFtdcOrderField),
                            source_id, MSG_TYPE_LF_RTN_ORDER_CTP,
                            1/*islast*/, (pOrder->RequestID > 0) ? pOrder->RequestID: -1);
}

void TDEngineCTP::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    auto rtn_trade = parseFrom(*pTrade);
    on_rtn_trade(&rtn_trade);
    raw_writer->write_frame(pTrade, sizeof(CThostFtdcTradeField),
                            source_id, MSG_TYPE_LF_RTN_TRADE_CTP, 1/*islast*/, -1/*invalidRid*/);
}

void TDEngineCTP::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    CThostFtdcTradingAccountField empty = {};
    if (pTradingAccount == nullptr)
        pTradingAccount = &empty;
    auto account = parseFrom(*pTradingAccount);
    on_rsp_account(&account, bIsLast, nRequestID, errorId, errorMsg);
    raw_writer->write_error_frame(pTradingAccount, sizeof(CThostFtdcTradingAccountField), source_id, MSG_TYPE_LF_RSP_ACCOUNT_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

BOOST_PYTHON_MODULE(libctptd)
{
    using namespace boost::python;
    class_<TDEngineCTP, boost::shared_ptr<TDEngineCTP> >("Engine")
    .def(init<>())
    .def("init", &TDEngineCTP::initialize)
    .def("start", &TDEngineCTP::start)
    .def("stop", &TDEngineCTP::stop)
    .def("logout", &TDEngineCTP::logout)
    .def("wait_for_stop", &TDEngineCTP::wait_for_stop);
}