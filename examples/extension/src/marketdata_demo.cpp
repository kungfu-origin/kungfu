#include "marketdata_demo.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            MarketDataDemo::MarketDataDemo(bool low_latency, yijinjing::data::locator_ptr locator,
                                 std::map<std::string, std::string> &config_str,
                                 std::map<std::string, int> &config_int,
                                 std::map<std::string, double> &config_double) : broker::MarketData(low_latency, std::move(locator), "kfext_demo")
            {
                SPDLOG_INFO("Connecting kfext demo MD");
            }

            void MarketDataDemo::on_start()
            {
                broker::MarketData::on_start();
                SPDLOG_INFO("Start kfext demo MD");
            }

            bool MarketDataDemo::subscribe(const std::vector<wingchun::msg::data::Instrument> &instruments)
            {
                return false;
            }

            bool MarketDataDemo::unsubscribe(const std::vector<wingchun::msg::data::Instrument> &instruments)
            {
                return false;
            }
        }
    }
}
