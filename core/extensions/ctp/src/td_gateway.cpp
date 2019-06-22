//
// Created by qlu on 2019/1/14.
//

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/util/code_convert.h>
#include <kungfu/wingchun/util/instrument.h>
#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/gateway/macro.h>

#include "serialize_ctp.h"
#include "type_convert_ctp.h"
#include "td_gateway.h"

namespace kungfu
{
    namespace ctp
    {
        const std::unordered_map<int, std::string> TdGateway::kDisconnectedReasonMap{
                {0x1001, "网络读失败"},
                {0x1002, "网络写失败"},
                {0x2001, "接收心跳超时"},
                {0x2002, "发送心跳失败"},
                {0x2003, "收到错误报文"}
        };


        TdGateway::TdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double):
                TdGatewayImpl(SOURCE_CTP, config_str["account_id"])
        {
            front_uri_ = config_str["td_uri"];
            broker_id_ = config_str["broker_id"];
            account_id_ = config_str["account_id"];
            password_ = config_str["password"];
            front_id_ = -1;
            session_id_ = -1;
            order_ref_ = -1;
            request_id_ = 0;
        }

        void TdGateway::init()
        {
            TdGatewayImpl::init();
            spdlog::set_default_logger(get_logger());
            std::string order_mapper_db_file = ORDER_MAPPER_DB_FILE(this->get_name());
#ifdef _WINDOWS
            std::replace(order_mapper_db_file.begin(), order_mapper_db_file.end(), '/', '\\');
#endif
            order_mapper_ = std::shared_ptr<kungfu::ctp::OrderMapper>(new kungfu::ctp::OrderMapper(order_mapper_db_file));
        }

        void TdGateway::start()
        {
            std::string runtime_folder = RUNTIME_FOLDER;
#ifdef  _WINDOWS
            std::replace(runtime_folder.begin(), runtime_folder.end(), '/', '\\');
#endif
            SPDLOG_INFO("create ctp td api with path: {}", runtime_folder);
            api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(runtime_folder.c_str());
            api_->RegisterSpi(this);
            api_->RegisterFront((char*)front_uri_.c_str());
            api_->SubscribePublicTopic(THOST_TERT_QUICK);
            api_->SubscribePrivateTopic(THOST_TERT_QUICK);
            api_->Init();

            GatewayImpl::start();
        }

        bool TdGateway::login()
        {
            CThostFtdcReqUserLoginField login_field = {};
            strcpy(login_field.TradingDay, "");
            strcpy(login_field.UserID, account_id_.c_str());
            strcpy(login_field.BrokerID, broker_id_.c_str());
            strcpy(login_field.Password, password_.c_str());
            LOGIN_TRACE(fmt::format("(UserID) {} (BrokerID) {} (Password) ***", login_field.UserID, login_field.BrokerID));
            int rtn = api_->ReqUserLogin(&login_field, ++request_id_);
            if (rtn != 0)
            {
                LOGIN_ERROR(fmt::format("failed to request login, (error_id) {}", rtn));
                return false;
            }
            else
            {
                return true;
            }
        }

        bool TdGateway::req_settlement_confirm()
        {
            CThostFtdcSettlementInfoConfirmField req = {};
            strcpy(req.InvestorID, account_id_.c_str());
            strcpy(req.BrokerID, broker_id_.c_str());
            int rtn = api_->ReqSettlementInfoConfirm(&req, ++request_id_);
            return rtn == 0;
        }


        bool TdGateway::insert_order(const OrderInput &input)
        {
            INSERT_ORDER_TRACE( kungfu::journal::to_string(input));

            CThostFtdcInputOrderField ctp_input;
            memset(&ctp_input, 0, sizeof(ctp_input));

            strcpy(ctp_input.BrokerID, broker_id_.c_str());
            to_ctp(ctp_input, input);

            int order_ref = order_ref_ ++;
            strcpy(ctp_input.OrderRef, std::to_string(order_ref).c_str());
            
            INSERT_ORDER_TRACE( to_string(ctp_input));
            int error_id = api_->ReqOrderInsert(&ctp_input, ++request_id_);

            int64_t nano = kungfu::yijinjing::time::getNanoTime();

            Order order = get_order(input);
            order.insert_time = nano;
            order.update_time = nano;
            order.rcv_time = nano;

            if (error_id != 0)
            {
                order.error_id = error_id;
                order.status = OrderStatusError;

                on_order(order);

                INSERT_ORDER_ERROR(fmt::format("(error_id) {}", error_id));
                return false;
            }
            else
            {
                CtpOrder order_record = {};
                order_record.internal_order_id = input.order_id;
                order_record.broker_id = broker_id_;
                order_record.investor_id = account_id_;
                order_record.exchange_id = input.exchange_id;
                order_record.instrument_id = ctp_input.InstrumentID;
                order_record.order_ref = ctp_input.OrderRef;
                order_record.front_id = front_id_;
                order_record.session_id = session_id_;
                order_record.client_id = input.client_id;
                order_record.parent_id = input.parent_id;
                order_record.insert_time = nano;
                order_mapper_->add_ctp_order(input.order_id, order_record);
                
                INSERT_ORDER_INFO(fmt::format("(FrontID) {} (SessionID) {} (OrderRef) {}", front_id_, session_id_, ctp_input.OrderRef));
                return true;
            }
        }

