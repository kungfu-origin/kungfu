//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>

#include <kungfu/wingchun/msg.h>

#include "trader.h"
#include <typeinfo>


using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            PassiveTrader::PassiveTrader(bool low_latency, yijinjing::data::locator_ptr locator,
                                         std::map <std::string, std::string> &config_str,
                                         std::map<std::string, int> &config_int,
                                         std::map<std::string, double> &config_double) :
                    gateway::Trader(low_latency, std::move(locator), SOURCE_PASSIVE, config_str["user_id"])
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), config_str["user_id"]);
                SPDLOG_INFO("[account] {}", config_str["user_id"]);
            }

            void PassiveTrader::rtn_order_from(const msg::data::OrderInput &input, OrderStatus status, int error_id)
            {
                auto writer = writers_[0];
                int64_t nano = time::now_in_nano();
                msg::data::Order &order = writer->open_data<msg::data::Order>(nano, msg::type::Order);
                order_from_input(input, order);
                order.update_time = nano;
                order.status = status;
                switch (status)
                {
                    case OrderStatus::Submitted:
                        order.insert_time = nano;
                        break;
                    case OrderStatus::Pending:
                        order.insert_time = nano;
                        break;
                    case OrderStatus::Filled:
                    {
                        order.volume_traded = order.volume;
                        writer->close_data();
                        msg::data::Trade &trade = writer->open_data<msg::data::Trade>(nano, msg::type::Trade);
                        strcpy(trade.instrument_id, input.instrument_id);
                        strcpy(trade.exchange_id, input.exchange_id);
                        trade.price = input.limit_price;
                        trade.volume = input.volume;
                        trade.trade_id = writer->current_frame_uid();
                        trade.trade_time = nano;
                        strcpy(trade.account_id, this->get_account_id().c_str());
                        break;
                    }
                    case OrderStatus::Error:
                    {
                        strcpy(order.error_msg, "error");
                        order.error_id = error_id;
                        break;
                    }
                    case OrderStatus::Cancelled:
                        break;
                    default :
                        break;
                }
                writer->close_data();
            }

            bool PassiveTrader::insert_order(const yijinjing::event_ptr &event)
            {
                const msg::data::OrderInput &input = event->data<msg::data::OrderInput>();

                if (mds_.find(std::string(input.instrument_id)) == mds_.end())
                {
                    rtn_order_from(input, OrderStatus::Error, 1);
                    return false;
                } else
                {
                    rtn_order_from(input, OrderStatus::Submitted);
                    if ((input.price_type == PriceType::Any) || (input.price_type == PriceType::Best) ||
                        (input.price_type == PriceType::Best5))
                        rtn_order_from(input, OrderStatus::Filled);
                    else if (input.price_type == PriceType::Limit)
                    {
                        if (input.side == Side::Buy)
                        {
                            if (input.limit_price >= mds_[std::string(input.instrument_id)].last_price)
                                rtn_order_from(input, OrderStatus::Filled);
                            else
                                rtn_order_from(input, OrderStatus::Pending);
                            //add dic
                        } else
                        {
                            if (input.limit_price <= mds_[std::string(input.instrument_id)].last_price)
                                rtn_order_from(input, OrderStatus::Filled);
                            else
                                rtn_order_from(input, OrderStatus::Pending);
                            //add dic
                        }
                    }
                }
                return true;
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

                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      on_md(event->data<msg::data::Quote>(), event->source());
                  });

            }

            void PassiveTrader::on_md(const msg::data::Quote &quote, uint32_t source)
            {
                //nlohmann::json _quote;
                //msg::data::to_json(_quote, quote);
                //SPDLOG_INFO("[quote] {}", _quote.dump());
                //mds_[std::string(quote.instrument_id)] = quote;
            }

            void PassiveTrader::on_passivectrl(const yijinjing::event_ptr &event)
            {
                auto writer = writers_[0];//get_writer(e->source());
                const nlohmann::json &data = event->data<nlohmann::json>();
                SPDLOG_INFO("received {}", data.dump());
                if (int(data["msg_type"]) == msg::type::GatewayState)
                {
                    msg::data::GatewayState &massage = writer->open_data<msg::data::GatewayState>(
                            time::now_in_nano(), msg::type::GatewayState);
                    massage = (msg::data::GatewayState)(data["state"]);
                    writer->close_data();
                } else if (data["msg_type"] == static_cast<int>(msg::type::Quote))
                {
                } else if (data["msg_type"] == static_cast<int>(msg::type::OrderInput))
                {
                    msg::data::OrderInput &massage = writer->open_data<msg::data::OrderInput>(
                            time::now_in_nano(), msg::type::OrderInput);
                    from_json(data["data"], massage);
                    writer->close_data();
                } else if (data["msg_type"] == static_cast<int>(msg::type::Order))
                {
                    msg::data::Order &massage = writer->open_data<msg::data::Order>(
                            time::now_in_nano(), msg::type::Order);
                    from_json(data["data"], massage);
                    writer->close_data();
                } else if (data["msg_type"] == static_cast<int>(msg::type::Trade))
                {
                    msg::data::Trade &massage = writer->open_data<msg::data::Trade>(
                            time::now_in_nano(), msg::type::Trade);
                    from_json(data["data"], massage);
                    writer->close_data();
                } else if (data["msg_type"] == static_cast<int>(msg::type::Position))
                {
                } else if (data["msg_type"] == static_cast<int>(msg::type::AssetInfo))
                {
                } else if (data["msg_type"] == static_cast<int>(msg::type::PositionDetail))
                {
                } else if (data["msg_type"] == static_cast<int>(msg::type::PositionEnd))
                {
                } else if (data["msg_type"] == static_cast<int>(msg::type::PositionDetailEnd))
                {
                }
            }
        }
    }
}