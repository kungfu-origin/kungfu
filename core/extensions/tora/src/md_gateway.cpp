//
// Created by PolarAir on 2019-06-18.
//

#include "md_gateway.h"
#include "type_convert_tora.h"
#include "serialize_tora.h"
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/gateway/macro.h>
#include <kungfu/wingchun/util/code_convert.h>
#include <fmt/format.h>
#include <array>

namespace kungfu
{
    namespace tora
    {
        MdGateWay::MdGateWay(std::map <string, string> &config_str, std::map<int> &config_int,
                             std::map<double> &config_double)
                             : kungfu::MdGatewayImpl(SOURCE_TORA), api_(nullptr)
        {
            kungfu::yijinjing::log::copy_log_settings(get_name());

            front_address_ = config_str["front_address"];
            user_id_ = config_str["user_id"];
            password_ = config_str["password"];
        }

        MdGateWay::~MdGateway()
        {
            if (nullptr != api_)
            {
                api_->Release();
                api_ = nullptr;
            }
        }

        void MdGateWay::init()
        {
            if (nullptr == api_)
            {
                api_ = CTORATstpMdApi::CreateTstpMdApi();
                api_->RegisterSpi(this);
                api_->RegisterFront((char*)(front_address_.c_str()));
                api_->Init();
                SPDLOG_INFO("[Init] Api Version: {}", api_->GetApiVersion());
            }
            else
            {
                SPDLOG_WARN("[Init] Already Inited");
            }
        }

        bool MdGateWay::subscribe(const std::vector<Instrument> &instruments, bool is_level2)
        {
            std::vector<std::string> sh_tickers;
            std::vector<std::string> sz_tickers;

            for (const auto& ins : instruments)
            {
                if (strcmp(ins.exchange_id, EXCHANGE_SSE) == 0)
                {
                    sh_tickers.emplace_back(ins.instrument_id);
                }
                else if (strcmp(ins.exchange_id, EXCHANGE_SZE) == 0)
                {
                    sz_tickers.emplace_back(ins.instrument_id);
                }
            }

            auto func = [api_](std::vector<std::string>& tickers, const string& exchange) -> bool {
                std::array<char*, tickers.size()> ticker_array;
                for (size_t i = 0; i < tickers.size(); ++i)
                {
                    ticker_array[i] = (char*)(tickers[i].c_str());
                }
                TTORATstpExchangeIDType ex;
                to_tora(exchange.c_str(), ex);
                int ret = api_->SubscribeMarketData(ticker_array.data(), ticker_array.size(), ex);
                if (ret == 0)
                {
                    SUBSCRIBE_INFO(fmt::format("subscribe request success, (size) {} (exchange_id) {}", ticker_array.size(), exchange));
                    return true;
                }
                else
                {
                    SUBSCRIBE_ERROR("failed to subscribe, (exchange_id) {}", exchange);
                    return false;
                }
            };

            auto ret = func(sh_tickers, EXCHANGE_SSE);
            ret = ret && func(sz_tickers, EXCHANGE_SZE);
            return ret;
        }

        bool MdGateWay::unsubscribe(const std::vector<Instrument> &instruments)
        {
            std::vector<std::string> sh_tickers;
            std::vector<std::string> sz_tickers;

            for (const auto& ins : instruments)
            {
                if (strcmp(ins.exchange_id, EXCHANGE_SSE) == 0)
                {
                    sh_tickers.emplace_back(ins.instrument_id);
                }
                else if (strcmp(ins.exchange_id, EXCHANGE_SZE) == 0)
                {
                    sz_tickers.emplace_back(ins.instrument_id);
                }
            }

            auto func = [api_](std::vector<std::string>& tickers, const string& exchange) -> bool {
                std::array<char*, tickers.size()> ticker_array;
                for (size_t i = 0; i < tickers.size(); ++i)
                {
                    ticker_array[i] = (char*)(tickers[i].c_str());
                }
                TTORATstpExchangeIDType ex;
                to_tora(exchange.c_str(), ex);
                int ret = api_->UnSubscribeMarketData(ticker_array.data(), ticker_array.size(), ex);
                if (ret == 0)
                {
                    UNSUBSCRIBE_INFO(fmt::format("unsubscribe request success, (size) {} (exchange_id) {}", ticker_array.size(), exchange));
                    return true;
                }
                else
                {
                    UNSUBSCRIBE_ERROR("failed to unsubscribe, (exchange_id) {}", exchange);
                    return false;
                }
            };

            auto ret = func(sh_tickers, EXCHANGE_SSE);
            ret = ret && func(sz_tickers, EXCHANGE_SZE);
            return ret;
        }

