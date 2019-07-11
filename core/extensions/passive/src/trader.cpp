//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>

#include <kungfu/wingchun/msg.h>

#include "trader.h"

using namespace kungfu::rx;
using namespace kungfu::yijinjing;

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            PassiveTrader::PassiveTrader(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str,
                                         std::map<std::string, int> &config_int,
                                         std::map<std::string, double> &config_double) :
                    gateway::Trader(low_latency, std::move(locator), SOURCE_PASSIVE, config_str["user_id"])
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), config_str["user_id"]);
            }

            bool PassiveTrader::insert_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool PassiveTrader::cancel_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool PassiveTrader::req_position()
            {
                return false;
            }

            bool PassiveTrader::req_account()
            {
                return false;
            }

            void PassiveTrader::on_start(const rx::observable<yijinjing::event_ptr> &events)
            {
                gateway::Trader::on_start(events);

                events | is(msg::type::PassiveCtrl) |
                $([&](event_ptr e)
                  {
                      const nlohmann::json &data = e->data<nlohmann::json>();
                      SPDLOG_INFO("received {}", data.dump());
                  });
            }
        }
    }
}