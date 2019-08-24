//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>
#include <time.h>
#include <string.h>
#include "marketdata_sim.h"

using namespace kungfu::rx;
using namespace kungfu::yijinjing;

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            MarketDataSim::MarketDataSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config) :
                    MarketData(low_latency, std::move(locator), SOURCE_PASSIVE)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_PASSIVE);
                nlohmann::json config = nlohmann::json::parse(json_config);
                SPDLOG_INFO("[json_config] {}",json_config);
                if (config.find("md_parameter") != config.end())
                {
                    for (nlohmann::json::iterator it =config["md_parameter"].begin(); it != config["md_parameter"].end(); ++it)
                    {
                        //合约、数据类型、幅值、周期、前收价、行情时间间隔
                        MdParameter mdp;
                        strcpy(mdp.instrument_id ,it.key().c_str());
                        mdp.type = DataType(it.value()["type"]);
                        mdp.cycle = it.value()["cycle"];
                        mdp.limit = it.value()["limit"];
                        mdp.pre_price = it.value()["pre"];
                        mdp.interval = it.value()["interval"];
                        md_parameters_.push_back(mdp);
                    }
                    init_md();
                }
                else
                {
                    SPDLOG_ERROR("no md_parameter was found");
                }
            }

            bool MarketDataSim::init_md()
            {
                for (auto mdp : md_parameters_)
                {
                    msg::data::Quote quote;
                    strcpy(quote.source_id, "sim");
                    strcpy(quote.trading_day, "");
                    quote.data_time = time::now_in_nano();
                    strcpy(quote.instrument_id, mdp.instrument_id);
                    if ((quote.instrument_id[0] >= '0') && (quote.instrument_id[0] <= '9'))
                    {
                        if (quote.instrument_id[0] >= '3')
                            strcpy(quote.exchange_id, "SSE");
                        else
                            strcpy(quote.exchange_id, "SZE");
                        quote.pre_close_price = 0;
                        quote.pre_settlement_price = mdp.pre_price;
                    } else
                    {
                        strcpy(quote.exchange_id, "SHFE");
                        quote.pre_close_price = mdp.pre_price;
                        quote.pre_settlement_price = 0;
                    }
                    quote.volume = 0;
                    quote.last_price = mdp.pre_price;
                    quote.open_price = mdp.pre_price;
                    quote.high_price = mdp.pre_price;
                    quote.low_price = mdp.pre_price;
                    quote.upper_limit_price = mdp.pre_price;
                    quote.lower_limit_price = mdp.pre_price;
                    for (int i = 0; i < 10; i++)
                    {
                        quote.ask_price[i] = 0;
                        quote.ask_volume[i] = 0;
                        quote.bid_price[i] = 0;
                        quote.bid_volume[i] = 0;
                    }
                    nlohmann::json j;
                    msg::data::to_json(j, quote);
                    SPDLOG_TRACE("[QUOTE] {}", j.dump());
                    mds_[std::string(mdp.instrument_id)] = quote;
                }
                return true;
            }

            bool MarketDataSim::subscribe(const std::vector <msg::data::Instrument> &instruments)
            {
                return false;
            }

            bool MarketDataSim::unsubscribe(const std::vector <msg::data::Instrument> &instruments)
            {
                return false;
            }

            void MarketDataSim::on_start()
            {
                MarketData::on_start();

                events_ | time_interval(std::chrono::seconds(1)) |
                $([&](event_ptr e)
                  {
                      creat_md();
                  });
            }

            void MarketDataSim::creat_md()
            {
                auto writer = writers_[0];
                int timep = int(kungfu::yijinjing::time::now_in_nano() / 1e9);
                for (auto mdp : md_parameters_)
                {
                    if (mdp.interval == 0)
                    {
                        for (int i = 0; i<10000; i++)
                        {
                            msg::data::Quote &quote = writer->open_data<msg::data::Quote>(
                                    kungfu::yijinjing::time::now_in_nano(), msg::type::Quote);

                            quote = mds_[std::string(mdp.instrument_id)];
                            quote.data_time = kungfu::yijinjing::time::now_in_nano();
                            mds_[std::string(quote.instrument_id)].volume += 10;
                            quote.volume = mds_[std::string(quote.instrument_id)].volume;
                            if (mdp.type == DataType::StandardSine)
                                sin_quota(quote, timep, mdp);
                            else if (mdp.type == DataType::StandardLine)
                                line_quota(quote, timep, mdp);
                            else if (mdp.type == DataType::StandardRandom)
                                random_quota(quote, timep, mdp);
                            nlohmann::json j;
                            msg::data::to_json(j, quote);
                            SPDLOG_TRACE("[QUOTE] {}", j.dump());
                            writer->close_data();
                        }
                    }
                    else if (timep % mdp.interval == 0)
                    {
                        msg::data::Quote &quote = writer->open_data<msg::data::Quote>(
                                kungfu::yijinjing::time::now_in_nano(), msg::type::Quote);

                        quote = mds_[std::string(mdp.instrument_id)];
                        quote.data_time = kungfu::yijinjing::time::now_in_nano();
                        mds_[std::string(quote.instrument_id)].volume += 10;
                        quote.volume = mds_[std::string(quote.instrument_id)].volume;
                        if (mdp.type == DataType::StandardSine)
                            sin_quota(quote, timep, mdp);
                        else if (mdp.type == DataType::StandardLine)
                            line_quota(quote, timep, mdp);
                        else if (mdp.type == DataType::StandardRandom)
                            random_quota(quote, timep, mdp);
                        nlohmann::json j;
                        msg::data::to_json(j, quote);
                        SPDLOG_TRACE("[QUOTE] {}", j.dump());
                        writer->close_data();
                    }
                }
            }

            int MarketDataSim::get_inter_val(char *instrument_id)
            {
                if ((instrument_id[0] >= '0') && (instrument_id[0] <= '9'))
                    return 3;
                else
                    return 1;
            }

            void MarketDataSim::static_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp)
            {
                quote.last_price = mdp.pre_price;
            }

            void MarketDataSim::sin_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp)
            {
                quote.last_price = mdp.pre_price +
                                   (mdp.pre_price * double(mdp.limit) / 100.0) * sin(time / double(mdp.cycle));
                quote.last_price = int(quote.last_price * 100.00 + 0.5) / 100.00;
            }

            void MarketDataSim::line_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp)
            {
                quote.last_price = pow(-1, int(time) / int(mdp.cycle)) * (mdp.limit / mdp.cycle * time -
                                    int(time + mdp.cycle) / (2 * int(mdp.cycle* 2 * mdp.limit))) -
                                   1.0 * mdp.limit / 2;
                quote.last_price = int(quote.last_price * 100.00 + 0.5) / 100.00;
            }

            void MarketDataSim::random_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp)
            {
                double last_price = mds_[std::string(quote.instrument_id)].last_price;
                double pre_price = std::max(mds_[std::string(quote.instrument_id)].pre_close_price,
                                            mds_[std::string(quote.instrument_id)].pre_settlement_price);
                double up_limit_price = pre_price * (1 + mdp.limit / 100);
                double low_limit_price = pre_price * (1 - mdp.limit / 100);
                if (pow(-1, int(time) / int(mdp.cycle)) > 0)
                    quote.last_price = last_price + (up_limit_price - last_price) * (rand() % 100 / 100.0);
                else
                    quote.last_price = last_price + (low_limit_price - last_price) * (rand() % 100 / 100.0);
            }
        }
    }
}
