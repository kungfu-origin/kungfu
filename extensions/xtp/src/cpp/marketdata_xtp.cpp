//
// Created by qlu on 2019/2/11.
//

#include "marketdata_xtp.h"
#include "serialize_xtp.h"
#include "type_convert_xtp.h"

namespace kungfu::wingchun::xtp {
MarketDataXTP::MarketDataXTP(broker::BrokerVendor &vendor)
    : MarketData(vendor), api_(nullptr) {
  config_ = nlohmann::json::parse(json_config);
  if (config_.client_id < 1 or config_.client_id > 99) {
    throw wingchun_error("client_id must between 1 and 99");
  }
}

MarketDataXTP::~MarketDataXTP() {
  if (api_ != nullptr) {
    api_->Release();
  }
}

void MarketDataXTP::on_start() {
  auto md_ip = config_.md_ip.c_str();
  auto account_id = config_.account_id.c_str();
  auto password = config_.password.c_str();
  auto protocol_type = get_xtp_protocol_type(config_.protocol);
  std::string runtime_folder = get_runtime_folder();
  SPDLOG_INFO("Connecting XTP MD for {} at {}://{}:{}", account_id, config_.protocol, md_ip, config_.md_port);
  api_ = XTP::API::QuoteApi::CreateQuoteApi(config_.client_id, runtime_folder.c_str());
  if (config_.protocol == "udp") {
    api_->SetUDPBufferSize(config_.buffer_size);
  }
  api_->RegisterSpi(this);
  if (api_->Login(md_ip, config_.md_port, account_id, password, protocol_type) == 0) {
    update_broker_state(BrokerState::LoggedIn);
    update_broker_state(BrokerState::Ready);
    SPDLOG_INFO("login success! (account_id) {}", config_.account_id);
    api_->QueryAllTickers(XTP_EXCHANGE_SH);
    api_->QueryAllTickers(XTP_EXCHANGE_SZ);
  } else {
    update_broker_state(BrokerState::LoginFailed);
    SPDLOG_ERROR("failed to login, [{}] {}", api_->GetApiLastError()->error_id, api_->GetApiLastError()->error_msg);
  }
}

bool MarketDataXTP::subscribe(const std::vector<InstrumentKey> &instrument_keys) {
  bool result = true;
  std::vector<std::string> sse_tickers;
  std::vector<std::string> sze_tickers;
  for (const auto &inst : instrument_keys) {
    std::string ticker = inst.instrument_id;
    if (strcmp(inst.exchange_id, EXCHANGE_SSE) == 0) {
      sse_tickers.push_back(ticker);
    } else if (strcmp(inst.exchange_id, EXCHANGE_SZE) == 0) {
      sze_tickers.push_back(ticker);
    }
  }
  if (!sse_tickers.empty()) {
    result = result && subscribe(sse_tickers, EXCHANGE_SSE);
  }
  if (!sze_tickers.empty()) {
    result = result && subscribe(sze_tickers, EXCHANGE_SZE);
  }
  return result;
}

bool MarketDataXTP::subscribe(const std::vector<std::string> &instruments, const std::string &exchange_id) {
  int size = instruments.size();
  std::vector<char *> insts;
  insts.reserve(size);
  for (auto &s : instruments) {
    insts.push_back((char *)&s[0]);
  }
  XTP_EXCHANGE_TYPE exchange;
  to_xtp(exchange, (char *)exchange_id.c_str());
  int level1_result = api_->SubscribeMarketData(insts.data(), size, exchange);
  int level2_result = api_->SubscribeTickByTick(insts.data(), size, exchange);
  SPDLOG_INFO("subscribe {} from {}, l1 rtn code {}, l2 rtn code {}", size, exchange_id, level1_result, level2_result);
  return level1_result == 0 and level2_result == 0;
}

bool MarketDataXTP::subscribe_all() {
  auto result = api_->SubscribeAllMarketData() && api_->SubscribeAllTickByTick();
  SPDLOG_INFO("subscribe all, rtn code {}", result);
  return result;
}

void MarketDataXTP::OnDisconnected(int reason) {
  SPDLOG_ERROR("disconnected with reason {}", reason);
  update_broker_state(BrokerState::DisConnected);
}

void MarketDataXTP::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) {
  if (error_info != nullptr && error_info->error_id != 0) {
    SPDLOG_ERROR("failed to subscribe level 1, [{}] {}", error_info->error_id, error_info->error_msg);
  }
}

void MarketDataXTP::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {
  if (error_info != nullptr && error_info->error_id != 0) {
    SPDLOG_ERROR("failed to subscribe level 2, [{}] {}", error_info->error_id, error_info->error_msg);
  }
}

void MarketDataXTP::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {
  if (error_info != nullptr && error_info->error_id != 0) {
    SPDLOG_ERROR("failed to subscribe level 2 all, [{}] {}", error_info->error_id, error_info->error_msg);
  }
}

void MarketDataXTP::OnQueryAllTickers(XTPQSI *ticker_info, XTPRI *error_info, bool is_last) {
  Instrument &quote = get_writer(0)->open_data<Instrument>(0);
  from_xtp(ticker_info, quote);
  get_writer(0)->close_data();
}

void MarketDataXTP::OnDepthMarketData(XTPMD *market_data, int64_t *bid1_qty, int32_t bid1_count, int32_t max_bid1_count,
                                      int64_t *ask1_qty, int32_t ask1_count, int32_t max_ask1_count) {
  Quote &quote = get_writer(0)->open_data<Quote>(0);
  from_xtp(*market_data, quote);
  get_writer(0)->close_data();
  if (max_bid1_count + max_ask1_count != 0) {
    TopOfBook &top_of_book = get_writer(0)->open_data<TopOfBook>(0);
    from_xtp(*market_data, bid1_qty, bid1_count, max_bid1_count, ask1_qty, ask1_count, max_ask1_count, top_of_book);
    get_writer(0)->close_data();
  }
}

void MarketDataXTP::OnTickByTick(XTPTBT *tbt_data) {
  if (tbt_data->type == XTP_TBT_ENTRUST) {
    Entrust &entrust = get_writer(0)->open_data<Entrust>(0);
    from_xtp(*tbt_data, entrust);
    get_writer(0)->close_data();
  } else if (tbt_data->type == XTP_TBT_TRADE) {
    Transaction &transaction = get_writer(0)->open_data<Transaction>(0);
    from_xtp(*tbt_data, transaction);
    get_writer(0)->close_data();
  }
}
} // namespace kungfu::wingchun::xtp
