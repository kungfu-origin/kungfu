//
// Created by qlu on 2019/2/11.
//

#include <fmt/format.h>
#include <kungfu/wingchun/util/code_convert.h>
#include <kungfu/wingchun/gateway/macro.h>

#include <kungfu/log/spdlog_config.h>

#include "md_gateway.h"

#include "serialize_xtp.h"
#include "type_convert_xtp.h"

namespace kungfu
{
    namespace xtp
    {
        MdGateway::MdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double): kungfu::MdGatewayImpl(SOURCE_XTP)
        {
            client_id_ = config_int["client_id"];
            user_ = config_str["user_id"];
            password_ = config_int["password"];
            ip_ = config_str["md_ip"];
            port_ = config_int["md_port"];
            save_file_path_ = config_str["save_file_path"];
            api_ = nullptr;
            SPDLOG_INFO("Connecting XTP MD for {} at {}:{}", user_, ip_, port_);
        }
        MdGateway::~MdGateway()
        {
            if(api_ != nullptr)
            {
                api_->Release();
                api_ = nullptr;
            }
        }

        void MdGateway::init()
        {
            MdGatewayImpl::init();
            spdlog::set_default_logger(get_logger());
        }

        void MdGateway::start()
        {
            login();
            GatewayImpl::start();
        }

        bool MdGateway::login()
        {
            if (api_ != nullptr)
            {
                api_->Release();
                api_ = nullptr;
            }
            api_ = XTP::API::QuoteApi::CreateQuoteApi(client_id_, save_file_path_.c_str());
            api_->RegisterSpi(this);
            int res = api_->Login(ip_.c_str(), port_, user_.c_str(), password_.c_str(), XTP_PROTOCOL_TCP);
            if (res != 0 && res != -2)
            {
                XTPRI* error_info = api_->GetApiLastError();
                LOGIN_ERROR(fmt::format("(ErrorId) {}, (ErrorMsg){}", error_info->error_id, error_info->error_msg));
                set_state(GatewayState::LoggedInFailed, error_info->error_msg);

                client_id_ ++;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                login();

                return false;
            }
            else
            {
                LOGIN_INFO(fmt::format("login success! (user_id) {}", user_));
                set_state(GatewayState::Ready);

                auto subscriptions = get_subscriptions();
                return true;
            }
        }

        bool MdGateway::subscribe(const std::vector<Instrument> &instruments, bool is_level2)
        {
            SUBSCRIBE_TRACE(fmt::format("(size) {}",instruments.size()));
            for (const auto& inst: instruments)
            {
                get_subscription_storage()->add_subscription(inst);
            }

            std::vector<std::string> sse_tickers;
            std::vector<std::string> sze_tickers;
            for (const auto& inst: instruments)
            {
                std::string ticker = inst.instrument_id;
                if (strcmp(inst.exchange_id, EXCHANGE_SSE) == 0)
                {
                    sse_tickers.push_back(ticker);
                }
                else if (strcmp(inst.exchange_id, EXCHANGE_SZE) == 0)
                {
                    sze_tickers.push_back(ticker);
                }
            }
            bool res = true;
            if (sse_tickers.size() > 0)
            {
                res = res && subscribe(sse_tickers, EXCHANGE_SSE, is_level2);
            }
            if (sze_tickers.size() > 0)
            {
                res = res && subscribe(sze_tickers, EXCHANGE_SZE, is_level2);
            }
            return res;
        }

        bool MdGateway::subscribe(const std::vector<std::string> &instruments, const std::string& exchange_id, bool is_level2)
        {
            int size = instruments.size();
            char** insts = new char*[size];
            for (int i = 0; i < size; i++)
            {
                insts[i] = (char*)instruments[i].c_str();
            }
            XTP_EXCHANGE_TYPE exchange;
            to_xtp(exchange, (char*)exchange_id.c_str());
            int rtn = api_->SubscribeMarketData(insts, size, exchange);
            if (rtn == 0)
            {
                SUBSCRIBE_INFO(fmt::format("subscribe request success, (size) {} (exchange_id) {}", size, exchange_id));
            }
            else
            {
                SUBSCRIBE_ERROR("failed to subscribe");
            }
            int level2_rtn = api_->SubscribeTickByTick(insts, size, exchange);
            if (level2_rtn == 0)
            {
                SUBSCRIBE_INFO(fmt::format("subscribe tick by tick request success, (size) {} (exchange_id) {}", size, exchange_id));
            }
            else
            {
                SUBSCRIBE_ERROR("failed to subscribe tick by tick");
            }
            return level2_rtn == 0 && rtn == 0;
        }

        void MdGateway::OnDisconnected(int reason)
        {
            DISCONNECTED_ERROR(fmt::format("(reason) {}", reason));
            set_state(GatewayState::DisConnected, fmt::format("reason {}", reason));
            int retry = 5;
            while (retry > 0)
            {
                int res = api_->Login(ip_.c_str(), port_, user_.c_str(), password_.c_str(), XTP_PROTOCOL_TCP);
                if (res != 0 && res != -2)
                {
                    XTPRI* error_info = api_->GetApiLastError();
                    LOGIN_ERROR(fmt::format("(ErrorId) {}, (ErrorMsg){}", error_info->error_id, error_info->error_msg));
                    set_state(GatewayState::LoggedInFailed, error_info->error_msg);
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    retry--;
                }
                else
                {
                    LOGIN_INFO(fmt::format("login success! (user_id) {}", user_));
                    set_state(GatewayState::Ready);
                    break;
                }
            }
        }

        void MdGateway::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
        {
            if (error_info != nullptr && error_info->error_id != 0)
            {
                SUBSCRIBE_ERROR(fmt::format("(error_id) {} {error_msg} {} (is_last) {}", error_info->error_id, error_info->error_msg, is_last));
            }
            else
            {
                SUBSCRIBE_INFO(fmt::format("(ticker) {} (is_last) {}", ticker->ticker, is_last));
            }
        }

        void MdGateway::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
        {
            if (error_info != nullptr && error_info->error_id != 0)
            {
                SUBSCRIBE_ERROR(fmt::format("(error_id) {} {error_msg} {} (is_last) {}", error_info->error_id, error_info->error_msg, is_last));
            }
            else
            {
                SUBSCRIBE_INFO(fmt::format("(ticker) {} (is_last) {}", ticker->ticker, is_last));
            }
        }

        void MdGateway::OnDepthMarketData(XTPMD *market_data, int64_t *bid1_qty, int32_t bid1_count,
                                          int32_t max_bid1_count, int64_t *ask1_qty, int32_t ask1_count,
                                          int32_t max_ask1_count)
        {
            QUOTE_TRACE(to_string(*market_data));
            Quote quote = {};
            from_xtp(*market_data, quote);
            quote.rcv_time = kungfu::yijinjing::getNanoTime();
            on_quote(quote);
        }

        void MdGateway::OnTickByTick(XTPTBT *tbt_data)
        {
            if (tbt_data->type == XTP_TBT_ENTRUST)
            {
                ENTRUST_TRACE(to_string(*tbt_data));
                Entrust entrust = {};
                from_xtp(*tbt_data, entrust);
                entrust.rcv_time = kungfu::yijinjing::getNanoTime();
                on_entrust(entrust);
            }
            else if (tbt_data->type == XTP_TBT_TRADE)
            {
                TRANSACTION_TRACE(to_string(*tbt_data));
                Transaction transaction = {};
                from_xtp(*tbt_data, transaction);
                transaction.rcv_time = kungfu::yijinjing::getNanoTime();
                on_transaction(transaction);
            }
        }
    }
}
