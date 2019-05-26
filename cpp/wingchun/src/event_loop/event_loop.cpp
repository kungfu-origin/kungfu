//
// Created by qlu on 2019/1/16.
//
#include <nlohmann/json.hpp>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>

#include <kungfu/time/timer.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/event_loop/event_loop.h>
#include <kungfu/wingchun/reqrsp.h>
#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/oms/def.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    volatile sig_atomic_t EventLoop::signal_received_ = -1;

    void EventLoop::set_logger(std::shared_ptr<spdlog::logger> logger) const
    {
        spdlog::set_default_logger(logger->clone("event_loop"));
    };

    void EventLoop::subscribe_nanomsg(const std::string& url)
    {
        std::shared_ptr<yijinjing::nanomsg::socket> socket = std::shared_ptr<yijinjing::nanomsg::socket>(new yijinjing::nanomsg::socket(AF_SP, NN_SUB));
        socket->connect(url.c_str());
        socket->setsockopt(NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
        socket_vec_.emplace_back(socket);
    }

    void EventLoop::bind_nanomsg(const std::string &url)
    {
        std::shared_ptr<yijinjing::nanomsg::socket> socket = std::shared_ptr<yijinjing::nanomsg::socket>(new yijinjing::nanomsg::socket(AF_SP, NN_REP));
        socket->bind(url.c_str());
        socket_vec_.emplace_back(socket);
    }

    void EventLoop::subscribe_yjj_journal(const std::string& journal_folder, const std::string& journal_name, int64_t offset_nano)
    {
        if (reader_.get() == nullptr)
        {
            reader_ = kungfu::yijinjing::JournalReader::create(journal_folder, journal_name, offset_nano, name_);
        }
        else
        {
            int journal_idx = reader_->addJournal(journal_folder, journal_name);
            reader_->seekTimeJournal(journal_idx, offset_nano);
        }
    }

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

    void EventLoop::register_signal_callback(SignalCallback signal_callback)
    {
        signal_callbacks_.push_back(signal_callback);
    }

    void EventLoop::signal_handler(int signal)
    {
        signal_received_ = signal;
    }

    void EventLoop::run()
    {
        signal(SIGINT, EventLoop::signal_handler);
        signal(SIGTERM, EventLoop::signal_handler);
#ifndef _WINDOWS
        signal(SIGHUP,  EventLoop::signal_handler);
        signal(SIGQUIT, EventLoop::signal_handler);
        signal(SIGKILL, EventLoop::signal_handler);
#endif

        quit_ = false;
        while (! quit_ && signal_received_ < 0)
        {
            iteration();
        }
        if (signal_received_ >= 0)
        {
            SPDLOG_TRACE("signal received {}", signal_received_);
            for (const auto& cb: signal_callbacks_)
            {
                cb(signal_received_);
            }
        }
    }

    void EventLoop::stop()
    {
        quit_= true;
    }

    void EventLoop::iteration()
    {
        if (!low_latency_ && notice_.wait())
        {
            notice_.last_message();
        }
        
        int64_t nano = -1;
        if (reader_ != nullptr)
        {
            yijinjing::FramePtr frame = reader_->getNextFrame();
            if (frame != nullptr)
            {
                nano = frame->getNano();
                int msg_type = frame->getMsgType();
                switch (msg_type)
                {
                    case (int)MsgType::Quote:
                    {
                        Quote* quote_ptr = (Quote*) frame->getData();
                        if (quote_callback_)
                        {
                            quote_callback_(*quote_ptr);
                        }
                        break;
                    }
                    case (int)MsgType::Entrust:
                    {
                        Entrust* entrust_ptr = (Entrust*) frame->getData();
                        if (entrust_callback_)
                        {
                            entrust_callback_(*entrust_ptr);
                        }
                        break;
                    }
                    case (int)MsgType::Transaction:
                    {
                        Transaction* transaction_ptr = (Transaction*) frame->getData();
                        if (transaction_callback_)
                        {
                            transaction_callback_(*transaction_ptr);
                        }
                        break;
                    }
                    case (int) MsgType::OrderInput:
                    {
                        OrderInput* input = (OrderInput*) frame->getData();
                        if (order_input_callback_)
                        {
                            order_input_callback_(*input);
                        }
                        break;
                    }
                    case (int)MsgType::OrderAction:
                    {
                        OrderAction* action = (OrderAction*) frame->getData();
                        if (order_action_callback_)
                        {
                            order_action_callback_(*action);
                        }
                        break;
                    }
                    case (int)MsgType::Order:
                    {
                        Order* order = (Order*) frame->getData();
                        if (order_callback_)
                        {
                            order_callback_(*order);
                        }
                        break;
                    }
                    case (int)MsgType::Trade:
                    {
                        Trade* trade = (Trade*) frame->getData();
                        if (trade_callback_)
                        {
                            trade_callback_(*trade);
                        }
                        break;
                    }
                    case (int)MsgType::AlgoOrderInput:
                    {
                        std::string js((char *) frame->getData());
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
                            SPDLOG_ERROR("failed to parse algo order input msg, data[{}], exception: {}", (char*)frame->getData(), e.what());
                        }
                        break;
                    }
                    case (int)MsgType::AlgoOrderStatus:
                    {
                        std::string js((char*) frame->getData());
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
                            SPDLOG_ERROR("failed to parse algo order action msg, data[{}], exception: {}", (char*)frame->getData(), e.what());
                        }
                        break;
                    }
                    case (int)MsgType::AlgoOrderAction:
                    {
                        std::string js((char*) frame->getData());
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
                            SPDLOG_ERROR("failed to parse algo order action msg, data[{}], exception: {}", (char*)frame->getData(), e.what());
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }

        if (nano == -1)
        {
            nano = yijinjing::getNanoTime();
        }
        scheduler_->update_nano(nano);

        for (const auto& socket: socket_vec_)
        {
            int rc = socket->recv(NN_DONTWAIT);
            if (rc > 0)
            {
                SPDLOG_TRACE("recv: data[{}]\n", socket->last_message());
                try
                {
                    nlohmann::json content = nlohmann::json::parse(socket->last_message());
                    // 兼容req
                    if (content.find("req") != content.end() && content.find("msg_type") == content.end())
                    {
                        content["msg_type"] = content["req"];
                    }
                    NNMsg msg = content;
                    switch (msg.msg_type)
                    {
                        case MsgType::ReqLogin:
                        {
                            if (login_callback_)
                            {
                                LoginRequest req = msg.data;
                                login_callback_(req.recipient, req.sender);
                            }
                            break;
                        }
                        case MsgType::Subscribe:
                        {
                            if (sub_callback_)
                            {
                                SubscribeRequest req = msg.data;
                                sub_callback_(req.recipient, req.instruments, req.is_level2);
                            }
                            break;
                        }
                        case MsgType::OrderInput:
                        {
                            if (order_input_callback_)
                            {
                                OrderInput order_input = msg.data;
                                order_input_callback_(order_input);
                            }
                            break;
                        }
                        case MsgType::OrderAction:
                        {
                            if (order_action_callback_)
                            {
                                OrderAction order_action = msg.data;
                                order_action_callback_(order_action);
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
                                strcpy(order_input.instrument_id, msg.data["instrument_id"].get<std::string>().c_str());
                                strcpy(order_input.account_id, msg.data["account_id"].get<std::string>().c_str());
                                strcpy(order_input.client_id, msg.data["client_id"].get<std::string>().c_str());
                                order_input.price_type = msg.data["price_type"].get<std::string>()[0];
                                order_input.limit_price = msg.data["limit_price"].get<double>();
                                order_input.volume = msg.data["volume"].get<int64_t>();
                                order_input.side = msg.data["side"].get<std::string>()[0];
                                order_input.offset = msg.data["offset"].get<std::string>()[0];

                                manual_order_input_callback_(order_input);
                            }
                        }
                        case MsgType::ReqOrderAction:
                        {
                            if (manual_order_action_callback_)
                            {
                                std::string account_id = "";
                                if (msg.data.find("account_id") != msg.data.end())
                                {
                                    account_id = msg.data["account_id"].get<std::string>();
                                }
                                std::string client_id = "";
                                if (msg.data.find("client_id") != msg.data.end())
                                {
                                    client_id = msg.data["client_id"].get<std::string>();
                                }
                                std::vector<uint64_t> order_ids;
                                if (msg.data.find("order_id") != msg.data.end())
                                {
                                    const auto& j_orders_ids = msg.data["order_id"];
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
                            break;
                        }
                    }
                }
                catch(std::exception &e)
                {
                    SPDLOG_ERROR("failed to parse data[{}], exception: {}", socket->last_message(), e.what());
                    continue;
                }
            }
        }
    }

    int64_t EventLoop::get_nano() const
    {
        int64_t nano = scheduler_->get_nano();
        return nano > 0 ? nano : kungfu::yijinjing::getNanoTime();
    }
}
