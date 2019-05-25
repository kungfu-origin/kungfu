//
// Created by qlu on 2019/1/22.
//

#ifndef PROJECT_STATE_H
#define PROJECT_STATE_H

#include <string>
#include <iostream>
#include <cstring>
#include <nlohmann/json.hpp>

namespace kungfu
{
    enum class GatewayState: int
    {
        Unknown = 0,
        Idle = 1,
        DisConnected = 2,
        Connected = 3,
        LoggedIn = 4,
        LoggedInFailed = 5,
        SettlementConfirmed = 6,
        SettlementConfirmFailed = 7,
        AccountInfoConfirmed = 8,
        AccountInfoConfirmFailed = 9,
        PositionInfoConfirmed = 10,
        PositionInfoConfirmFailed = 11,
        InstrumentInfoConfirmed = 12,
        InstrumentInfoConfirmFailed = 13,
        PositionDetailConfirmed = 14,
        PositionDetailConfirmFailed = 15,
        Ready = 100
    };

    struct GatewayStateRecord
    {
        char name[50];
        uint64_t update_time;
        GatewayState state;
        char message[200];
    };

    inline void to_json(nlohmann::json& j, const GatewayStateRecord& record)
    {
        j["name"] = std::string(record.name);
        j["update_time"] = record.update_time;
        j["state"] = record.state;
        j["message"] = std::string(record.message);
    }
}
#endif //PROJECT_STATE_H
