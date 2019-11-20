//
// Created by qlu on 2019/2/11.
//

#include <utility>
#include <kungfu/yijinjing/log/setup.h>
#include "marketdata_xtp.h"
#include "serialize_xtp.h"
#include "type_convert_xtp.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace xtp
        {
            MarketDataXTP::MarketDataXTP(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config):
                    MarketData(low_latency, std::move(locator), SOURCE_XTP)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_XTP);
                config_ = nlohmann::json::parse(json_config);
                if(config_.client_id < 1 or config_.client_id > 99)
                {
                    throw wingchun_error("client_id must between 1 and 99");
                }
            }

            MarketDataXTP::~MarketDataXTP()
            {
                if (api_ != nullptr)
                {
                    api_->Release();
                }
            }

            std::string MarketDataXTP::get_runtime_folder() const
            {
                auto home = get_io_device()->get_home();
                return home->locator->layout_dir(home, yijinjing::data::layout::LOG);
            }

            void MarketDataXTP::on_start()
            {
                MarketData::on_start();
                std::string runtime_folder = get_runtime_folder();
                SPDLOG_INFO("Connecting XTP MD for {} at {}://{}:{} with runtime folder {}", config_.user_id, config_.protocol, config_.md_ip, config_.md_port, runtime_folder);
                api_ = XTP::API::QuoteApi::CreateQuoteApi(config_.client_id, runtime_folder.c_str());
                if (config_.protocol == "udp")
                {
                    api_->SetUDPBufferSize(config_.buffer_size);
                }
                api_->RegisterSpi(this);
                int res = api_->Login(config_.md_ip.c_str(), config_.md_port, config_.user_id.c_str(), config_.password.c_str(), get_xtp_protocol_type(config_.protocol));
                if (res == 0)
                {
                    publish_state(BrokerState::LoggedIn);
                    SPDLOG_INFO("login success! (user_id) {}", config_.user_id);
                } else
                {
                    publish_state(BrokerState::LoggedInFailed);
                    XTPRI *error_info = api_->GetApiLastError();
                    SPDLOG_ERROR("(ErrorId) {}, (ErrorMsg){}", error_info->error_id, error_info->error_msg);
                }
            }

            bool MarketDataXTP::subscribe(const std::vector<Instrument> &instruments)
            {
                SPDLOG_INFO("size: {}", instruments.size());
                bool res = true;
                std::vector<std::string> sse_tickers;
                std::vector<std::string> sze_tickers;
                for (const auto &inst: instruments)
                {
                    std::string ticker = inst.instrument_id;
                    if (strcmp(inst.exchange_id, EXCHANGE_SSE) == 0)
                    {
                        sse_tickers.push_back(ticker);
                    } else if (strcmp(inst.exchange_id, EXCHANGE_SZE) == 0)
                    {
                        sze_tickers.push_back(ticker);
                    }
                }
                if (!sse_tickers.empty())
                {
                    res = res && subscribe(sse_tickers, EXCHANGE_SSE);
                }
                if (!sze_tickers.empty())
                {
                    res = res && subscribe(sze_tickers, EXCHANGE_SZE);
                }
                return res;
            }

            bool MarketDataXTP::subscribe(const std::vector<std::string> &instruments, const std::string &exchange_id)
            {
                int size = instruments.size();
                std::vector<char *> insts;
                insts.reserve(size);
                for (auto &s: instruments)
                {
                    insts.push_back((char *) &s[0]);
                }
                XTP_EXCHANGE_TYPE exchange;
                to_xtp(exchange, (char *) exchange_id.c_str());
                int rtn = api_->SubscribeMarketData(insts.data(), size, exchange);
                if (rtn == 0)
                {
                    SPDLOG_INFO("subscribe request success, (size) {} (exchange_id) {}", size, exchange_id);
                } else
                {
                    SPDLOG_ERROR("failed to subscribe");
                }
                int level2_rtn = api_->SubscribeTickByTick(insts.data(), size, exchange);
                if (level2_rtn == 0)
                {
                    SPDLOG_INFO("subscribe tick by tick request success, (size) {} (exchange_id) {}", size, exchange_id);
                } else
                {
                    SPDLOG_ERROR("failed to subscribe tick by tick");
                }
                return level2_rtn == 0 && rtn == 0;
            }

            bool MarketDataXTP::subscribe_all()
            {
                auto res = api_->SubscribeAllMarketData() && api_->SubscribeAllTickByTick();
                SPDLOG_INFO("subscribe all");
                return res;
            }

            void MarketDataXTP::OnDisconnected(int reason)
            {
                SPDLOG_ERROR("(reason) {}", reason);
                publish_state(BrokerState::DisConnected);
            }

            void MarketDataXTP::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
            {
                if (error_info != nullptr && error_info->error_id != 0)
                {
                    SPDLOG_ERROR("(error_id) {} {error_msg} {} (is_last) {}", error_info->error_id, error_info->error_msg, is_last);
                } else
                {
                    SPDLOG_INFO("(ticker) {} (is_last) {}", ticker->ticker, is_last);
                }
            }

            void MarketDataXTP::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
            {
                if (error_info != nullptr && error_info->error_id != 0)
                {
                    SPDLOG_ERROR("(error_id) {} {error_msg} {} (is_last) {}", error_info->error_id, error_info->error_msg, is_last);
                } else
                {
                    SPDLOG_INFO("(ticker) {} (is_last) {}", ticker->ticker, is_last);
                }
            }

            void MarketDataXTP::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info)
            {
                if (error_info != nullptr && error_info->error_id != 0)
                {
                    SPDLOG_ERROR("(error_id) {} {error_msg} {}", error_info->error_id, error_info->error_msg);
                } else
                {
                    SPDLOG_INFO("subscribe all tick by tick success, exchange: {}", exchange_id_from_xtp(exchange_id));
                }
            }

            void MarketDataXTP::OnDepthMarketData(XTPMD *market_data, int64_t *bid1_qty, int32_t bid1_count,
                                                  int32_t max_bid1_count, int64_t *ask1_qty, int32_t ask1_count,
                                                  int32_t max_ask1_count)
            {
                SPDLOG_TRACE(to_string(*market_data));
                msg::data::Quote &quote = get_writer(0)->open_data<msg::data::Quote>(0, msg::type::Quote);
                from_xtp(*market_data, quote);
                get_writer(0)->close_data();
            }

            void MarketDataXTP::OnTickByTick(XTPTBT *tbt_data)
            {
                if (tbt_data->type == XTP_TBT_ENTRUST)
                {
                    SPDLOG_TRACE(to_string(*tbt_data));
                    msg::data::Entrust &entrust = get_writer(0)->open_data<msg::data::Entrust>(0, msg::type::Entrust);
                    from_xtp(*tbt_data, entrust);
                    get_writer(0)->close_data();
                }
                else if (tbt_data->type == XTP_TBT_TRADE)
                {
                    SPDLOG_TRACE(to_string(*tbt_data));
                    msg::data::Transaction &transaction = get_writer(0)->open_data<msg::data::Transaction>(0, msg::type::Transaction);
                    from_xtp(*tbt_data, transaction);
                    get_writer(0)->close_data();
                }
            }
        }
    }
}