        bool TdGateway::cancel_order(const OrderAction &action)
        {
            CANCEL_ORDER_TRACE(fmt::format("(order_id) {} (action_id) {}", action.order_id, action.order_action_id));

            CtpOrder order_record = order_mapper_->get_order_info(action.order_id);
            CThostFtdcInputOrderActionField ctp_action = {};
            strcpy(ctp_action.BrokerID, order_record.broker_id.c_str());
            strcpy(ctp_action.InvestorID, order_record.investor_id.c_str());
            strcpy(ctp_action.OrderRef, order_record.order_ref.c_str());
            ctp_action.FrontID = order_record.front_id;
            ctp_action.SessionID = order_record.session_id;
            ctp_action.ActionFlag = THOST_FTDC_AF_Delete;
            strcpy(ctp_action.InstrumentID, order_record.instrument_id.c_str());

            CANCEL_ORDER_TRACE(fmt::format("(FrontID) {} (SessionID) {} (OrderRef)", ctp_action.FrontID, ctp_action.SessionID, ctp_action.OrderRef));

            int error_id = api_->ReqOrderAction(&ctp_action, ++request_id_);
            if (error_id == 0)
            {
                return true;
            }
            else
            {
                CANCEL_ORDER_ERROR(fmt::format("(error_id) {}", error_id));
                return false;
            }
        }

        bool TdGateway::req_account()
        {
            CThostFtdcQryTradingAccountField req = {};
            strcpy(req.BrokerID, broker_id_.c_str());
            strcpy(req.InvestorID, account_id_.c_str());
            int rtn = api_->ReqQryTradingAccount(&req, ++request_id_);
            return rtn == 0;
        }

        bool TdGateway::req_position()
        {
            long_pos_map_.clear();
            short_pos_map_.clear();
            CThostFtdcQryInvestorPositionField req = {};
            strcpy(req.BrokerID, broker_id_.c_str());
            strcpy(req.InvestorID, account_id_.c_str());
            int rtn = api_->ReqQryInvestorPosition(&req, ++request_id_);
            return rtn == 0;
        }

        bool TdGateway::req_position_detail()
        {
            CThostFtdcQryInvestorPositionDetailField req = {};
            strcpy(req.BrokerID, broker_id_.c_str());
            strcpy(req.InvestorID, account_id_.c_str());
            int rtn = api_->ReqQryInvestorPositionDetail(&req, ++request_id_);
            return rtn == 0;
        }

        bool TdGateway::req_qry_instrument()
        {
            future_instruments_.clear();
            CThostFtdcQryInstrumentField req = {};
            int rtn = api_->ReqQryInstrument(&req, ++request_id_);
            return rtn == 0;
        }

        void TdGateway::OnFrontConnected()
        {
            CONNECT_INFO();
            set_state(GatewayState::Connected);
            login();
        }

        void TdGateway::OnFrontDisconnected(int nReason)
        {
            auto it = kDisconnectedReasonMap.find(nReason);
            if (it != kDisconnectedReasonMap.end())
            {
                DISCONNECTED_ERROR(fmt::format("(nReason) {} (Info) {}", nReason, it->second));
                set_state(GatewayState::DisConnected, it->second);
            }
            else
            {
                DISCONNECTED_ERROR(fmt::format("(nReason) {}", nReason));
                set_state(GatewayState::DisConnected);
            }
        }

