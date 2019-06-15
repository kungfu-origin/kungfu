//
// Created by qlu on 2019/3/8.
//

#ifndef KUNGFU_WINGCHUN_UTIL_HPP
#define KUNGFU_WINGCHUN_UTIL_HPP

#include <utility>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

#include <kungfu/wingchun/gateway/state.h>
#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/macro.h>

namespace kungfu
{
    namespace gateway
    {
        using namespace kungfu::flying;
        struct GatewayLoginRsp
        {
            AccountType type; // TdGateway 需要返回账号类型
            GatewayState state;
            std::string error_msg;
        };

        inline void to_json(nlohmann::json& j, const GatewayLoginRsp& login_rsp)
        {
            j["type"] = std::string(1, login_rsp.type);
            j["state"] = login_rsp.state;
        }

        inline std::string to_string(const GatewayLoginRsp& login_rsp)
        {
            nlohmann::json j = login_rsp;
            return j.dump();
        }

        inline void from_json(const nlohmann::json& j, GatewayLoginRsp& login_rsp)
        {
            if (j.find("type") != j.end())
            {
                login_rsp.type = j["type"].get<std::string>()[0];
            }
            if (j.find("state") != j.end())
            {
                login_rsp.state = j["state"];
            }
            else
            {
                login_rsp.state = GatewayState::Unknown;
            }
            if(j.find("error_msg") != j.end())
            {
                login_rsp.error_msg = j["error_msg"].get<std::string>();
            }
        }

        inline GatewayLoginRsp login(yijinjing::io_device_client_ptr io_device, yijinjing::data::category c, const std::string& group, const std::string& name, const std::string& sender)
        {
            LoginRequest request;
            request.sender = sender;
            request.source = group;
            request.name = name;
            nlohmann::json js_req = request;

            NNMsg req_msg;
            req_msg.msg_type = kungfu::MsgType::ReqLogin;
            req_msg.data = js_req;

            nlohmann::json j = req_msg;
            j["gen_time"] = yijinjing::time::now_in_nano();
            j["trigger_time"] = yijinjing::time::now_in_nano();
            j["source"] = 0;

            auto socket = io_device->connect_socket(yijinjing::data::location(yijinjing::data::mode::LIVE, c, group, name), yijinjing::nanomsg::protocol::REQUEST, kungfu::yijinjing::time_unit::MILLISECONDS_PER_SECOND * 5);
            socket->send_json(j);

            SPDLOG_TRACE("gateway login sent {}", j.dump());
            GatewayLoginRsp rsp = {};

            int rc = socket->recv();
            if (rc > 0)
            {
                NNMsg rsp_msg = nlohmann::json::parse(socket->last_message());
                from_json(rsp_msg.data, rsp);
            }

            return rsp;
        }

        inline GatewayLoginRsp add_market_feed(yijinjing::io_device_client_ptr io_device, const std::string& source, const std::string& sender)
        {
            return login(io_device, yijinjing::data::category::MD, source, source, sender);
        }

        inline GatewayLoginRsp register_trade_account(yijinjing::io_device_client_ptr io_device, const std::string& source, const std::string& account_id, const std::string& sender)
        {
            return login(io_device, yijinjing::data::category::TD, source, account_id, sender);
        }

        inline void subscribe(kungfu::yijinjing::io_device_client_ptr io_device, const std::string& source, const std::vector<journal::Instrument>& instruments, bool is_level2, const std::string& sender)
        {

            SubscribeRequest request = {};
            request.sender = sender;
            request.source = source;
            request.instruments = instruments;
            nlohmann::json j = request;

            NNMsg req_msg;
            req_msg.msg_type = MsgType::Subscribe;
            req_msg.data = j;

            j = req_msg;
            j["gen_time"] = yijinjing::time::now_in_nano();
            j["trigger_time"] = yijinjing::time::now_in_nano();
            j["source"] = 0;

            auto socket = io_device->connect_socket(yijinjing::data::location(yijinjing::data::mode::LIVE, yijinjing::data::category::MD, source, source),
                    yijinjing::nanomsg::protocol::REQUEST);
            SPDLOG_DEBUG("subscribe sending {}", j.dump());
            socket->send_json(j);
            SPDLOG_DEBUG("subscribe send {}", j.dump());
        }
    }
}

#endif //KUNGFU_WINGCHUN_UTIL_HPP
