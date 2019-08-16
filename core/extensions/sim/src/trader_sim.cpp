//
// Created by Keren Dong on 2019-07-11.
//

#include <utility>

#include <kungfu/wingchun/msg.h>

#include "trader_sim.h"
#include <typeinfo>


using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            TraderSim::TraderSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config) :
                    Trader(low_latency, std::move(locator), SOURCE_PASSIVE, account_id)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), account_id);
                SPDLOG_INFO("[account] {}", account_id);
            }

            void TraderSim::rtn_order_from(const msg::data::OrderInput &input, OrderStatus status, int error_id)
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
                        pending_orders_[order_id_] = order;
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

            bool TraderSim::insert_order(const yijinjing::event_ptr &event)
            {
                
                const msg::data::OrderInput &input = event->data<msg::data::OrderInput>();

                if (mds_.find(std::string(input.instrument_id)) == mds_.end())
                {
                    rtn_order_from(input, OrderStatus::Error, 1);
                    return false;
                } else
                {
                    rtn_order_from(input, OrderStatus::Submitted);
                    if (input.price_type == PriceType::Any)
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
                        } 
                        else
                        {
                            if (input.limit_price <= mds_[std::string(input.instrument_id)].last_price)
                                rtn_order_from(input, OrderStatus::Filled);
                            else
                                rtn_order_from(input, OrderStatus::Pending);
                            //add dic
                        }
                    }
                }
                order_id_ +=1;
                return true;
            }

            bool TraderSim::cancel_order(const yijinjing::event_ptr &event)
            {
                return false;
            }

            bool TraderSim::req_position()
            {
                return false;
            }

            bool TraderSim::req_account()
            {
                return false;
            }

            void TraderSim::on_start()
            {
                Trader::on_start();
                add_md("sim");

                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      on_md(event->data<msg::data::Quote>(), event->source());
                  });

            }

            void TraderSim::add_md(std::string source_)
            {
                SPDLOG_INFO("try to add md {} ", source_);
                auto md_location = data::location::make(data::mode::LIVE, data::category::MD, source_, source_,
                                                  get_io_device()->get_home()->locator);
                if (not has_location(md_location->uid))
                {
                    throw wingchun_error(fmt::format("invalid md {}", source_));
                }
                request_read_from(now(), md_location->uid, true);
                SPDLOG_INFO("added md {} [{:08x}]", source_, md_location->uid);
            }

            void TraderSim::on_md(const msg::data::Quote &quote, uint32_t source)
            {
                nlohmann::json _quote;
                msg::data::to_json(_quote, quote);
                SPDLOG_INFO("[quote] {}", _quote.dump());
                mds_[std::string(quote.instrument_id)] = quote;
                for (auto &val : pending_orders_)
                {
                    if (std::strcmp(quote.instrument_id,val.second.instrument_id) == 0)
                    {
                        if (((val.second.side == Side::Buy) && (val.second.limit_price >= mds_[std::string(val.second.instrument_id)].last_price)) || ((val.second.side == Side::Buy) && (val.second.limit_price <= mds_[std::string(val.second.instrument_id)].last_price)))
                        {
                            auto writer = writers_[0];
                            int64_t nano = time::now_in_nano();
                            msg::data::Order &order = writer->open_data<msg::data::Order>(nano, msg::type::Order);
                            order = val.second;
                            order.volume_traded = order.volume;
                            writer->close_data();
                            msg::data::Trade &trade = writer->open_data<msg::data::Trade>(nano, msg::type::Trade);
                            strcpy(trade.instrument_id, val.second.instrument_id);
                            strcpy(trade.exchange_id, val.second.exchange_id);
                            trade.price = val.second.limit_price;
                            trade.volume = val.second.volume;
                            trade.trade_id = writer->current_frame_uid();
                            trade.trade_time = nano;
                            strcpy(trade.account_id, this->get_account_id().c_str());
                            writer->close_data();
                        } 
                    }
                }
            }
        }
    }
}