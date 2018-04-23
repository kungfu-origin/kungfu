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
 * ITDEngine: base class of all trade engine.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   April, 2017
 */

#include "ITDEngine.h"
#include "PageCommStruct.h" /**< REQUEST_ID_RANGE */
#include "Timer.h"
#include "longfist/LFConstants.h"
#include "longfist/LFUtils.h"
#include "longfist/sys_messages.h"
#include <algorithm>

USING_WC_NAMESPACE

class RidClientManager
{
private:
    /** rid, strategy_name map */
    map<int, string> rid2client;
public:
    inline void set(int rid_start, string name)
    {
        rid2client[rid_start / REQUEST_ID_RANGE] = name;
    }
    inline string get(int rid)
    {
        auto iter = rid2client.find(rid / REQUEST_ID_RANGE);
        if (iter == rid2client.end())
            return "";
        else
            return iter->second;
    }
};

/** manage rid and client_name matching */
RidClientManager rid_manager;

ITDEngine::ITDEngine(short source): IEngine(source), default_account_index(-1), request_id(1), local_id(1)
{}

void ITDEngine::set_reader_thread()
{
    reader_thread = ThreadPtr(new std::thread(boost::bind(&ITDEngine::listening, this)));
}

void ITDEngine::init()
{
    reader = yijinjing::JournalReader::createRevisableReader(name());
    JournalPair tdPair = getTdJournalPair(source_id);
    writer = yijinjing::JournalSafeWriter::create(tdPair.first, tdPair.second, name());
    JournalPair tdSendPair = getTdSendJournalPair(source_id);
    send_writer = yijinjing::JournalWriter::create(tdSendPair.first, tdSendPair.second, "SEND_" + name());
    user_helper = TDUserInfoHelperPtr(new TDUserInfoHelper(source_id));
    td_helper = TDEngineInfoHelperPtr(new TDEngineInfoHelper(source_id, name()));
}

