//
// Created by qlu on 2019/1/10.
//

#include "marketdata_ctp.h"
#include "serialize_ctp.h"
#include "type_convert_ctp.h"
#include <kungfu/yijinjing/time.h>

using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::ctp {

MarketDataCTP::MarketDataCTP(bool low_latency, locator_ptr locator, const std::string &json_config)
    : MarketData(low_latency, std::move(locator), SOURCE_CTP), api_(nullptr), request_id_(0) {
  yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_CTP);
  config_ = nlohmann::json::parse(json_config);
}

void MarketDataCTP::on_start() {
  broker::MarketData::on_start();

  if (api_ == nullptr) {
    std::string runtime_folder = get_runtime_folder();
    api_ = CThostFtdcMdApi::CreateFtdcMdApi(runtime_folder.c_str());
    api_->RegisterSpi(this);
    api_->RegisterFront((char *)config_.md_uri.c_str());
    api_->Init();
  }
}

bool MarketDataCTP::login() {
  CThostFtdcReqUserLoginField login_field = {};
  strcpy(login_field.UserID, config_.account_id.c_str());
  strcpy(login_field.BrokerID, config_.broker_id.c_str());
  strcpy(login_field.Password, config_.password.c_str());

  int rtn = api_->ReqUserLogin(&login_field, ++request_id_);

  return rtn == 0;
}

bool MarketDataCTP::subscribe(const std::vector<InstrumentKey> &instruments) {
  std::vector<std::string> insts;
  for (const auto &ins : instruments) {
    insts.push_back(ins.instrument_id);
  }
  return subscribe(insts);
}

bool MarketDataCTP::subscribe_all() { return false; }

bool MarketDataCTP::unsubscribe(const std::vector<InstrumentKey> &instruments) {
  std::vector<std::string> insts;
  for (const auto &ins : instruments) {
    insts.push_back(ins.instrument_id);
  }
  return unsubscribe(insts);
}

bool MarketDataCTP::subscribe(const std::vector<std::string> &instrument_ids) {
  if (api_ == nullptr) {
    SPDLOG_ERROR("api has not been initialized");
    return false;
  }

  std::vector<char *> insts;
  insts.reserve(instrument_ids.size());
  for (auto &s : instrument_ids) {
    insts.push_back((char *)&s[0]);
  }
  return api_->SubscribeMarketData(insts.data(), insts.size()) == 0;
}

bool MarketDataCTP::unsubscribe(const std::vector<std::string> &instrument_ids) {
  std::vector<char *> insts;
  insts.reserve(instrument_ids.size());
  for (auto &s : instrument_ids) {
    insts.push_back((char *)&s[0]);
  }
  return api_->UnSubscribeMarketData(insts.data(), insts.size()) == 0;
}

void MarketDataCTP::OnFrontConnected() {
  SPDLOG_INFO("connected");
  login();
}

void MarketDataCTP::OnFrontDisconnected(int nReason) {
  update_broker_state(BrokerState::DisConnected);
  SPDLOG_ERROR("disconnected {}", disconnected_reason(nReason));
}

void MarketDataCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                   int nRequestID, bool bIsLast) {
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    update_broker_state(BrokerState::LoginFailed);
    SPDLOG_ERROR("(ErrorId) {} (ErrorMsg) {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
  } else {
    update_broker_state(BrokerState::LoggedIn);
    SPDLOG_INFO("login successfully with BrokerID {} UserID {} at TradingDay {} ", pRspUserLogin->BrokerID, pRspUserLogin->UserID,
                pRspUserLogin->TradingDay);
  }
}

void MarketDataCTP::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                    int nRequestID, bool bIsLast) {}

void MarketDataCTP::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                       CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  SPDLOG_INFO("subscribed {}", pSpecificInstrument->InstrumentID);
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    SPDLOG_ERROR("failed to subscribe [{}] {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
  }
}

void MarketDataCTP::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  SPDLOG_INFO("unsubscribed {}", pSpecificInstrument->InstrumentID);
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    SPDLOG_ERROR("failed to unsubscribe [{}] {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
  }
}

void MarketDataCTP::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
  auto writer = get_writer(location::PUBLIC);
  Quote &quote = writer->open_data<Quote>(now());
  from_ctp(*pDepthMarketData, quote);
  writer->close_data();
}
} // namespace kungfu::wingchun::ctp
