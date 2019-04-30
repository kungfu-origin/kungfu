//
// Created by qlu on 2019/3/8.
//

#ifndef KUNGFU_UTIL_H
#define KUNGFU_UTIL_H

#include "state.h"
#include "util/include/nanomsg_util.h"
#include "util/include/env.h"
#include "nlohmann/json.hpp"
#include "fmt/format.h"
#include "config.h"
#include "serialize.h"
#include "msg.h"
#include "macro.h"

#include <utility>

#include "Timer.h"

namespace kungfu
{
    namespace gateway
    {
        struct GatewayLoginRsp
        {
            AccountType type; // TdGateway 需要返回账号类型
            GatewayState state;
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
        }

        inline GatewayLoginRsp login(const std::string& gateway, const std::string& sender)
        {
            LoginRequest request;
            request.sender = sender;
            request.recipient = gateway;
            nlohmann::json js_req = request;

            NNMsg req_msg;
            req_msg.msg_type = kungfu::MsgType::ReqLogin;
            req_msg.data = js_req;
            std::string url = GATEWAY_REP_URL(gateway);
            NNMsg rsp_msg = get_rsp(url, req_msg, kungfu::yijinjing::MILLISECONDS_PER_SECOND * 5);

            GatewayLoginRsp rsp = {};
            from_json(rsp_msg.data, rsp);

            return rsp;
        }

        inline GatewayLoginRsp add_market_feed(const std::string& source, const std::string& sender)
        {
            std::string gateway = fmt::format(MD_GATEWAY_NAME_FORMAT, source);
            return login(gateway, sender);
        }

        inline GatewayLoginRsp register_trade_account(const std::string& source, const std::string& account_id, const std::string& sender)
        {
            std::string gateway = fmt::format(TD_GATEWAY_NAME_FORMAT, source, account_id);
            return login(gateway, sender);
        }

        inline void subscribe(const std::string& source, const std::vector<Instrument>& instruments, bool is_level2, const std::string& sender)
        {
            std::string gateway = fmt::format(MD_GATEWAY_NAME_FORMAT, source);
            std::string url = GATEWAY_REP_URL(gateway);

            SubscribeRequest request = {};
            request.sender = sender;
            request.recipient = gateway;
            request.instruments = instruments;
            nlohmann::json j = request;

            NNMsg req_msg;
            req_msg.msg_type = MsgType::Subscribe;
            req_msg.data = j;

            get_rsp(url, req_msg);
        }
    }
}

#endif //KUNGFU_UTIL_H