void ITDEngine::listening()
{
    yijinjing::FramePtr frame;
    while (isRunning && signal_received < 0)
    {
        frame = reader->getNextFrame();
        if (frame.get() != nullptr)
        {
            short msg_type = frame->getMsgType();
            short msg_source = frame->getSource();
            cur_time = frame->getNano();
            if (msg_type == MSG_TYPE_LF_MD)
            {
                void* fdata = frame->getData();
                LFMarketDataField* md = (LFMarketDataField*)fdata;
                on_market_data(md, cur_time);
                continue;
            }
            else if (msg_type < 200)
            {
                // system related...
                if (msg_type == MSG_TYPE_TRADE_ENGINE_LOGIN && msg_source == source_id)
                {
                    try
                    {
                        string content((char*)frame->getData());
                        json j_request = json::parse(content);
                        string client_name = j_request["name"].get<string>();
                        if (add_client(client_name, j_request))
                            KF_LOG_INFO(logger, "[user] Accepted: " << client_name);
                        else
                            KF_LOG_INFO(logger, "[user] Rejected: " << client_name);
                    }
                    catch (...)
                    {
                        KF_LOG_ERROR(logger, "error in parsing TRADE_ENGINE_LOGIN: " << (char*)frame->getData());
                    }
                }
                else if (msg_type == MSG_TYPE_STRATEGY_END)
                {
                    try
                    {
                        string content((char*)frame->getData());
                        json j_request = json::parse(content);
                        string client_name = j_request["name"].get<string>();
                        if (remove_client(client_name, j_request))
                            KF_LOG_INFO(logger, "[user] Removed: " << client_name);
                    }
                    catch (...)
                    {
                        KF_LOG_ERROR(logger, "error in parsing STRATEGY_END: " << (char*)frame->getData());
                    }
                }
                else if(msg_type == MSG_TYPE_STRATEGY_POS_SET && msg_source == source_id)
                {
                    try
                    {
                        string content((char*)frame->getData());
                        json j_request = json::parse(content);
                        string client_name = j_request["name"].get<string>();
                        user_helper->set_pos(client_name, j_request);
                        clients[client_name].pos_handler = PosHandler::create(source_id, content);
                        clients[client_name].pos_handler->set_fee(accounts[clients[client_name].account_index].fee_handler);
                        KF_LOG_INFO(logger, "[user] set pos: (client)" << client_name
                                                                       << " (pos)" << clients[client_name].pos_handler->to_string());
                    }
                    catch (...)
                    {
                        KF_LOG_ERROR(logger, "error in parsing STRATEGY_POS_SET: " << (char*)frame->getData());
                    }
                }
                else if (msg_type == MSG_TYPE_TRADE_ENGINE_OPEN && (msg_source <= 0 || msg_source == source_id))
                {
                    on_engine_open();
                }
                else if (msg_type == MSG_TYPE_TRADE_ENGINE_CLOSE && (msg_source <= 0 || msg_source == source_id))
                {
                    on_engine_close();
                }
                else if (msg_type == MSG_TYPE_STRING_COMMAND)
                {
                    string cmd((char*)frame->getData());
                    on_command(cmd);
                }
                else if (msg_type == MSG_TYPE_SWITCH_TRADING_DAY)
                {
                    user_helper->switch_day();
                    for (auto iter: clients)
                    {
                        if (iter.second.pos_handler.get() != nullptr)
                            iter.second.pos_handler->switch_day();
                    }
                    local_id = 1;
                    on_switch_day();
                }
            }
            else if (msg_source == source_id && is_logged_in())
            {
                // from client
                string name = reader->getFrameName();
                auto iter = clients.find(name);
                if (iter == clients.end())
                    continue;

                void* fdata = frame->getData();
                int requestId = frame->getRequestId();
                int idx = iter->second.account_index;
                switch (msg_type)
                {
                    case MSG_TYPE_LF_QRY_POS:
                    {
                        LFQryPositionField* pos = (LFQryPositionField*)fdata;
                        strcpy(pos->BrokerID, accounts[idx].BrokerID);
                        strcpy(pos->InvestorID, accounts[idx].InvestorID);
                        req_investor_position(pos, idx, requestId);
                        break;
                    }
                    case MSG_TYPE_LF_ORDER:
                    {
                        LFInputOrderField* order = (LFInputOrderField*)fdata;
                        strcpy(order->BrokerID, accounts[idx].BrokerID);
                        strcpy(order->InvestorID, accounts[idx].InvestorID);
                        strcpy(order->UserID, accounts[idx].UserID);
                        strcpy(order->BusinessUnit, accounts[idx].BusinessUnit);
                        string order_ref = std::to_string(local_id);
                        td_helper->record_order(local_id, requestId);
                        user_helper->record_order(name, local_id, requestId, order->InstrumentID);
                        local_id ++;
                        strcpy(order->OrderRef, order_ref.c_str());
                        long before_nano = kungfu::yijinjing::getNanoTime();
                        req_order_insert(order, idx, requestId, cur_time);
                        // insert order, we need to track in send
                        send_writer->write_frame_extra(order, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER, 1/*ISLAST*/, requestId, before_nano);
                        KF_LOG_DEBUG(logger, "[insert_order] (rid)" << requestId << " (ticker)" << order->InstrumentID << " (ref)" << order_ref);
                        break;
                    }
                    case MSG_TYPE_LF_ORDER_ACTION:
                    {
                        LFOrderActionField* order = (LFOrderActionField*)fdata;
                        strcpy(order->BrokerID, accounts[idx].BrokerID);
                        strcpy(order->InvestorID, accounts[idx].InvestorID);
                        int order_id = order->KfOrderID;
                        int local_id;
                        if (user_helper->get_order(name, order_id, local_id, order->InstrumentID))
                        {
                            string order_ref = std::to_string(local_id);
                            strcpy(order->OrderRef, order_ref.c_str());
                            KF_LOG_DEBUG(logger, "[cancel_order] (rid)" << order_id << " (ticker)" << order->InstrumentID << " (ref)" << order_ref);
                            req_order_action(order, idx, requestId, cur_time);
                        }
                        break;
                    }
                    case MSG_TYPE_LF_QRY_ACCOUNT:
                    {
                        LFQryAccountField* acc = (LFQryAccountField*)fdata;
                        strcpy(acc->BrokerID, accounts[idx].BrokerID);
                        strcpy(acc->InvestorID, accounts[idx].InvestorID);
                        req_qry_account(acc, idx, requestId);
                        break;
                    }
                    default:
                        KF_LOG_DEBUG(logger, "[Unexpected] frame found: (msg_type)" << msg_type << ", (name)" << name);
                }
            }
        }
    }

    if (IEngine::signal_received >= 0)
    {
        KF_LOG_INFO(logger, "[IEngine] signal received: " << IEngine::signal_received);
    }

    if (!isRunning)
    {
        KF_LOG_INFO(logger, "[IEngine] forced to stop.");
    }
}

