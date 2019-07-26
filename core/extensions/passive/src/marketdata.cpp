//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>
#include <time.h>
#include <string.h>
#include "marketdata.h"

using namespace kungfu::rx;
using namespace kungfu::yijinjing;

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            PassiveMarketData::PassiveMarketData(bool low_latency, yijinjing::data::locator_ptr locator,
                                                 std::map <std::string, std::string> &config_str,
                                                 std::map<std::string, int> &config_int,
                                                 std::map<std::string, double> &config_double) :
                    gateway::MarketData(low_latency, std::move(locator), SOURCE_PASSIVE)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_PASSIVE);
                for (auto it = config_str.begin(); it != config_str.end(); it++)
                    SPDLOG_INFO("config_str {}: {}", it->first, it->second);
                for (auto it = config_int.begin(); it != config_int.end(); it++)
                    SPDLOG_INFO("config_int {}: {}", it->first, it->second);
                for (auto it = config_double.begin(); it != config_double.end(); it++)
                    SPDLOG_INFO("config_double {}: {}", it->first, it->second);
                if (config_str.find("md_parameter") != config_str.end())
                {
                    //此处应解析一个字符串{id:[type,cycle,limit,pre],id:[type,cycle,limit,pre]]
                    SPDLOG_INFO("******md_parameter*****: {}", config_str["md_parameter"]);

                    MdParameter md1 = {"rb1901", 2, 10, 3500, 1};
                    MdParameter md2 = {"600000", 2, 10, 13.1, 3};
                    md_parameters_[StandardSine] = {md1, md2};
                    init_md();
                }
            }

            bool PassiveMarketData::init_md()
            {
                for (auto it = md_parameters_.begin(); it != md_parameters_.end(); it++)
                { ;
                    for (auto parameter : it->second)
                    {
                        msg::data::Quote quote;
                        strcpy(quote.source_id, "passive");
                        strcpy(quote.trading_day, "");
                        quote.data_time = time::now_in_nano();
                        strcpy(quote.instrument_id, parameter.InstrumentId);
                        if ((parameter.InstrumentId[0] >= '0') && (parameter.InstrumentId[0] <= '9'))
                        {
                            if (parameter.InstrumentId[0] >= '3')
                                strcpy(quote.exchange_id, "SZE");
                            else
                                strcpy(quote.exchange_id, "SSE");
                            quote.pre_close_price = 0;
                            quote.pre_settlement_price = parameter.PrePrice;
                        } else
                        {
                            strcpy(quote.exchange_id, "SHFE");
                            quote.pre_close_price = parameter.PrePrice;
                            quote.pre_settlement_price = 0;
                        }
                        quote.volume = 0;
                        quote.last_price = parameter.PrePrice;
                        quote.open_price = parameter.PrePrice;
                        quote.high_price = parameter.PrePrice;
                        quote.low_price = parameter.PrePrice;
                        quote.upper_limit_price = parameter.PrePrice;
                        quote.lower_limit_price = parameter.PrePrice;
                        for (int i = 0; i < 10; i++)
                        {
                            quote.ask_price[i] = 0;
                            quote.ask_volume[i] = 0;
                            quote.bid_price[i] = 0;
                            quote.bid_volume[i] = 0;
                        }
                        mds_[std::string(parameter.InstrumentId)] = quote;
                    }
                }
                return true;
            }

            bool PassiveMarketData::subscribe(const std::vector <msg::data::Instrument> &instruments)
            {
                return false;
            }

            bool PassiveMarketData::unsubscribe(const std::vector <msg::data::Instrument> &instruments)
            {
                return false;
            }

            void PassiveMarketData::on_start()
            {
                gateway::MarketData::on_start();

                events_ | is(msg::type::PassiveCtrl) |
                $([&](event_ptr e)
                  {
                      const nlohmann::json &data = e->data<nlohmann::json>();
                      SPDLOG_INFO("received {}", data.dump());
                  });

                events_ | time_interval(std::chrono::seconds(1)) |
                $([&](event_ptr e)
                  {
                      creat_md();
                  });
            }

            void PassiveMarketData::creat_md()
            {
                auto writer = writers_[0];
                int timep = int(kungfu::yijinjing::time::now_in_nano() / 1e9);
                for (auto it = md_parameters_.begin(); it != md_parameters_.end(); it++)
                {
                    for (auto parameter : it->second)
                    {
                        if (timep % parameter.Interval == 0)
                        {
                            msg::data::Quote &quote = writer->open_data<msg::data::Quote>(
                                    kungfu::yijinjing::time::now_in_nano(), msg::type::Quote);

                            quote = mds_[std::string(parameter.InstrumentId)];
                            quote.data_time = kungfu::yijinjing::time::now_in_nano();
                            mds_[std::string(quote.instrument_id)].volume += 10;
                            quote.volume = mds_[std::string(quote.instrument_id)].volume;
                            sin_quota(quote, timep, parameter);
                            //nlohmann::json j;
                            //msg::data::to_json(j, quote);
                            //SPDLOG_TRACE("[QUOTE] {}", j.dump());
                            writer->close_data();
                        }
                    }

                }
            }

            int PassiveMarketData::get_inter_val(char *instrument_id)
            {
                if ((instrument_id[0] >= '0') && (instrument_id[0] <= '9'))
                    return 3;
                else
                    return 1;
            }

            void PassiveMarketData::static_quota(msg::data::Quote &quote, const int &time, const MdParameter &parameter)
            {
                quote.last_price = parameter.PrePrice;
            }

            void PassiveMarketData::sin_quota(msg::data::Quote &quote, const int &time, const MdParameter &parameter)
            {
                quote.last_price = parameter.PrePrice +
                                   (parameter.PrePrice * parameter.Limit / 100.0) * sin(time / parameter.Cycle);
            }

            void PassiveMarketData::line_quota(msg::data::Quote &quote, const int &time, const MdParameter &parameter)
            {
                quote.last_price = pow(-1, int(time) / int(parameter.Cycle)) *
                                   (parameter.Limit / parameter.Cycle * time -
                                    int(time + parameter.Cycle) / (2 * int(parameter.Cycle * 2 * parameter.Limit))) -
                                   1.0 * parameter.Limit / 2;
            }

            void PassiveMarketData::random_quota(msg::data::Quote &quote, const int &time, const MdParameter &parameter)
            {
                double last_price = mds_[std::string(quote.instrument_id)].last_price;
                double pre_price = std::max(mds_[std::string(quote.instrument_id)].pre_close_price,
                                            mds_[std::string(quote.instrument_id)].pre_settlement_price);
                double up_limit_price = pre_price * (1 + parameter.Limit / 100);
                double low_limit_price = pre_price * (1 - parameter.Limit / 100);
                if (pow(-1, int(time) / int(parameter.Cycle)) > 0)
                    quote.last_price = last_price + (up_limit_price - last_price) * (rand() % 100 / 100.0);
                else
                    quote.last_price = last_price + (low_limit_price - last_price) * (rand() % 100 / 100.0);
            }
        }
    }
}
