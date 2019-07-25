#include "td_gateway.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            TdGateway::TdGateway(bool low_latency, yijinjing::data::locator_ptr locator,
                                 std::map<std::string, std::string> &config_str,
                                 std::map<std::string, int> &config_int,
                                 std::map<std::string, double> &config_double) :
                    gateway::Trader(low_latency, std::move(locator), "kfext_demo_td", "kfext_demo_td")
            {
                SPDLOG_INFO("Connecting kfext demo TD");
            }

            void TdGateway::on_start()
            {
                gateway::Trader::on_start();
                SPDLOG_INFO("Start kfext demo TD");
            }

            bool TdGateway::insert_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool TdGateway::cancel_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool TdGateway::req_position()
            {
                return false;
            }

            bool TdGateway::req_account()
            {
                return false;
            }
        }
    }
}
