//
// Created by qlu on 2019/2/11.
//

#include <algorithm>
#include <fmt/format.h>

#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/gateway/macro.h>
#include <kungfu/wingchun/util/business_helper.h>

#include "td_gateway.h"
#include "type_convert_xtp.h"
#include "serialize_xtp.h"

namespace kungfu
{
    namespace xtp
    {
        TdGateway::TdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double):
            kungfu::TdGatewayImpl(SOURCE_XTP, TD_GATEWAY_NAME(SOURCE_XTP, config_str["user_id"]))
        {
            client_id_ = config_int["client_id"];
            software_key_ = config_str["software_key"];
            user_ = config_str["user_id"];
            password_ = config_str["password"];
            ip_ = config_str["td_ip"];
            port_ = config_int["td_port"];
            save_file_path_ = config_str["save_file_path"];
            session_id_ = 0;
            request_id_ = 0;
            api_ = nullptr;
            SPDLOG_INFO("Connecting XTP TD for {} at {}:{}", user_, ip_, port_);
        }
        TdGateway::~TdGateway()
        {
            if(api_ != nullptr)
            {
                api_->Release();
                api_ = nullptr;
            }
        }

        void TdGateway::init()
        {
            TdGatewayImpl::init();
            spdlog::set_default_logger(get_logger());
            std::string order_mapper_db_file = fmt::format(ORDER_MAPPER_DB_FILE_FORMAT, get_base_dir(), get_name());
#ifdef _WINDOWS
            std::replace(order_mapper_db_file.begin(), order_mapper_db_file.end(), '/', '\\');
#endif
            order_mapper_ = std::shared_ptr<kungfu::xtp::OrderMapper>(new kungfu::xtp::OrderMapper(order_mapper_db_file));
        }

        void TdGateway::start()
        {
            login();

            GatewayImpl::start();
        }

        bool TdGateway::login()
        {
            if (api_ != nullptr)
            {
                api_->Release();
                api_ = nullptr;
            }
            api_ = XTP::API::TraderApi::CreateTraderApi(client_id_, save_file_path_.c_str());
            api_->RegisterSpi(this);
            api_->SubscribePublicTopic(XTP_TERT_QUICK);//只传送登录后公有流（订单响应、成交回报）的内容
            api_->SetSoftwareVersion("1.1.0");
            api_->SetSoftwareKey(software_key_.c_str());

            int retry = 10;
            while (retry > 0)
            {
                session_id_ = api_->Login(ip_.c_str(), port_, user_.c_str(), password_.c_str(), XTP_PROTOCOL_TCP);
                if (session_id_ > 0)
                {
                    LOGIN_INFO("login success");
                    set_state(GatewayState::LoggedIn);

                    req_account();
                    break;
                }
                else
                {
                    XTPRI* error_info = api_->GetApiLastError();
                    LOGIN_ERROR(fmt::format("(ErrorId) {}, (ErrorMsg){}", error_info->error_id, error_info->error_msg));
                    set_state(GatewayState::LoggedInFailed, std::string(error_info->error_msg));
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    retry--;
                }
            }
            return session_id_ > 0;
        }

        bool TdGateway::insert_order(const OrderInput &input)
        {
            INSERT_ORDER_TRACE(kungfu::journal::to_string(input));
            XTPOrderInsertInfo xtp_input = {};
            to_xtp(xtp_input, input);

            // TODO for test only!!!
            xtp_input.order_client_id = 2;

            INSERT_ORDER_TRACE(to_string(xtp_input));

            uint64_t xtp_order_id = api_->InsertOrder(& xtp_input, session_id_);
            int64_t nano = kungfu::yijinjing::getNanoTime();
            if (xtp_order_id == 0)
            {
                Order order = get_order(input);
                order.insert_time = nano;
                order.update_time = nano;
                order.rcv_time = nano;
                strcpy(order.trading_day, get_calendar()->get_current_trading_day().c_str());

                XTPRI* error_info = api_->GetApiLastError();
                order.error_id = error_info->error_id;
                strcpy(order.error_msg, error_info->error_msg);
                order.status = OrderStatusError;
                on_order(order);

                INSERT_ORDER_ERROR(fmt::format("(input){}(ErrorId){}, (ErrorMsg){}", to_string(xtp_input), error_info->error_id, error_info->error_msg));
                return false;
            }
            else
            {
                XtpOrder info = {};
                info.internal_order_id = input.order_id;
                info.xtp_order_id = xtp_order_id;
                info.parent_id = input.parent_id;
                info.insert_time = nano;
                strcpy(info.client_id, input.client_id);
                strcpy(info.trading_day, get_calendar()->get_current_trading_day().c_str());
                order_mapper_->add_order(info);

                INSERT_ORDER_TRACE(fmt::format("success to insert order, (order_id){} (xtp_order_id) {}", input.order_id, xtp_order_id));
                return true;
            }
        }

