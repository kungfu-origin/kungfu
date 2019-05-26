//
// Created by qlu on 2019/1/23.
//

#ifndef PROJECT_MSG_H
#define PROJECT_MSG_H

#include <cstdint>
#include <nlohmann/json.hpp>

namespace kungfu
{
    enum class MsgType: int16_t
    {
        Error = 0,

        Quote = 101,
        Entrust = 102,
        Transaction = 103,

        OrderInput = 201,
        OrderAction = 202,
        Order = 203,
        Trade = 204,
        Position = 205,
        AccountInfo = 206,
        Portfolio = 207,
        AccountInfoByMin = 208,
        PortfolioByMin = 209,
        PositionDetail = 210,
        SubPortfolioInfo = 211,


        ReqLogin = 301,
        Subscribe = 302,
        ReqOrderInput = 303,
        ReqOrderAction = 304,

        RspLogin = 351,
        RspSubscribe = 352,
        RspOrderInput = 353,
        RspOrderAction = 354,

        GatewayState = 401,

        AlgoOrderInput = 501,
        AlgoOrderStatus = 502,
        AlgoOrderAction = 503,

        SwitchDay = 601,
        RspTradingDay = 602,

        ReloadFutureInstrument = 701
    };

    struct NNMsg
    {
        kungfu::MsgType msg_type;
        nlohmann::json data;
    };

    inline void to_json(nlohmann::json& j, const NNMsg& msg)
    {
        j["msg_type"] = msg.msg_type;
        j["data"] = msg.data;
    }

    inline void from_json(const nlohmann::json& j, NNMsg& msg)
    {
        msg.msg_type = j["msg_type"];
        msg.data = j["data"];
    }

    inline std::string to_string(const NNMsg& msg)
    {
        nlohmann::json j = msg;
        return j.dump();
    }
}

#endif //PROJECT_MSG_H
