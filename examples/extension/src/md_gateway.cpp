#include "md_gateway.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            MdGateway::MdGateway(bool low_latency, yijinjing::data::locator_ptr locator,
                                 std::map<std::string, std::string> &config_str,
                                 std::map<std::string, int> &config_int,
                                 std::map<std::string, double> &config_double) : gateway::MarketData(low_latency, std::move(locator), "kfext_demo")
            {
                SPDLOG_INFO("Connecting kfext demo MD");
            }

            void MdGateway::on_start()
            {
                gateway::MarketData::on_start();
                SPDLOG_INFO("Start kfext demo MD");
            }

            bool MdGateway::subscribe(const std::vector<wingchun::msg::data::Instrument> &instruments)
            {
                return false;
            }

            bool MdGateway::unsubscribe(const std::vector<wingchun::msg::data::Instrument> &instruments)
            {
                return false;
            }
        }
    }
}