static const LFRspPositionField empty_pos = {};

/** on investor position, engine (on_data) */
void ITDEngine::on_rsp_position(const LFRspPositionField* pos, bool isLast, int requestId, int errorId, const char* errorMsg)
{
    if (errorId == 0)
    {
        writer->write_frame(pos, sizeof(LFRspPositionField), source_id, MSG_TYPE_LF_RSP_POS, isLast, requestId);
        KF_LOG_DEBUG(logger, "[RspPosition]" << " (rid)" << requestId
                                             << " (ticker)" << pos->InstrumentID
                                             << " (dir)" << pos->PosiDirection
                                             << " (cost)" << pos->PositionCost
                                             << " (pos)" << pos->Position
                                             << " (yd)" << pos->YdPosition);
    }
    else
    {
        writer->write_error_frame(pos, sizeof(LFRspPositionField), source_id, MSG_TYPE_LF_RSP_POS, isLast, requestId, errorId, errorMsg);
        KF_LOG_ERROR(logger, "[RspPosition] fail! " << " (rid)" << requestId << " (errorId)" << errorId << " (errorMsg)" << errorMsg);
    }
}
/** on rsp order insert, engine (on_data) */
void ITDEngine::on_rsp_order_insert(const LFInputOrderField* order, int requestId, int errorId, const char* errorMsg)
{
    if (errorId == 0)
    {
        writer->write_frame(order, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER, true, requestId);
        KF_LOG_DEBUG(logger, "[RspOrder]" << " (rid)" << requestId
                                          << " (ticker)" << order->InstrumentID);
    }
    else
    {
        string name = rid_manager.get(requestId);
        user_helper->set_order_status(name, requestId, LF_CHAR_Error);
        writer->write_error_frame(order, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER, true, requestId, errorId, errorMsg);
        KF_LOG_ERROR(logger, "[RspOrder] fail!" << " (rid)" << requestId << " (errorId)" << errorId << " (errorMsg)" << errorMsg);
    }
}
/** on rsp order action, engine (on_data) */
void ITDEngine::on_rsp_order_action(const LFOrderActionField *action, int requestId, int errorId, const char *errorMsg)
{
    if (errorId == 0)
    {
        writer->write_frame(action, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION, true, requestId);
        KF_LOG_DEBUG(logger, "[RspAction]" << " (rid)" << requestId
                                           << " (ticker)" << action->InstrumentID);
    }
    else
    {
        writer->write_error_frame(action, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION, true, requestId, errorId, errorMsg);
        KF_LOG_ERROR(logger, "[RspAction] fail!" << " (rid)" << requestId << " (errorId)" << errorId << " (errorMsg)" << errorMsg);
    }
}
/** on rsp account info, engine (on_data) */
void ITDEngine::on_rsp_account(const LFRspAccountField* account, bool isLast, int requestId, int errorId, const char* errorMsg)
{
    if (errorId == 0)
    {
        writer->write_frame(account, sizeof(LFRspAccountField), source_id, MSG_TYPE_LF_RSP_ACCOUNT, isLast, requestId);
        KF_LOG_DEBUG(logger, "[RspAccount]" << " (rid)" << requestId
                                            << " (investor)" << account->InvestorID
                                            << " (balance)" << account->Balance
                                            << " (value)" << account->MarketValue);
    }
    else
    {
        writer->write_error_frame(account, sizeof(LFRspAccountField), source_id, MSG_TYPE_LF_RSP_ACCOUNT, isLast, requestId, errorId, errorMsg);
        KF_LOG_ERROR(logger, "[RspAccount] fail!" << " (rid)" << requestId << " (errorId)" << errorId << " (errorMsg)" << errorMsg);
    }
}
/** on rtn order, engine (on_data) */
void ITDEngine::on_rtn_order(const LFRtnOrderField* rtn_order)
{
    int local_id = std::stoi(string(rtn_order->OrderRef));
    int rid = td_helper->get_order_id(local_id);
    writer->write_frame(rtn_order, sizeof(LFRtnOrderField), source_id, MSG_TYPE_LF_RTN_ORDER, 1/*islast*/, rid);
    KF_LOG_DEBUG_FMT(logger, "[o] (id)%d (ref)%s (ticker)%s (Vsum)%d (Vtrd)%d (Vrmn)%d (St)%c",
                     rid,
                     rtn_order->OrderRef,
                     rtn_order->InstrumentID,
                     rtn_order->VolumeTotalOriginal,
                     rtn_order->VolumeTraded,
                     rtn_order->VolumeTotal,
                     rtn_order->OrderStatus);
    string name = rid_manager.get(rid);
    user_helper->set_order_status(name, rid, rtn_order->OrderStatus);
    td_helper->set_order_status(local_id, rtn_order->OrderStatus);
}
/** on rtn trade, engine (on_data) */
void ITDEngine::on_rtn_trade(const LFRtnTradeField* rtn_trade)
{
    int local_id = std::stoi(string(rtn_trade->OrderRef));
    int rid = td_helper->get_order_id(local_id);
    writer->write_frame(rtn_trade, sizeof(LFRtnTradeField), source_id, MSG_TYPE_LF_RTN_TRADE, 1/*islast*/, rid);
    KF_LOG_DEBUG_FMT(logger, "[t] (id)%d (ref)%s (ticker)%s (V)%d (P)%f (D)%c",
                     rid,
                     rtn_trade->OrderRef,
                     rtn_trade->InstrumentID,
                     rtn_trade->Volume,
                     rtn_trade->Price,
                     rtn_trade->Direction);
    string name = rid_manager.get(rid);
    auto iter = clients.find(name);
    if (iter != clients.end() && iter->second.pos_handler.get() != nullptr)
    {
        iter->second.pos_handler->update(rtn_trade);
        KF_LOG_DEBUG(logger, "[cost]"
                << " (long_amt)" << iter->second.pos_handler->get_long_balance(rtn_trade->InstrumentID)
                << " (long_fee)" << iter->second.pos_handler->get_long_fee(rtn_trade->InstrumentID)
                << " (short_amt)" << iter->second.pos_handler->get_short_balance(rtn_trade->InstrumentID)
                << " (short_fee)" << iter->second.pos_handler->get_short_fee(rtn_trade->InstrumentID)
        );
    }
}

