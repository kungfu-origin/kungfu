//
// Created by qlu on 2019/1/14.
//

#include <chrono>
#include <kungfu/wingchun/encoding.h>
#include "trader_ctp.h"
#include "serialize_ctp.h"
#include "type_convert_ctp.h"

using namespace kungfu::yijinjing;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            TraderCTP::TraderCTP(bool low_latency, data::locator_ptr locator, const std::string &account_id, const std::string &json_config) :
                    Trader(low_latency, std::move(locator), SOURCE_CTP, account_id), front_id_(-1), session_id_(-1),order_ref_(-1), request_id_(0), api_(nullptr)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_CTP);
                config_ =  nlohmann::json::parse(json_config);
                order_mapper_ = std::make_shared<OrderMapper>(get_app_db_file("order_mapper"));
                trade_mapper_ = std::make_shared<TradeMapper>(get_app_db_file("trade_mapper"));
            }

            void TraderCTP::on_start()
            {
                broker::Trader::on_start();

                if (api_ == nullptr)
                {
                    auto home = get_io_device()->get_home();
                    std::string runtime_folder = home->locator->layout_dir(home, yijinjing::data::layout::LOG);
                    SPDLOG_INFO("create ctp td api with path: {}", runtime_folder);
                    api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(runtime_folder.c_str());
                    api_->RegisterSpi(this);
                    api_->RegisterFront((char *) config_.td_uri.c_str());
                    api_->SubscribePublicTopic(THOST_TERT_QUICK);
                    api_->SubscribePrivateTopic(THOST_TERT_QUICK);
                    api_->Init();
                }
            }

            bool TraderCTP::login()
            {
                CThostFtdcReqUserLoginField login_field = {};
                strcpy(login_field.TradingDay, "");
                strcpy(login_field.UserID, config_.account_id.c_str());
                strcpy(login_field.BrokerID, config_.broker_id.c_str());
                strcpy(login_field.Password, config_.password.c_str());
                SPDLOG_TRACE("(UserID) {} (BrokerID) {} (Password) ***", login_field.UserID, login_field.BrokerID);
                int rtn = api_->ReqUserLogin(&login_field, ++request_id_);
                if (rtn != 0)
                {
                    SPDLOG_ERROR("failed to request login, (error_id) {}", rtn);
                    return false;
                } else
                {
                    return true;
                }
            }

            bool TraderCTP::req_settlement_confirm()
            {
                CThostFtdcSettlementInfoConfirmField req = {};
                strcpy(req.InvestorID, config_.account_id.c_str());
                strcpy(req.BrokerID, config_.broker_id.c_str());
                int rtn = api_->ReqSettlementInfoConfirm(&req, ++request_id_);
                return rtn == 0;
            }

            bool TraderCTP::req_auth()
            {
                struct CThostFtdcReqAuthenticateField req = {};
                strcpy(req.UserID, config_.account_id.c_str());
                strcpy(req.BrokerID, config_.broker_id.c_str());
                if (config_.product_info.length() > 0)
                {
                    strcpy(req.UserProductInfo, config_.product_info.c_str());
                }
                strcpy(req.AppID, config_.app_id.c_str());
                strcpy(req.AuthCode, config_.auth_code.c_str());
                int rtn = this->api_->ReqAuthenticate(&req, ++request_id_);
                if (rtn != 0)
                {
                    SPDLOG_ERROR("failed to req auth, error id = {}", rtn);
                }
                return rtn == 0;
            }

            bool TraderCTP::req_account()
            {
                CThostFtdcQryTradingAccountField req = {};
                strcpy(req.BrokerID, config_.broker_id.c_str());
                strcpy(req.InvestorID, config_.account_id.c_str());
                int rtn = api_->ReqQryTradingAccount(&req, ++request_id_);
                return rtn == 0;
            }

            bool TraderCTP::req_position()
            {
                long_position_map_.clear();
                short_position_map_.clear();
                CThostFtdcQryInvestorPositionField req = {};
                strcpy(req.BrokerID, config_.broker_id.c_str());
                strcpy(req.InvestorID, config_.account_id.c_str());
                int rtn = api_->ReqQryInvestorPosition(&req, ++request_id_);
                return rtn == 0;
            }

            bool TraderCTP::req_position_detail()
            {
                CThostFtdcQryInvestorPositionDetailField req = {};
                strcpy(req.BrokerID, config_.broker_id.c_str());
                strcpy(req.InvestorID, config_.account_id.c_str());
                int rtn = api_->ReqQryInvestorPositionDetail(&req, ++request_id_);
                return rtn == 0;
            }

            bool TraderCTP::req_qry_instrument()
            {
                CThostFtdcQryInstrumentField req = {};
                int rtn = api_->ReqQryInstrument(&req, ++request_id_);
                return rtn == 0;
            }

            bool TraderCTP::insert_order(const yijinjing::event_ptr& event)
            {
                const OrderInput &input = event->data<OrderInput>();

                CThostFtdcInputOrderField ctp_input;
                memset(&ctp_input, 0, sizeof(ctp_input));

                to_ctp(ctp_input, input);
                strcpy(ctp_input.BrokerID, config_.broker_id.c_str());
                strcpy(ctp_input.InvestorID, config_.account_id.c_str());

                int order_ref = order_ref_++;
                strcpy(ctp_input.OrderRef, std::to_string(order_ref).c_str());

                SPDLOG_TRACE(to_string(ctp_input));
                int error_id = api_->ReqOrderInsert(&ctp_input, ++request_id_);
                int64_t nano = kungfu::yijinjing::time::now_in_nano();
                if (error_id != 0)
                {
                    auto writer = get_writer(event->source());
                    msg::data::Order &order = writer->open_data<msg::data::Order>(event->gen_time(), msg::type::Order);
                    order_from_input(input, order);
                    order.insert_time = nano;
                    order.update_time = nano;
                    order.error_id = error_id;
                    order.status = OrderStatus::Error;
                    writer->close_data();
                    SPDLOG_ERROR("failed to insert order {}, (error_id) {}", input.order_id, error_id);
                    return false;
                } else
                {
                    CtpOrder order_record = {};
                    order_record.internal_order_id = input.order_id;
                    order_record.broker_id = config_.broker_id;
                    order_record.investor_id = config_.account_id;
                    order_record.exchange_id = input.exchange_id;
                    order_record.instrument_id = ctp_input.InstrumentID;
                    order_record.order_ref = ctp_input.OrderRef;
                    order_record.front_id = front_id_;
                    order_record.session_id = session_id_;
                    order_record.source = event->source();
                    order_record.parent_id = input.parent_id;
                    order_record.insert_time = nano;
                    order_mapper_->add_order(input.order_id, order_record);
                    SPDLOG_INFO("(FrontID) {} (SessionID) {} (OrderRef) {}", front_id_, session_id_, ctp_input.OrderRef);
                    return true;
                }
            }

            bool TraderCTP::cancel_order(const yijinjing::event_ptr& event)
            {
                const OrderAction &action = event->data<OrderAction>();
                CtpOrder order_record = order_mapper_->get_order_info(action.order_id);
                if (order_record.internal_order_id == 0)
                {
                    auto writer = get_writer(event->source());
                    msg::data::OrderActionError& error = writer->open_data<msg::data::OrderActionError>(event->gen_time(), msg::type::OrderActionError);
                    strncpy(error.error_msg, "failed to find order info", ERROR_MSG_LEN);
                    error.order_id = action.order_id;
                    error.order_action_id = action.order_action_id;
                    writer->close_data();
                    SPDLOG_ERROR("failed to find order info for {}", action.order_id);
                    return false;
                }
                CThostFtdcInputOrderActionField ctp_action = {};
                strcpy(ctp_action.BrokerID, order_record.broker_id.c_str());
                strcpy(ctp_action.InvestorID, order_record.investor_id.c_str());
                strcpy(ctp_action.OrderRef, order_record.order_ref.c_str());
                ctp_action.FrontID = order_record.front_id;
                ctp_action.SessionID = order_record.session_id;
                ctp_action.ActionFlag = THOST_FTDC_AF_Delete;
                strcpy(ctp_action.InstrumentID, order_record.instrument_id.c_str());
                strcpy(ctp_action.ExchangeID, order_record.exchange_id.c_str());
                SPDLOG_TRACE("(FrontID) {} (SessionID) {} (OrderRef)", ctp_action.FrontID, ctp_action.SessionID, ctp_action.OrderRef);

                int error_id = api_->ReqOrderAction(&ctp_action, ++request_id_);
                if (error_id == 0)
                {
                    action_event_map_[request_id_] = action;
                    SPDLOG_TRACE("success to req cancel order {}, order action id {}, request id {}", action.order_id, action.order_action_id, request_id_);
                    return true;
                } else
                {
                    auto writer = get_writer(event->source());
                    msg::data::OrderActionError& error = writer->open_data<msg::data::OrderActionError>(event->gen_time(), msg::type::OrderActionError);
                    error.error_id = error_id;
                    error.order_id = action.order_id;
                    error.order_action_id = action.order_action_id;
                    writer->close_data();
                    SPDLOG_ERROR("failed to cancel order {}, error_id: {}", action.order_id, error_id);
                    return false;
                }
            }

            void TraderCTP::OnFrontConnected()
            {
                SPDLOG_INFO("connected");
                req_auth();
            }

            void TraderCTP::OnFrontDisconnected(int nReason)
            {
                SPDLOG_ERROR("(nReason) {} (Info) {}", nReason, disconnected_reason(nReason));
            }

            void TraderCTP::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                              CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
                }
                else
                {
                    SPDLOG_INFO("success to authenticate");
                    login();
                }
            }

            void TraderCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                           int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                } else
                {
                    SPDLOG_INFO("(Bid) {} (Uid) {} (SName) {} (TradingDay) {} (FrontID) {} (SessionID) {} (TradingDay) {}",
                            pRspUserLogin->BrokerID, pRspUserLogin->UserID, pRspUserLogin->SystemName, pRspUserLogin->TradingDay,
                            pRspUserLogin->FrontID, pRspUserLogin->SessionID, pRspUserLogin->TradingDay);
                    session_id_ = pRspUserLogin->SessionID;
                    front_id_ = pRspUserLogin->FrontID;
                    order_ref_ = atoi(pRspUserLogin->MaxOrderRef);
                    trading_day_ = pRspUserLogin->TradingDay;
                    req_settlement_confirm();
                }
            }

            void TraderCTP::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                            int nRequestID, bool bIsLast)
            {}

            void TraderCTP::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo,
                                                       int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                } else
                {
                    SPDLOG_INFO("success to confirm settlement info");
                    publish_state(BrokerState::Ready);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    req_qry_instrument();
                }
            }

            void TraderCTP::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
                                             int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    auto order_info = order_mapper_->get_order_info_by_order_ref(this->front_id_, this->session_id_, pInputOrder->OrderRef);
                    if (order_info.internal_order_id != 0)
                    {
                        auto writer = get_writer(order_info.source);
                        msg::data::Order &order = writer->open_data<msg::data::Order>(0, msg::type::Order);
                        order.order_id = order_info.internal_order_id;
                        order.parent_id = order_info.parent_id;
                        order.insert_time = order_info.insert_time;
                        order.update_time = kungfu::yijinjing::time::now_in_nano();
                        strcpy(order.trading_day, trading_day_.c_str());
                        strcpy(order.instrument_id, pInputOrder->InstrumentID);
                        strcpy(order.exchange_id, pInputOrder->ExchangeID);
                        strcpy(order.account_id, pInputOrder->InvestorID);
                        order.instrument_type = InstrumentType::Future;
                        order.limit_price = pInputOrder->LimitPrice;
                        order.volume = pInputOrder->VolumeTotalOriginal;
                        order.status = OrderStatus::Error;
                        order.error_id = pRspInfo->ErrorID;
                        strcpy(order.error_msg, gbk2utf8(pRspInfo->ErrorMsg).c_str());
                        from_ctp_direction(pInputOrder->Direction, order.side);
                        from_ctp_comb_offset(pInputOrder->CombOffsetFlag, order.offset);
                        from_ctp_price_type(pInputOrder->OrderPriceType, pInputOrder->VolumeCondition, pInputOrder->TimeCondition, order.price_type);
                        writer->close_data();
                    }
                    SPDLOG_ERROR("(ErrorId) {} (ErrorMsg) {}, (InputOrder) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg), pInputOrder == nullptr ? "" : to_string(*pInputOrder));
                }
            }

            void TraderCTP::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    auto it = action_event_map_.find(nRequestID);
                    if(it != action_event_map_.end())
                    {
                        const auto& action = it->second;
                        uint32_t source = (action.order_action_id >> 32) ^ get_home_uid();
                        if(has_writer(source))
                        {
                            auto writer = get_writer(source);
                            msg::data::OrderActionError& error = writer->open_data<msg::data::OrderActionError>(0, msg::type::OrderActionError);
                            error.error_id = pRspInfo->ErrorID;
                            strncpy(error.error_msg, gbk2utf8(pRspInfo->ErrorMsg).c_str(), ERROR_MSG_LEN);
                            error.order_id = action.order_id;
                            error.order_action_id = action.order_action_id;
                            writer->close_data();
                        }
                        else
                        {
                            SPDLOG_ERROR("has no writer for [{:08x}]", source);
                        }
                    }
                    SPDLOG_ERROR("ErrorId) {} (ErrorMsg) {} (InputOrderAction) {} (nRequestID) {} (bIsLast) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg),
                            pInputOrderAction == nullptr ? "" : to_string(*pInputOrderAction), nRequestID, bIsLast);
                }
            }

            void TraderCTP::OnRtnOrder(CThostFtdcOrderField *pOrder)
            {
                SPDLOG_TRACE(to_string(*pOrder));
                order_mapper_->update_sys_id(pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef, pOrder->ExchangeID, pOrder->OrderSysID);
                auto order_info = order_mapper_->get_order_info_by_order_ref(pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);
                if (order_info.internal_order_id == 0)
                {
                    SPDLOG_ERROR("can't find FrontID {} SessionID {} OrderRef {}", pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);
                }
                else
                {
                    auto writer = get_writer(order_info.source);
                    msg::data::Order &order = writer->open_data<msg::data::Order>(0, msg::type::Order);
                    from_ctp(*pOrder, order);
                    order.order_id = order_info.internal_order_id;
                    order.parent_id = order_info.parent_id;
                    order.insert_time = order_info.insert_time;
                    order.update_time = kungfu::yijinjing::time::now_in_nano();
                    writer->close_data();
                }
            }

            void TraderCTP::OnRtnTrade(CThostFtdcTradeField *pTrade)
            {
                SPDLOG_TRACE(to_string(*pTrade));
                auto order_info = order_mapper_->get_order_info_by_sys_id(pTrade->ExchangeID, pTrade->OrderSysID);
                if (order_info.internal_order_id == 0)
                {
                    SPDLOG_ERROR("can't find ExchangeID {} and OrderSysID {}", pTrade->ExchangeID, pTrade->OrderSysID);
                }
                else
                {
                    auto writer = get_writer(order_info.source);
                    msg::data::Trade &trade = writer->open_data<msg::data::Trade>(0, msg::type::Trade);
                    from_ctp(*pTrade, trade);
                    strncpy(trade.trading_day, trading_day_.c_str(), DATE_LEN);
                    uint64_t trade_id = writer->current_frame_uid();
                    trade.trade_id = trade_id;
                    trade.order_id = order_info.internal_order_id;
                    trade.parent_order_id = order_info.parent_id;
                    writer->close_data();

                    trade_mapper_->save(trade_id, kungfu::yijinjing::time::now_in_nano(), pTrade->TradingDay, pTrade->ExchangeID, pTrade->TradeID);

                }
            }

            void TraderCTP::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                }
                else
                {
                    SPDLOG_TRACE(to_string(*pTradingAccount));
                    auto writer = get_writer(0);
                    msg::data::Asset &account = writer->open_data<msg::data::Asset>(0, msg::type::Asset);
                    strcpy(account.account_id, get_account_id().c_str());
                    from_ctp(*pTradingAccount, account);
                    account.update_time = kungfu::yijinjing::time::now_in_nano();
                    account.holder_uid = get_io_device()->get_home()->uid;
                    writer->close_data();
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    req_position();
                }
            }

            void TraderCTP::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                                     CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                }
                else if(pInvestorPosition != nullptr)
                {
                    SPDLOG_TRACE(to_string(*pInvestorPosition));
                    auto& position_map = pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long ? long_position_map_ : short_position_map_;
                    if(position_map.find(pInvestorPosition->InstrumentID) == position_map.end())
                    {
                        msg::data::Position position = {};
                        strncpy(position.trading_day, pInvestorPosition->TradingDay, DATE_LEN);
                        strncpy(position.instrument_id, pInvestorPosition->InstrumentID, INSTRUMENT_ID_LEN);
                        strncpy(position.exchange_id, pInvestorPosition->ExchangeID, EXCHANGE_ID_LEN);
                        strncpy(position.account_id, pInvestorPosition->InvestorID, ACCOUNT_ID_LEN);
                        position.holder_uid = get_io_device()->get_home()->uid;
                        position.direction = pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long ? Direction::Long : Direction::Short;
                        position_map[pInvestorPosition->InstrumentID] = position;
                    }
                    auto& position = position_map[pInvestorPosition->InstrumentID];
                    auto& inst_info = instrument_map_[pInvestorPosition->InstrumentID];
                    if (strcmp(pInvestorPosition->ExchangeID, EXCHANGE_SHFE) == 0)
                    {
                        if(pInvestorPosition->YdPosition > 0 && pInvestorPosition->TodayPosition <= 0)
                        {
                            position.yesterday_volume = pInvestorPosition->Position;
                        }
                    }
                    else
                    {
                        position.yesterday_volume = pInvestorPosition->Position - pInvestorPosition->TodayPosition;
                    }
                    position.volume += pInvestorPosition->Position;
                    position.margin += pInvestorPosition->ExchangeMargin;
                    if (position.volume > 0)
                    {
                        double cost = position.avg_open_price * (position.volume - pInvestorPosition->Position)* inst_info.contract_multiplier + pInvestorPosition->OpenCost;
                        position.avg_open_price = cost / (position.volume * inst_info.contract_multiplier);
                    }
                }
                if (bIsLast)
                {
                    SPDLOG_TRACE("RequestID {}", nRequestID);
                    auto writer = get_writer(0);
                    for (const auto& kv: long_position_map_)
                    {
                        const auto& position = kv.second;
                        SPDLOG_TRACE(kungfu::wingchun::msg::data::to_string(position));
                        writer->write(0, msg::type::Position, position);
                    }
                    for(const auto& kv: short_position_map_)
                    {
                        const auto& position = kv.second;
                        SPDLOG_TRACE(kungfu::wingchun::msg::data::to_string(position));
                        writer->write(0, msg::type::Position, position);
                    }
                    msg::data::PositionEnd &end = writer->open_data<msg::data::PositionEnd>(0, msg::type::PositionEnd);
                    end.holder_uid = get_io_device()->get_home()->uid;
                    writer->close_data();
                    short_position_map_.clear();
                    long_position_map_.clear();
                }
            }

            void TraderCTP::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
                                                           CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                                           bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                } else
                {
                    auto writer = get_writer(0);
                    if (pInvestorPositionDetail != nullptr)
                    {
                        SPDLOG_TRACE(to_string(*pInvestorPositionDetail));
                        msg::data::PositionDetail &pos_detail = writer->open_data<msg::data::PositionDetail>(0, msg::type::PositionDetail);
                        from_ctp(*pInvestorPositionDetail, pos_detail);
                        pos_detail.update_time = kungfu::yijinjing::time::now_in_nano();
                        CtpTrade trade_info = trade_mapper_->get(pInvestorPositionDetail->OpenDate, pInvestorPositionDetail->ExchangeID, pInvestorPositionDetail->TradeID);
                        if (trade_info.trade_id != 0)
                        {
                            pos_detail.trade_id = trade_info.trade_id;
                            pos_detail.trade_time = trade_info.trade_time;
                        }
                        else
                        {
                            pos_detail.trade_id = writer->current_frame_uid();
                        }
                        writer->close_data();
                    }
                }
                if (bIsLast)
                {
                    SPDLOG_TRACE("RequestID {}", nRequestID);
                    get_writer(0)->mark(0, msg::type::PositionDetailEnd);
                }
            }

            void TraderCTP::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo,
                                               int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, gbk2utf8(pRspInfo->ErrorMsg));
                } else
                {
                    SPDLOG_TRACE(kungfu::wingchun::ctp::to_string(*pInstrument));
                    auto writer = get_writer(0);
                    if (pInstrument->ProductClass == THOST_FTDC_PC_Futures)
                    {
                        msg::data::Instrument &instrument = writer->open_data<msg::data::Instrument>(0, msg::type::Instrument);
                        from_ctp(*pInstrument, instrument);
                        instrument_map_[pInstrument->InstrumentID] = instrument;
                        writer->close_data();
                    }
                    if (bIsLast)
                    {
                        writer->mark(0, msg::type::InstrumentEnd);
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        req_account();
                    }
                }
            }
        }
    }
}