        void MdGateWay::OnFrontConnected()
        {
            SPDLOG_INFO("[Connect] Api Connected");
            set_state(GatewayState::Connected);

            login();
        }

        void MdGateWay::OnFrontDisconnected(int nReason)
        {
            SPDLOG_WARN("[Connect] Api Disconnected. If this is not caused by user logout, api will automatically try to reconnect.");
            set_state(GatewayState::DisConnected);
        }

        void MdGateWay::OnRspError(CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
        {
            if (pRspInfo->ErrorID != 0)
            {
                SPDLOG_ERROR("[RspError] (ErrorId) {}, (ErrorMsg) {}", pRspInfo->ErrorID, kungfu::gbk2utf8(pRspInfo->ErrorMsg));
            }
        }

        void MdGateWay::OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLogin, CTORATstpRspInfoField *pRspInfo,
                                       int nRequestID, bool bIsLast)
        {
            if (pRspInfo->ErrorID == 0)
            {
                LOGIN_INFO(fmt::format("Login Success, (user_id) {}", pRspUserLogin->LogInAccount));
                set_state(GatewayState::LoggedIn, "");
            }
            else
            {
                auto error_msg = kungfu::gbk2utf8(pRspInfo->ErrorMsg);
                LOGIN_ERROR(fmt::format("Login Failed, (ErrorId) {}, (ErrorMsg) {}", pRspInfo->ErrorID, error_msg));
                set_state(GatewayState::LoggedInFailed, error_msg);
            }
        }

        void MdGateWay::OnRspUserLogout(CTORATstpUserLogoutField *pUserLogout, CTORATstpRspInfoField *pRspInfo,
                                        int nRequestID, bool bIsLast)
        {
            if (pRspInfo->ErrorID == 0)
            {
                SPDLOG_INFO("[Logout] Logout Success, (user_id) {}", pUserLogout->UserID));
                set_state(GatewayState::Connected, "");
            }
            else
            {
                auto error_msg = kungfu::gbk2utf8(pRspInfo->ErrorMsg);
                SPDLOG_INFO("[Logout] Login Failed, (ErrorId) {}, (ErrorMsg) {}", pRspInfo->ErrorID, error_msg));
            }
        }

        void MdGateWay::OnRtnDepthMarketData(CTORATstpMarketDataField *pDepthMarketData)
        {
            QUOTE_TRACE(to_string(*pDepthMarketData));
            Quote quote = {};
            from_tora(*pDepthMarketData, quote);
            quote.rcv_time = kungfu::yijinjing::time::now_in_nano();
            on_quote(quote);
        };

        void MdGateWay::login()
        {
            if (nullptr != api_)
            {
                CTORATstpReqUserLoginField req = {};
                strcpy(req.LogInAccount, user_id_.c_str());
                strcpy(req.LogInAccountType, TORA_TSTP_LACT_UserID);
                strcpy(req.Password, password_.c_str());

                int ret = api_->ReqUserLogin(&req, get_req_id());
                if (ret != 0)
                {
                    set_state(GatewayState::LoggedInFailed, "");
                    LOGIN_ERROR(fmt::format("Login Failed, (ret) {}", ret));
                }
            }
        }

        int MdGateWay::get_req_id() const
        {
            static int req_id = 0;
            if (req_id < 0)
                req_id = 0;
            return ++req_id;
        }
    }
}