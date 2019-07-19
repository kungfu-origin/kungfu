//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>
#include <time.h>

#include "marketdata.h"

using namespace kungfu::rx;
using namespace kungfu::yijinjing;

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            PassiveMarketData::PassiveMarketData(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str,
                              std::map<std::string, int> &config_int,
                              std::map<std::string, double> &config_double) :
                              gateway::MarketData(low_latency, std::move(locator), SOURCE_PASSIVE)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), SOURCE_PASSIVE);
            }

            bool PassiveMarketData::subscribe(const std::vector<msg::data::Instrument> &instruments)
            {
                return false;
            }

            bool PassiveMarketData::unsubscribe(const std::vector<msg::data::Instrument> &instruments)
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
                    write_1();
                });
            }

            void PassiveMarketData::write_1()
            {
                auto writer = writers_[0];
                int timep = kungfu::yijinjing::time::now_in_nano()/1e9;
                SPDLOG_INFO("time {}", timep);
                msg::data::Quote &quote = writer->open_data<msg::data::Quote>(kungfu::yijinjing::time::now_in_nano(), msg::type::Quote);
                sin_quota(quote, 20, 340, 3400, double(timep));
                writer->close_data();
            }

            void PassiveMarketData::sin_quota(msg::data::Quote &quote, double cycle, double amplitude, double start_price, double pass_time)
            {
                quote.last_price = start_price + amplitude*sin(pass_time/cycle);
            }


        }
    }
}