        bool TdGateway::cancel_order(const OrderAction &action)
        {
            CANCEL_ORDER_TRACE(kungfu::journal::to_string(action));
            uint64_t xtp_order_id = order_mapper_->get_xtp_order_id(action.order_id);
            if (xtp_order_id != 0)
            {
                auto xtp_action_id = api_->CancelOrder(xtp_order_id, session_id_);
                if (xtp_action_id == 0)
                {
                    XTPRI* error_info = api_->GetApiLastError();
                    CANCEL_ORDER_ERROR(fmt::format("(order_xtp_id) {} (session_id) {} (error_id) {} (error_msg) {}", xtp_order_id, session_id_, error_info->error_id, error_info->error_msg));
                    return false;
                }
                else
                {
                    CANCEL_ORDER_TRACE(fmt::format("success to request cancel order, (order_id){}, (xtp_order_id){}, (xtp_action_id){}", action.order_id, xtp_order_id, xtp_action_id));
                    return true;
                }
            }
            else
            {
                CANCEL_ORDER_TRACE(fmt::format("fail to cancel order, can't find related xtp order id of order_id {}", action.order_id));
                return false;
            }
        }

        bool TdGateway::req_position()
        {
            int rtn = api_->QueryPosition(nullptr, this->session_id_, ++ request_id_);
            return rtn == 0;
        }

        bool TdGateway::req_account()
        {
            int rtn = api_->QueryAsset(session_id_, ++request_id_);
            return rtn == 0;
        }

        void TdGateway::OnDisconnected(uint64_t session_id, int reason)
        {
            if (session_id == session_id_)
            {
                DISCONNECTED_ERROR(fmt::format("(reason) {}", reason));
                set_state(GatewayState::DisConnected, fmt::format("reason {}", reason));
                int retry = 10;
                while (retry > 0)
                {
                    session_id_ = api_->Login(ip_.c_str(), port_, user_.c_str(), password_.c_str(), XTP_PROTOCOL_TCP);
                    if (session_id_ > 0)
                    {
                        LOGIN_INFO("login success");
                        set_state(GatewayState::LoggedIn);
                        req_account();
                        break;
                    }
                    else
                    {
                        XTPRI* error_info = api_->GetApiLastError();
                        LOGIN_ERROR(fmt::format("(ErrorId) {}, (ErrorMsg){}", error_info->error_id, error_info->error_msg));
                        set_state(GatewayState::LoggedInFailed, std::string(error_info->error_msg));
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        retry--;
                    }
                }
            }
        }

