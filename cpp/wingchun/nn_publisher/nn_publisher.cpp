//
// Created by qlu on 2019/3/14.
//

#include "nn_publisher.h"
#include "../include/serialize.h"
#include <nanomsg/pubsub.h>
#include "spdlog/spdlog.h"

namespace kungfu
{
    NNPublisher::NNPublisher(const std::string& url)
    {
        pub_socket_ = std::shared_ptr<nn::socket>(new nn::socket(AF_SP, NN_PUB));
        pub_socket_->bind(url.c_str());
    }

    void NNPublisher::set_logger(std::shared_ptr<spdlog::logger> logger) const
    {
        spdlog::set_default_logger( logger->clone("nn_publisher"));
    }

    void NNPublisher::publish(kungfu::MsgType msg_type, nlohmann::json& data) const
    {
        nlohmann::json j;
        j["msg_type"] = int(msg_type);
        j["data"] = data;
        std::string js = j.dump();
        SPDLOG_TRACE(js);
        pub_socket_->send(js.c_str(), js.length() + 1, 0);
    }

    void NNPublisher::publish_order(const kungfu::journal::Order &order) const
    {
        nlohmann::json j = order;
        publish(kungfu::MsgType::Order, j);
    }

    void NNPublisher::publish_trade(const kungfu::journal::Trade &trade) const
    {
        nlohmann::json j = trade;
        publish(kungfu::MsgType::Trade, j);
    }

    void NNPublisher::publish_pos(const kungfu::flying::Position& pos) const
    {
        nlohmann::json j = pos;
        publish(kungfu::MsgType::Position, j);
    }

    void NNPublisher::publish_portfolio_info(const kungfu::flying::PortfolioInfo &portfolio, kungfu::MsgType msg_type) const
    {
        nlohmann::json j = portfolio;
        publish(msg_type, j);
    }

    void NNPublisher::publish_sub_portfolio_info(const kungfu::flying::SubPortfolioInfo &info) const
    {
        nlohmann::json j = info;
        publish(kungfu::MsgType::SubPortfolioInfo, j);
    }

    void NNPublisher::publish_account_info(const kungfu::flying::AccountInfo &account_info, kungfu::MsgType msg_type) const
    {
        nlohmann::json j = account_info;
        publish(msg_type, j);
    }
}