        void TdGateway::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                       int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                std::string utf_msg = gbk2utf8(pRspInfo->ErrorMsg);
                LOGIN_ERROR(fmt::format("[OnRspUserLogin] (ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, utf_msg));
                set_state(GatewayState::LoggedInFailed, utf_msg);
            }
            else
            {
                LOGIN_INFO(fmt::format("[OnRspUserLogin] (Bid) {} (Uid) {} (SName) {} (TradingDay) {} (FrontID) {} (SessionID) {}", pRspUserLogin->BrokerID,pRspUserLogin->UserID, pRspUserLogin->SystemName, pRspUserLogin->TradingDay, pRspUserLogin->FrontID, pRspUserLogin->SessionID));

                session_id_ = pRspUserLogin->SessionID;
                front_id_ = pRspUserLogin->FrontID;
                order_ref_ = atoi(pRspUserLogin->MaxOrderRef);

                set_state(GatewayState::LoggedIn);

                req_settlement_confirm();
            }
        }

        void TdGateway::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                        int nRequestID, bool bIsLast)
        {}

        void TdGateway::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                std::string utf_msg = gbk2utf8(pRspInfo->ErrorMsg);
                LOGIN_ERROR(fmt::format("[OnRspSettlementInfoConfirm] (ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, utf_msg));
                set_state(GatewayState::SettlementConfirmFailed, utf_msg);
            }
            else
            {
                LOGIN_INFO(fmt::format("[OnRspSettlementInfoConfirm]"));
                set_state(GatewayState::SettlementConfirmed);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                req_qry_instrument();
            }
        }

        void TdGateway::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
                                         int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                auto order_info = order_mapper_->get_order_info_by_order_ref(this->front_id_, this->session_id_, pInputOrder->OrderRef);
                if (order_info.internal_order_id != 0)
                {
                    OrderInput input = {};
                    from_ctp(*pInputOrder, input);

                    auto order = get_order(input);
                    order.order_id = order_info.internal_order_id;
                    order.parent_id = order_info.parent_id;
                    order.insert_time = order_info.insert_time;
                    order.rcv_time = kungfu::yijinjing::time::getNanoTime();
                    strcpy(order.client_id, order_info.client_id.c_str());
                    order.error_id = pRspInfo->ErrorID;
                    strcpy(order.error_msg, pRspInfo->ErrorMsg);
                    order.status = OrderStatusError;

                    on_order(order);
                }
                ORDER_ERROR(fmt::format("[OnRspOrderInsert] (ErrorId) {} (ErrorMsg) {}, (InputOrder) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg), pInputOrder == nullptr ? "" : to_string(*pInputOrder)));
            }
        }

        void TdGateway::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                CANCEL_ORDER_ERROR( fmt::format("[OnRspOrderAction] (ErrorId) {} (ErrorMsg) {} (InputOrderAction) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg), pInputOrderAction ==
                        nullptr ? "" : to_string(*pInputOrderAction)));
            }
        }

        void TdGateway::OnRtnOrder(CThostFtdcOrderField *pOrder)
        {
            ORDER_TRACE(to_string(*pOrder));
            order_mapper_->update_sys_id(pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef, pOrder->ExchangeID, pOrder->OrderSysID);
            auto order_info = order_mapper_->get_order_info_by_order_ref(pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);
            if (order_info.internal_order_id == 0)
            {
                ORDER_ERROR(fmt::format("can't find FrontID {} SessionID {} OrderRef {}", pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef));
            }
            else
            {
                Order order = {};
                from_ctp(*pOrder, order);
                order.order_id = order_info.internal_order_id;
                order.parent_id = order_info.parent_id;
                order.insert_time = order_info.insert_time;
                order.rcv_time = kungfu::yijinjing::time::getNanoTime();
                strcpy(order.client_id, order_info.client_id.c_str());

                on_order(order);
            }
        }

        void TdGateway::OnRtnTrade(CThostFtdcTradeField *pTrade)
        {
            TRADE_TRACE(to_string(*pTrade));
            auto order_info = order_mapper_->get_order_info_by_sys_id(pTrade->ExchangeID, pTrade->OrderSysID);

            if (order_info.internal_order_id == 0)
            {
                TRADE_ERROR( fmt::format("can't find ExchangeID {} and OrderSysID {}", pTrade->ExchangeID, pTrade->OrderSysID));
            }
            else
            {
                Trade trade = {};
                from_ctp(*pTrade, trade);

                trade.order_id = order_info.internal_order_id;
                trade.parent_order_id = order_info.parent_id;
                trade.rcv_time = kungfu::yijinjing::time::getNanoTime();
                strcpy(trade.client_id, order_info.client_id.c_str());
                on_trade(trade);
            }
        }

        void TdGateway::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                               CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                ACCOUNT_ERROR(fmt::format("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg)));
            }
            else
            {
                ACCOUNT_TRACE(to_string(*pTradingAccount));

                AccountInfo account = {};
                strcpy(account.account_id, get_account_id().c_str());
                account.account_type = AccountTypeFuture;
                from_ctp(*pTradingAccount, account);
                int64_t nano = kungfu::yijinjing::time::getNanoTime();
                account.rcv_time = nano;
                account.update_time = nano;
                on_account(account);

                set_state(GatewayState::AccountInfoConfirmed);
                std::this_thread::sleep_for(std::chrono::seconds(1));    
                req_position();
            }
        }

        void TdGateway::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                                 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                POSITION_ERROR(fmt::format("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg)));
            }
            else
            {
                if (pInvestorPosition != nullptr)
                {
                    POSITION_TRACE(to_string(*pInvestorPosition));
                    Position pos = {};
                    from_ctp(*pInvestorPosition, pos);
                    int64_t nano = kungfu::yijinjing::time::getNanoTime();
                    pos.rcv_time = nano;
                    pos.update_time = nano;
                    auto& pos_map = pos.direction == DirectionLong ? long_pos_map_ : short_pos_map_;
                    auto iter = pos_map.find(std::string(pInvestorPosition->InstrumentID));
                    if (iter != pos_map.end())
                    {
                        pos.cost_price = (pos.cost_price * pos.volume + iter->second.cost_price * iter->second.volume) /
                                (pos.volume + iter->second.volume);
                        pos.volume += iter->second.volume;
                        pos.yesterday_volume += iter->second.yesterday_volume;
                        pos.margin += iter->second.margin;
                        pos.realized_pnl += iter->second.realized_pnl;
                        pos.unrealized_pnl += iter->second.unrealized_pnl;
                        pos.position_pnl = pos.realized_pnl + pos.unrealized_pnl;
                    }
                    pos_map[std::string(pInvestorPosition->InstrumentID)] = pos;
                    on_position(pos, bIsLast);
                }

                if (bIsLast)
                {
                    long_pos_map_.clear();
                    short_pos_map_.clear();
                    set_state(GatewayState::PositionInfoConfirmed);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    req_position_detail();
                }
            }
        }

        void TdGateway::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
                                                       CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                                       bool bIsLast)
        {
            if(pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                POSITION_DETAIL_ERROR(fmt::format("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg)));
                set_state(GatewayState::PositionDetailConfirmFailed);
            }
            else
            {
                if (pInvestorPositionDetail != nullptr)
                {
                    POSITION_DETAIL_TRACE(to_string(*pInvestorPositionDetail));
                    Position pos_detail = {};
                    from_ctp(*pInvestorPositionDetail, pos_detail);
                    int64_t nano = kungfu::yijinjing::time::getNanoTime();
                    pos_detail.rcv_time = nano;
                    pos_detail.update_time = nano;
                    on_position_detail(pos_detail, bIsLast);
                }
                if (bIsLast)
                {
                    set_state(GatewayState::Ready);
                }
            }
        }

        void TdGateway::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo,
                                           int nRequestID, bool bIsLast)
        {
            if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
            {
                INSTRUMENT_ERROR(fmt::format("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg)));
                set_state(GatewayState::InstrumentInfoConfirmFailed);
            }
            else
            {
                INSTRUMENT_TRACE(kungfu::ctp::to_string(*pInstrument));
                if (pInstrument->ProductClass == THOST_FTDC_PC_Futures)
                {
                    FutureInstrument instrument;
                    from_ctp(*pInstrument, instrument);
                    INSTRUMENT_TRACE(kungfu::journal::to_string(instrument));
                    future_instruments_.emplace_back(instrument);
                }
                if (bIsLast)
                {
                    set_state(GatewayState::InstrumentInfoConfirmed);

                    FutureInstrumentStorage(FUTURE_INSTRUMENT_DB_FILE).set_future_instruments(future_instruments_);
                    nlohmann::json j;
                    get_publisher()->publish(msg::type::ReloadFutureInstrument, j);

                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    req_account();
                }
            }
        }
    }
}

