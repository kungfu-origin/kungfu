//
// Created by qlu on 2019/1/16.
//

#include "event_loop.h"
#include "reqrsp.h"
#include "serialize.h"
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>
#include "nlohmann/json.hpp"
#include "Timer.h"
#include "util/include/nanomsg_util.h"
#include "oms/include/def.h"

namespace kungfu
{
    volatile sig_atomic_t EventLoop::signal_received_ = -1;

    void EventLoop::subscribe_nanomsg(const std::string& url)
    {
        std::shared_ptr<nn::socket> socket = std::shared_ptr<nn::socket>(new nn::socket(AF_SP, NN_SUB));
        socket->connect(url.c_str());
        socket->setsockopt(NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
        socket_vec_.emplace_back(socket);
    }

    void EventLoop::bind_nanomsg(const std::string &url)
    {
        std::shared_ptr<nn::socket> socket = std::shared_ptr<nn::socket>(new nn::socket(AF_SP, NN_REP));
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
        for (int s = 1; s < 32; s++)
        {
            signal(s, EventLoop::signal_handler);
        }
        
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
                        ;
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
            char* buf = nullptr;
            int rc = socket->recv(&buf, NN_MSG, NN_DONTWAIT); // non-blocking
            if (rc > 0)
            {
                SPDLOG_TRACE("recv: data[{}]\n", buf);
                try
                {
                    nlohmann::json content = nlohmann::json::parse(std::string(buf));
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
                        default:
                        {
                            break;
                        }
                    }
                }
                catch(std::exception &e)
                {
                    SPDLOG_ERROR("failed to parse data[{}], exception: {}", (char*)buf, e.what());
                    continue;
                }
            }
            if (buf != nullptr)
            {
                nn::freemsg(buf);
            }
        }
    }

    int64_t EventLoop::get_nano() const
    {
        int64_t nano = scheduler_->get_nano();
        return nano > 0 ? nano : kungfu::yijinjing::getNanoTime();
    }
}