bool ITDEngine::add_client(const string& client_name, const json& j_request)
{
    string folder = j_request["folder"].get<string>();
    int rid_s = j_request["rid_s"].get<int>();
    int rid_e = j_request["rid_e"].get<int>();
    long last_switch_day = j_request["last_switch_nano"].get<long>();
    rid_manager.set(rid_s, client_name);
    user_helper->load(client_name);
    auto iter = clients.find(client_name);
    if (iter == clients.end())
    {
        if (default_account_index < 0)
        {
            return false;
        }
        else
        {
            int idx = reader->addJournal(folder, client_name);
            reader->seekTimeJournal(idx, cur_time);
            ClientInfoUnit& status = clients[client_name];
            status.is_alive = true;
            status.journal_index = idx;
            status.account_index = default_account_index;
            status.rid_start = rid_s;
            status.rid_end = rid_e;
        }
    }
    else if (iter->second.is_alive)
    {
        KF_LOG_ERROR(logger, "login already exists... (info)" << client_name);
    }
    else
    {
        if (iter->second.journal_index < 0)
            iter->second.journal_index = reader->addJournal(folder, client_name);

        reader->seekTimeJournal(iter->second.journal_index, cur_time);
        iter->second.rid_start = rid_s;
        iter->second.rid_end = rid_e;
        iter->second.is_alive = true;
    }
    /**
     * json_ack: {'name': 'bl_test', 'Pos':{'ic1701':[100,50,0,0,0,0]}, 'Source': 1, 'ok': true}
     * failed: {'name': 'bl_test', 'ok': false}
     */
    json json_ack = user_helper->get_pos(client_name);
    json_ack[PH_FEE_SETUP_KEY] = accounts[clients[client_name].account_index].fee_handler->to_json();
    if (json_ack["ok"].get<bool>())
    {
        PosHandlerPtr pos_handler = PosHandler::create(source_id, json_ack);
        clients[client_name].pos_handler = pos_handler;
        if (json_ack.find("nano") != json_ack.end() && json_ack["nano"].get<long>() < last_switch_day)
        {
            pos_handler->switch_day();
            json_ack["Pos"] = pos_handler->to_json()["Pos"];
        }
    }
    string json_content = json_ack.dump();
    writer->write_frame(json_content.c_str(), json_content.length() + 1, source_id, MSG_TYPE_TRADE_ENGINE_ACK, 1, -1);
    return true;
}

