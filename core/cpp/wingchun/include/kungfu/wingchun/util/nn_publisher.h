//
// Created by qlu on 2019/3/14.
//

#ifndef KUNGFU_NN_PUBLISHER_H
#define KUNGFU_NN_PUBLISHER_H

#include <string>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/io.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        class NNPublisher
        {
        public:
            NNPublisher(yijinjing::io_device_ptr io_device) : io_device_(io_device)
            {};

            void publish_order(const msg::data::Order &order) const;

            void publish_trade(const msg::data::Trade &trade) const;

            void publish_pos(const msg::data::Position &pos) const;

            void publish_portfolio_info(const msg::data::PortfolioInfo &portfolio, msg::type::MsgType msg_type = msg::type::Portfolio) const;

            void publish_account_info(const msg::data::AccountInfo &account_info, msg::type::MsgType msg_type = msg::type::AccountInfo) const;

            void publish_sub_portfolio_info(const msg::data::SubPortfolioInfo &info) const;

            void publish(msg::type::MsgType msg_type, nlohmann::json &data) const;

        private:
            yijinjing::io_device_ptr io_device_;
        };
    }
}

#endif //KUNGFU_NN_PUBLISHER_H
