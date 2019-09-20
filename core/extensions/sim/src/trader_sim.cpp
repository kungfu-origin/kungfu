//
// Created by Keren Dong on 2019-07-11.
//

#include "trader_sim.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            TraderSim::TraderSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config) :
                    Trader(low_latency, std::move(locator), SOURCE_SIM, account_id)
            {
                yijinjing::log::copy_log_settings(get_io_device()->get_home(), account_id);
                nlohmann::json config = nlohmann::json::parse(json_config);
                std::string match_mode =  config.value("match_mode", "custom");
                this->mode_ = get_mode(match_mode);
                if (this->mode_ == MatchMode::Custom)
                {
                    throw wingchun_error("user defined match mode not supported");
                }
            }

            bool TraderSim::insert_order(const yijinjing::event_ptr &event)
            {
                const msg::data::OrderInput &input = event->data<msg::data::OrderInput>();
                msg::data::Order order = {};
                order_from_input(input, order);
                order.insert_time = kungfu::yijinjing::time::now_in_nano();
                order.update_time = kungfu::yijinjing::time::now_in_nano();
                int min_vol = get_instrument_type(input.instrument_id, input.exchange_id) == InstrumentType::Stock ? 100 : 1;
                if (order.volume < min_vol)
                {
                    order.status = OrderStatus::Error;
                }
                else
                {
                    switch (mode_)
                    {
                        case MatchMode::Reject:
                        {
                            order.status = OrderStatus::Error;
                            break;
                        }
                        case MatchMode::Pend:
                        {
                            order.status = OrderStatus::Pending;
                            break;
                        }
                        case MatchMode::Cancel:
                        {
                            order.status = OrderStatus::Cancelled;
                            break;
                        }
                        case MatchMode::Fill:
                        {
                            order.status = OrderStatus::Filled;
                            order.volume_traded = order.volume;
                            order.volume_left = 0;
                            break;
                        }
                        case MatchMode::PartialFillAndCancel:
                        {
                            order.volume_traded = min_vol;
                            order.volume_left = order.volume - order.volume_traded;
                            order.status = order.volume_left == 0 ? OrderStatus::Filled : OrderStatus::PartialFilledNotActive;
                            break;
                        }
                        case MatchMode::PartialFill:
                        {
                            order.volume_traded = min_vol;
                            order.volume_left = order.volume - order.volume_traded;
                            order.status = order.volume_left == 0 ? OrderStatus::Filled : OrderStatus::PartialFilledActive;
                            break;
                        }
                        default:
                        {
                            throw wingchun_error("user defined match mode not supported");
                        }
                    }
                }

                if (order.volume_traded > 0)
                {
                    auto writer = get_writer(event->source());
                    msg::data::Trade &trade = writer->open_data<msg::data::Trade>(0, msg::type::Trade);
                    uint64_t trade_id = writer->current_frame_uid();
                    trade.trade_id = trade_id;
                    trade.order_id = input.order_id;
                    trade.trade_time = kungfu::yijinjing::time::now_in_nano();
                    strcpy(trade.instrument_id, input.instrument_id);
                    strcpy(trade.exchange_id, input.exchange_id);
                    strcpy(trade.account_id, input.account_id);
                    trade.side = input.side;
                    trade.offset = input.offset;
                    trade.price = input.limit_price; //TODO support market order
                    trade.volume = order.volume_traded;
                    writer->close_data();
                }

                if (not is_final_status(order.status))
                {
                    pending_orders_[order.order_id] = order;
                    oid2source_[order.order_id] = event->source();
                }
                get_writer(event->source())->write(0, msg::type::Order, order);
                return true;
            }

            bool TraderSim::cancel_order(const yijinjing::event_ptr &event)
            {
                auto& action = event->data<msg::data::OrderAction>();
                auto order_id = action.order_id;
                auto it = pending_orders_.find(order_id);
                if (it == pending_orders_.end())
                {
                    SPDLOG_ERROR("target order {} is finished", order_id);
                    return false;
                }
                else
                {
                    auto& order =  it->second;
                    order.status = order.volume_traded == 0 ? OrderStatus::Cancelled : OrderStatus::PartialFilledNotActive;
                    if (oid2source_.find(order_id) != oid2source_.end())
                    {
                        auto source = oid2source_[order_id];
                        get_writer(source)->write(0, msg::type::Order, order);
                        oid2source_.erase(order_id);
                    }
                    pending_orders_.erase(it);
                }
                return true;
            }
        }
    }
}