        void TdGateway::OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id)
        {
            ORDER_TRACE(fmt::format("(xtp_order_info) {}, (error_id){}, (error_msg){}, (session_id){}",
                                                  order_info == nullptr ? "" : to_string(*order_info),
                                                  error_info == nullptr ? 0: error_info->error_id,
                                                  error_info == nullptr ? "": error_info->error_msg,
                                                  session_id));
            XtpOrder xtp_order = order_mapper_->get_order_by_xtp_order_id(get_calendar()->get_current_trading_day().c_str(), order_info->order_xtp_id);
            if (xtp_order.internal_order_id == 0)
            {
                ORDER_ERROR(fmt::format("unrecognized xtp_order_id: {}, trading_day: {}", order_info->order_xtp_id, get_calendar()->get_current_trading_day()));
            }
            else
            {
                Order order = {};
                from_xtp(*order_info, order);
                order.order_id = xtp_order.internal_order_id;
                order.parent_id = xtp_order.parent_id;
                order.insert_time = xtp_order.insert_time;
                int64_t nano = kungfu::yijinjing::getNanoTime();
                order.update_time = nano;
                order.rcv_time = nano;
                strcpy(order.client_id, xtp_order.client_id);
                strcpy(order.account_id, get_account_id().c_str());
                strcpy(order.trading_day, xtp_order.trading_day);
                order.instrument_type = get_instrument_type(order.instrument_id, order.exchange_id);
                if (error_info != nullptr)
                {
                    order.error_id = error_info->error_id;
                    strncpy(order.error_msg, error_info->error_msg, ERROR_MSG_LEN);
                }
                on_order(order);
            }
        }

        void TdGateway::OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id)
        {
            TRADE_TRACE(fmt::format("(trade_info) {}", to_string(*trade_info)));
            XtpOrder xtp_order = order_mapper_->get_order_by_xtp_order_id(get_calendar()->get_current_trading_day().c_str(),trade_info->order_xtp_id);
            if (xtp_order.internal_order_id == 0)
            {
                TRADE_ERROR(fmt::format("unrecognized xtp_order_id {}, trading_day: {}", trade_info->order_xtp_id, get_calendar()->get_current_trading_day()));
            }
            else
            {
                Trade trade = {};
                from_xtp(*trade_info, trade);
                trade.rcv_time = kungfu::yijinjing::getNanoTime();
                trade.order_id = xtp_order.internal_order_id;
                trade.parent_order_id = xtp_order.parent_id;
                strcpy(trade.client_id, xtp_order.client_id);
                strcpy(trade.account_id, this->get_account_id().c_str());
                trade.instrument_type = get_instrument_type(trade.instrument_id, trade.exchange_id);

                on_trade(trade);
            }
        }

        void TdGateway::OnCancelOrderError(XTPOrderCancelInfo *cancel_info, XTPRI *error_info, uint64_t session_id)
        {
            CANCEL_ORDER_ERROR(fmt::format("(cancel_info){}, (error_id){}, (error_msg){}, (session_id){}", to_string(*cancel_info), error_info->error_id, error_info->error_msg, session_id));
        }

        void TdGateway::OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
        {
            POSITION_TRACE(fmt::format("(position){},(error_id){},(error_msg){}", position != nullptr ? to_string(*position): "",
                                                     error_info != nullptr ? error_info->error_id : 0,
                                                     error_info!= nullptr ? error_info->error_msg : ""));
            if (error_info == nullptr || error_info->error_id == 0 || error_info->error_id == 11000350)
            {
                Position stock_pos = {};
                strcpy(stock_pos.account_id, get_account_id().c_str());
                if (error_info == nullptr || error_info->error_id == 0)
                {
                    from_xtp(*position, stock_pos);
                }
                stock_pos.instrument_type = get_instrument_type(stock_pos.instrument_id, stock_pos.exchange_id);
                stock_pos.direction = DirectionLong;
                int64_t nano = kungfu::yijinjing::getNanoTime();
                stock_pos.rcv_time = nano;
                stock_pos.update_time = nano;
                on_position(stock_pos, is_last);

                if (is_last)
                {
                    set_state(GatewayState::Ready);
                }
            }
            else
            {
                POSITION_ERROR(fmt::format("(error_id) {} (error_msg) {}", error_info->error_id, error_info->error_msg));
            }
        }

        void TdGateway::OnQueryAsset(XTPQueryAssetRsp* asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id)
        {
            ACCOUNT_TRACE(fmt::format("(asset){},(error_id){},(error_msg){}",
                                                    asset!= nullptr ? to_string(*asset): "",
                                                    error_info != nullptr ? error_info->error_id : 0,
                                                    error_info!= nullptr ? error_info->error_msg : ""));

            if (error_info == nullptr || error_info->error_id == 0 || error_info->error_id == 11000350)
            {
                AccountInfo account = {};
                strcpy(account.account_id, get_account_id().c_str());
                account.account_type = AccountTypeStock;
                if (error_info == nullptr || error_info->error_id == 0)
                {
                    from_xtp(*asset, account);
                }
                int64_t nano = kungfu::yijinjing::getNanoTime();
                account.rcv_time = nano;
                account.update_time = nano;
                on_account(account);
                set_state(GatewayState::AccountInfoConfirmed);

                req_position();

            }
            else
            {
                ACCOUNT_ERROR(fmt::format("(error_id) {} (error_msg) {}", error_info->error_id, error_info->error_msg));
            }
        }
    }
}
