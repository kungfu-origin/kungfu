//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>

#include <kungfu/wingchun/msg.h>

#include "trader.h"
#include <typeinfo>


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

            void PassiveTrader::on_start()
            {
                gateway::Trader::on_start();

                events_ | is(msg::type::PassiveCtrl) |
                $([&](event_ptr e)
                  {
                    on_passivectrl(e);
                  });
            }
            
            void PassiveTrader::on_passivectrl(const yijinjing::event_ptr& event)
            {
                auto writer = writers_[0];//get_writer(e->source());
                const nlohmann::json &data = event->data<nlohmann::json>();
                SPDLOG_INFO("received {}", data.dump());
                if (int(data["msg_type"])  == msg::type::GatewayState)
                {
                    msg::data::GatewayState &massage = writer->open_data<msg::data::GatewayState>(kungfu::yijinjing::time::now_in_nano(), msg::type::GatewayState);
                    massage = (msg::data::GatewayState)(data["state"]);
                    writer->close_data();
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::Quote))
                {
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::OrderInput))
                {
                    msg::data::OrderInput &massage = writer->open_data<msg::data::OrderInput>(kungfu::yijinjing::time::now_in_nano(), msg::type::OrderInput);
                    from_json(data["data"], massage);
                    writer->close_data();
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::Order))
                {
                    msg::data::Order &massage = writer->open_data<msg::data::Order>(kungfu::yijinjing::time::now_in_nano(), msg::type::Order);
                    from_json(data["data"], massage);
                    writer->close_data();
                }
                else if  (data["msg_type"] == static_cast<int>(msg::type::Trade))
                {
                    msg::data::Trade &massage = writer->open_data<msg::data::Trade>(kungfu::yijinjing::time::now_in_nano(), msg::type::Trade);
                    from_json(data["data"], massage);
                    writer->close_data();
                }
                else if  (data["msg_type"] == static_cast<int>(msg::type::Position))
                {
                }
                else if  (data["msg_type"] == static_cast<int>(msg::type::AssetInfo))
                {
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::PositionDetail))
                {
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::PositionEnd))
                {
                }
                else if (data["msg_type"] == static_cast<int>(msg::type::PositionDetailEnd))
                {
                }
            }
        }
    }
}