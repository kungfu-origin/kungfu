//
// Created by qlu on 2019/1/16.
//
#include <nlohmann/json.hpp>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/event_loop/event_loop.h>
#include <kungfu/wingchun/reqrsp.h>
#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/oms/def.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    void EventLoop::register_nanotime_callback(int64_t nano, TSCallback callback)
    {
        scheduler_->insert_callback_at(nano, callback);
    }

    void EventLoop::register_nanotime_callback_at_next(const char *time_str, kungfu::TSCallback callback)
    {
        scheduler_->insert_callback_at_next(time_str, callback);
    }

    void EventLoop::register_quote_callback(QuoteCallback callback)
    {
        quote_callback_ = callback;
    }

    void EventLoop::register_entrust_callback(EntrustCallback callback)
    {
        entrust_callback_ = callback;
    }

    void EventLoop::register_transaction_callback(TransactionCallback callback)
    {
        transaction_callback_ = callback;
    }

    void EventLoop::register_req_login_callback(ReqLoginCallback callback)
    {
        login_callback_ = callback;
    }

    void EventLoop::register_subscribe_callback(SubscribeCallback callback)
    {
        sub_callback_ = callback;
    }

    void EventLoop::register_order_input_callback(OrderInputCallback callback)
    {
        order_input_callback_ = callback;
    }

    void EventLoop::register_order_action_callback(OrderActionCallback callback)
    {
        order_action_callback_ = callback;
    }

    void EventLoop::register_order_callback(OrderCallback callback)
    {
        order_callback_ = callback;
    }

    void EventLoop::register_trade_callback(TradeCallback callback)
    {
        trade_callback_ = callback;
    }

    void EventLoop::register_algo_order_input_callback(AlgoOrderInputCallback callback)
    {
        algo_order_input_callback_ = callback;
    }

    void EventLoop::register_algo_order_status_callback(AlgoOrderStatusCallback callback)
    {
        algo_order_status_callback_ = callback;
    }

    void EventLoop::register_algo_order_action_callback(AlgoOrderActionCallback callback)
    {
        algo_order_action_callback_ = callback;
    }

    void EventLoop::register_manual_order_input_callback(kungfu::ManualOrderInputCallback callback)
    {
        manual_order_input_callback_ = callback;
    }

    void EventLoop::register_manual_order_action_callback(kungfu::ManualOrderActionCallback callback)
    {
        manual_order_action_callback_ = callback;
    }

    void EventLoop::register_reload_instruments_callback(kungfu::ReloadInstrumentsCallback callback)
    {
        reload_instruments_callback_ = callback;
    }

    void EventLoop::register_switch_day_callback(SwitchDayCallback callback)
    {
        switch_day_callback_ = callback;
    }

    int64_t EventLoop::get_nano() const
    {
        int64_t nano = scheduler_->get_nano();
        return nano > 0 ? nano : kungfu::yijinjing::time::now_in_nano();
    }

    void EventLoop::handle(const yijinjing::event_ptr event)
    {
        scheduler_->update_nano(event->gen_time());
        auto msg_type = static_cast<MsgType>(event->msg_type());
        switch (msg_type)
        {
            case MsgType::Quote:
            {
                if (quote_callback_)
                {
                    quote_callback_(event->data<Quote>());
                }
                break;
            }
            case MsgType::Entrust:
            {
                if (entrust_callback_)
                {
                    entrust_callback_(event->data<Entrust>());
                }
                break;
            }
            case MsgType::Transaction:
            {
                if (transaction_callback_)
                {
                    transaction_callback_(event->data<Transaction>());
                }
                break;
            }
            case  MsgType::OrderInput:
            {
                if (order_input_callback_)
                {
                    order_input_callback_(event->data<OrderInput>());
                }
                break;
            }
            case MsgType::OrderAction:
            {
                if (order_action_callback_)
                {
                    order_action_callback_(event->data<OrderAction>());
                }
                break;
            }
            case MsgType::Order:
            {
                if (order_callback_)
                {
                    order_callback_(event->data<Order>());
                }
                break;
            }
            case MsgType::Trade:
            {
                if (trade_callback_)
                {
                    trade_callback_(event->data<Trade>());
                }
                break;
            }
            case MsgType::SwitchDay:
            {
                if (switch_day_callback_)
                {
                    switch_day_callback_(event->data<nlohmann::json>());
                }
            }
            case MsgType::AlgoOrderInput:
            {
                std::string js((char *) event->data<char>());
                try
                {
                    nlohmann::json j = nlohmann::json::parse(js);
                    AlgoOrderInput msg = j;
                    if(algo_order_input_callback_)
                    {
                        algo_order_input_callback_(msg.order_id, msg.client_id, msg.algo_type, msg.input);
                    }
                }
                catch (std::exception& e)
                {
                    SPDLOG_ERROR("failed to parse algo order input msg, data[{}], exception: {}", (char*) event->data<char>(), e.what());
                }
                break;
            }
            case MsgType::AlgoOrderStatus:
            {
                std::string js((char*) event->data<char>());
                try
                {
                    nlohmann::json j = nlohmann::json::parse(js);
                    AlgoOrderStatus msg = j;
                    if(algo_order_status_callback_)
                    {
                        algo_order_status_callback_(msg.order_id, msg.algo_type, msg.status);
                    }
                }
                catch (std::exception& e)
                {
                    SPDLOG_ERROR("failed to parse algo order action msg, data[{}], exception: {}", (char*) event->data<char>(), e.what());
                }
                break;
            }
            case MsgType::AlgoOrderAction:
            {
                std::string js((char*) event->data<char>());
                try
                {
                    nlohmann::json j = nlohmann::json::parse(js);
                    AlgoOrderAction msg = j;
                    if(algo_order_action_callback_)
                    {
                        algo_order_action_callback_(msg.order_id, msg.order_action_id, msg.action);
                    }
                }
                catch (std::exception& e)
                {
                    SPDLOG_ERROR("failed to parse algo order action msg, data[{}], exception: {}", (char*) event->data<char>(), e.what());
                }
                break;
            }
            case MsgType::ReqLogin:
            {
                if (login_callback_)
                {
                    LoginRequest req = event->data<nlohmann::json>();
                    login_callback_(req.source, req.name, req.sender);
                }
                break;
            }
            case MsgType::Subscribe:
            {
                if (sub_callback_)
                {
                    SubscribeRequest req = event->data<nlohmann::json>();
                    sub_callback_(req.source, req.instruments, req.is_level2);
                }
                break;
            }
            case MsgType::ReloadFutureInstrument:
            {
                if (reload_instruments_callback_)
                {
                    reload_instruments_callback_();
                }
                break;
            }
            case MsgType::ReqOrderInput:
            {
                if (manual_order_input_callback_)
                {
                    OrderInput order_input = {};
                    auto data = event->data<nlohmann::json>();
                    strcpy(order_input.instrument_id, data["instrument_id"].get<std::string>().c_str());
                    strcpy(order_input.account_id, data["account_id"].get<std::string>().c_str());
                    strcpy(order_input.client_id, data["client_id"].get<std::string>().c_str());
                    order_input.price_type = data["price_type"].get<std::string>()[0];
                    order_input.limit_price = data["limit_price"].get<double>();
                    order_input.volume = data["volume"].get<int64_t>();
                    order_input.side = data["side"].get<std::string>()[0];
                    order_input.offset = data["offset"].get<std::string>()[0];

                    manual_order_input_callback_(order_input);
                }
            }
            case MsgType::ReqOrderAction:
            {
                if (manual_order_action_callback_)
                {
                    auto data = event->data<nlohmann::json>();
                    std::string account_id = "";
                    if (data.find("account_id") != data.end())
                    {
                        account_id = data["account_id"].get<std::string>();
                    }
                    std::string client_id = "";
                    if (data.find("client_id") != data.end())
                    {
                        client_id = data["client_id"].get<std::string>();
                    }
                    std::vector<uint64_t> order_ids;
                    if (data.find("order_id") != data.end())
                    {
                        const auto& j_orders_ids = data["order_id"];
                        if (j_orders_ids.is_array())
                        {
                            for (auto i = 0; i < j_orders_ids.size(); ++i)
                            {
                                order_ids.emplace_back(std::stoull(j_orders_ids[i].get<std::string>()));
                            }
                        }
                    }
                    manual_order_action_callback_(account_id, client_id, order_ids);
                }
                break;
            }
            default:
            {
                scheduler_->update_nano(yijinjing::time::now_in_nano());
                break;
            }
        }
    }
}
