//
// Created by qlu on 2019/3/14.
//

#include <spdlog/spdlog.h>
#include <nanomsg/pubsub.h>

#include <kungfu/wingchun/util/nn_publisher.h>

namespace kungfu
{
    namespace wingchun
    {
        void NNPublisher::publish(msg::type::MsgType msg_type, nlohmann::json &data) const
        {
            nlohmann::json j;
            j["gen_time"] = yijinjing::time::now_in_nano();
            j["trigger_time"] = yijinjing::time::now_in_nano();
            j["msg_type"] = int(msg_type);
            j["source"] = 0;
            j["data"] = data;
            io_device_->get_publisher()->publish(j.dump());
            SPDLOG_TRACE("nn published {}", j.dump());
        }

        void NNPublisher::publish_order(const msg::data::Order &order) const
        {
            nlohmann::json j = order;
            publish(msg::type::Order, j);
        }

        void NNPublisher::publish_trade(const msg::data::Trade &trade) const
        {
            nlohmann::json j = trade;
            publish(msg::type::Trade, j);
        }

        void NNPublisher::publish_pos(const msg::data::Position &pos) const
        {
            nlohmann::json j = pos;
            publish(msg::type::Position, j);
        }

        void NNPublisher::publish_portfolio_info(const msg::data::PortfolioInfo &portfolio, msg::type::MsgType msg_type) const
        {
            nlohmann::json j = portfolio;
            publish(msg_type, j);
        }

        void NNPublisher::publish_sub_portfolio_info(const msg::data::SubPortfolioInfo &info) const
        {
            nlohmann::json j = info;
            publish(msg::type::SubPortfolioInfo, j);
        }

        void NNPublisher::publish_account_info(const msg::data::AccountInfo &account_info, msg::type::MsgType msg_type) const
        {
            nlohmann::json j = account_info;
            publish(msg_type, j);
        }
    }
}