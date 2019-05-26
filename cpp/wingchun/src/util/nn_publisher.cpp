//
// Created by qlu on 2019/3/14.
//

#include <spdlog/spdlog.h>
#include <nanomsg/pubsub.h>

#include <kungfu/wingchun/util/nn_publisher.h>
#include <kungfu/wingchun/serialize.h>

namespace kungfu
{
    NNPublisher::NNPublisher(const std::string& url): socket_(AF_SP, NN_PUB)
    {
        socket_.bind(url);
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
        socket_.send_json(j);
        SPDLOG_TRACE("nn published {}", j.dump());
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