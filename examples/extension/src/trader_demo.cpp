#include "trader_demo.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace kfext_demo
        {
            TraderDemo::TraderDemo(bool low_latency, yijinjing::data::locator_ptr locator,
                                 std::map<std::string, std::string> &config_str,
                                 std::map<std::string, int> &config_int,
                                 std::map<std::string, double> &config_double) :
                    broker::Trader(low_latency, std::move(locator), "kfext_demo_td", "kfext_demo_td")
            {
                SPDLOG_INFO("Connecting kfext demo TD");
            }

            void TraderDemo::on_start()
            {
                broker::Trader::on_start();
                SPDLOG_INFO("Start kfext demo TD");
            }

            bool TraderDemo::insert_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool TraderDemo::cancel_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool TraderDemo::req_position()
            {
                return false;
            }

            bool TraderDemo::req_account()
            {
                return false;
            }
        }
    }
}
