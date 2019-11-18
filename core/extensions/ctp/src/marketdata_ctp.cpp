//
// Created by qlu on 2019/1/10.
//

#include <kungfu/yijinjing/time.h>
#include "marketdata_ctp.h"
#include "type_convert_ctp.h"
#include "serialize_ctp.h"
#include "common.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            MarketDataCTP::MarketDataCTP(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config) :
                    MarketData(low_latency, std::move(locator), SOURCE_CTP), api_(nullptr), request_id_(0)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_CTP);
                config_ = nlohmann::json::parse(json_config);
            }

            void MarketDataCTP::on_start()
            {
                broker::MarketData::on_start();

                if (api_ == nullptr)
                {
                    auto home = get_io_device()->get_home();
                    std::string runtime_folder = home->locator->layout_dir(home, yijinjing::data::layout::LOG);
                    SPDLOG_INFO("create ctp md api with path: {}", runtime_folder);
                    api_ = CThostFtdcMdApi::CreateFtdcMdApi(runtime_folder.c_str());
                    api_->RegisterSpi(this);
                    api_->RegisterFront((char *) config_.md_uri.c_str());
                    api_->Init();
                }
            }

            bool MarketDataCTP::login()
            {
                CThostFtdcReqUserLoginField login_field = {};
                strcpy(login_field.UserID, config_.account_id.c_str());
                strcpy(login_field.BrokerID, config_.broker_id.c_str());
                strcpy(login_field.Password, config_.password.c_str());

                int rtn = api_->ReqUserLogin(&login_field, ++request_id_);

                return rtn == 0;

            }

            bool MarketDataCTP::subscribe(const std::vector<msg::data::Instrument> &instruments)
            {
                std::vector<std::string> insts;
                for (const auto &ins : instruments)
                {
                    insts.push_back(ins.instrument_id);
                }
                return subscribe(insts);
            }

            bool MarketDataCTP::subscribe_all()
            {
                return false;
            }

            bool MarketDataCTP::unsubscribe(const std::vector<msg::data::Instrument> &instruments)
            {
                std::vector<std::string> insts;
                for (const auto &ins : instruments)
                {
                    insts.push_back(ins.instrument_id);
                }
                return unsubscribe(insts);
            }

            bool MarketDataCTP::subscribe(const std::vector<std::string> &instrument_ids)
            {
                if (api_ == nullptr)
                {
                    SPDLOG_ERROR("api is not initialized");
                    return false;
                }

                std::vector<char*> insts;
                insts.reserve(instrument_ids.size());
                for(auto& s: instrument_ids)
                {
                    insts.push_back((char*)&s[0]);
                }
                return api_->SubscribeMarketData(insts.data(), insts.size()) == 0;
            }

            bool MarketDataCTP::unsubscribe(const std::vector<std::string> &instrument_ids)
            {
                std::vector<char*> insts;
                insts.reserve(instrument_ids.size());
                for(auto& s: instrument_ids)
                {
                    insts.push_back((char*)&s[0]);
                }
                return api_->UnSubscribeMarketData(insts.data(), insts.size()) == 0;
            }

            void MarketDataCTP::OnFrontConnected()
            {
                SPDLOG_INFO("connected");
                login();
            }

            void MarketDataCTP::OnFrontDisconnected(int nReason)
            {
                publish_state(BrokerState::DisConnected);
                SPDLOG_ERROR("(nReason) {} (Info) {}", nReason, disconnected_reason(nReason));
            }

            void MarketDataCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                               int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    publish_state(BrokerState::LoggedInFailed);
                    SPDLOG_ERROR("(ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
                }
                else
                {
                    publish_state(BrokerState::LoggedIn);
                    SPDLOG_INFO("(BrokerID) {} (UserID) {} (TradingDay) {} ", pRspUserLogin->BrokerID, pRspUserLogin->UserID, pRspUserLogin->TradingDay);
                }
            }

            void MarketDataCTP::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                                int nRequestID, bool bIsLast)
            {}

            void MarketDataCTP::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
                } else
                {
                    SPDLOG_INFO("(Inst) {} (bIsLast) {}", pSpecificInstrument->InstrumentID == nullptr ? "" : pSpecificInstrument->InstrumentID, bIsLast);
                }
            }

            void MarketDataCTP::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                                     CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
            {
                if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
                {
                    SPDLOG_ERROR("(error_id) {} (error_msg) {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
                } else
                {
                    SPDLOG_INFO("(Inst) {} (bIsLast) {}", pSpecificInstrument->InstrumentID, bIsLast);
                }
            }

            void MarketDataCTP::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
            {
                if (pDepthMarketData != nullptr)
                {
                    SPDLOG_TRACE(to_string(*pDepthMarketData));
                    msg::data::Quote &quote = get_writer(0)->open_data<msg::data::Quote>(0, msg::type::Quote);
                    from_ctp(*pDepthMarketData, quote);
                    get_writer(0)->close_data();
                }
            }
        }
    }
}