bool ITDEngine::remove_client(const string &client_name, const json &j_request)
{
    auto iter = clients.find(client_name);
    if (iter == clients.end())
        return false;
    reader->expireJournal(iter->second.journal_index);
    iter->second.is_alive = false;
    if (iter->second.pos_handler.get() != nullptr)
    {
        json j_pos = iter->second.pos_handler->to_json();
        user_helper->set_pos(client_name, j_pos);
        iter->second.pos_handler.reset();
    }
    if (is_logged_in())
    {
        // cancel all pending orders, and clear the memory
        auto orders = user_helper->get_existing_orders(client_name);
        int idx = iter->second.account_index;
        for (int order_id: orders)
        {
            LFOrderActionField action = {};
            action.ActionFlag = LF_CHAR_Delete;
            action.KfOrderID = order_id;
            action.LimitPrice = 0;
            action.VolumeChange = 0;
            strcpy(action.BrokerID, accounts[idx].BrokerID);
            strcpy(action.InvestorID, accounts[idx].InvestorID);
            int local_id;
            if (user_helper->get_order(client_name, order_id, local_id, action.InstrumentID))
            {
                string order_ref = std::to_string(local_id);
                strcpy(action.OrderRef, order_ref.c_str());
                KF_LOG_DEBUG(logger, "[cancel_remain_order] (rid)" << order_id << " (ticker)" << action.InstrumentID << " (ref)" << order_ref);
                req_order_action(&action, iter->second.account_index, order_id, cur_time);
            }
        }
    }
    user_helper->remove(client_name);
    return true;
}

void ITDEngine::load(const json& j_config)
{
    if (j_config.find(PH_FEE_SETUP_KEY) != j_config.end())
    {
        default_fee_handler = FeeHandlerPtr(new FeeHandler(j_config[PH_FEE_SETUP_KEY]));
    }
    auto iter = j_config.find("accounts");
    if (iter != j_config.end())
    {
        int account_num = iter.value().size();
        KF_LOG_INFO(logger, "[account] number: " << account_num);
        // ITDEngine's resize
        accounts.resize(account_num);
        // base class resize account info structures.
        resize_accounts(account_num);
        int account_idx = 0;
        for (auto& j_account: iter.value())
        {
            const json& j_info = j_account["info"];
            accounts[account_idx] = load_account(account_idx, j_info);
            auto &fee_handler = accounts[account_idx].fee_handler;
            if (fee_handler.get() == nullptr)
            {
                if (j_info.find(PH_FEE_SETUP_KEY) != j_info.end())
                {
                    fee_handler = FeeHandlerPtr(new FeeHandler(j_info[PH_FEE_SETUP_KEY]));
                }
                else
                {
                    if (default_fee_handler.get() == nullptr)
                    {
                        KF_LOG_ERROR(logger, "[client] no fee_handler (idx)" << account_idx);
                        throw std::runtime_error("cannot find fee_handler for account!");
                    }
                    else
                    {
                        fee_handler = default_fee_handler;
                    }
                }
            }
            /** parse client */
            const json& j_clients = j_account["clients"];
            for (auto& j_client: j_clients)
            {
                string client_name = j_client.get<string>();
                ClientInfoUnit& client_status = clients[client_name];
                client_status.account_index = account_idx;
                client_status.is_alive = false;
                client_status.journal_index = -1;
                KF_LOG_INFO(logger, "[client] (name)" << client_name << " (account)" << accounts[client_status.account_index].InvestorID);
            }
            /** set default */
            if (j_account["is_default"].get<bool>())
                default_account_index = account_idx;
            // add
            account_idx ++;
        }
    }
}

TradeAccount ITDEngine::load_account(int idx, const json& j_account)
{
    KF_LOG_ERROR(logger, "[account] NOT IMPLEMENTED! (content)" << j_account);
    throw std::runtime_error("load_account not implemented yet!");
}
