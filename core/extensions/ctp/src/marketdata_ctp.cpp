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
  api_ = CThostFtdcMdApi::CreateFtdcMdApi(get_runtime_folder().c_str());
  api_->RegisterSpi(this);
  api_->RegisterFront((char *)config_.md_uri.c_str());
  api_->Init();
}

bool MarketDataCTP::subscribe(const std::vector<InstrumentKey> &instruments) {
  auto length = instruments.size();
  auto targets = new char *[length];
  for (int i = 0; i < length; i++) {
    targets[i] = const_cast<char *>(instruments[i].instrument_id.value);
  }
  auto rtn = api_->SubscribeMarketData(targets, length);
  delete[] targets;
  return rtn == 0;
}

bool MarketDataCTP::unsubscribe(const std::vector<InstrumentKey> &instruments) {
  auto length = instruments.size();
  auto targets = new char *[length];
  for (int i = 0; i < length; i++) {
    targets[i] = const_cast<char *>(instruments[i].instrument_id.value);
  }
  auto rtn = api_->UnSubscribeMarketData(targets, length);
  delete[] targets;
  return rtn == 0;
}

bool MarketDataCTP::subscribe_all() { return false; }

void MarketDataCTP::OnFrontConnected() {
  CThostFtdcReqUserLoginField login_field = {};
  strcpy(login_field.UserID, config_.account_id.c_str());
  strcpy(login_field.BrokerID, config_.broker_id.c_str());
  strcpy(login_field.Password, config_.password.c_str());
  auto rtn = api_->ReqUserLogin(&login_field, ++request_id_);
  SPDLOG_INFO("connected {}", rtn);
}

void MarketDataCTP::OnFrontDisconnected(int nReason) {
  update_broker_state(BrokerState::DisConnected);
  SPDLOG_INFO("disconnected {}", disconnected_reason(nReason));
}

void MarketDataCTP::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                   int nRequestID, bool bIsLast) {
  if (pRspInfo->ErrorID == 0) {
    update_broker_state(BrokerState::LoggedIn);
    update_broker_state(BrokerState::Ready);
    SPDLOG_INFO("login successfully with BrokerID {} UserID {} at TradingDay {} ", pRspUserLogin->BrokerID,
                pRspUserLogin->UserID, pRspUserLogin->TradingDay);
  } else {
    update_broker_state(BrokerState::LoginFailed);
    SPDLOG_ERROR("failed to login [{}] {}", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
  }
}

void MarketDataCTP::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                    int nRequestID, bool bIsLast) {}

void MarketDataCTP::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                       CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  SPDLOG_INFO("subscribed {} [{}] {}", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void MarketDataCTP::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  SPDLOG_INFO("unsubscribed {} [{}] {}", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void MarketDataCTP::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
  auto writer = get_writer(location::PUBLIC);
  Quote &quote = writer->open_data<Quote>(now());
  from_ctp(*pDepthMarketData, quote);
  writer->close_data();
}
} // namespace kungfu::wingchun::ctp
