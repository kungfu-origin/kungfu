//
// Created by qlu on 2019/3/8.
//

#ifndef KUNGFU_NANOMSG_UTIL_H
#define KUNGFU_NANOMSG_UTIL_H

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <nanomsg/reqrep.h>

#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
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

    // timeout 超时选项
    // -1: 阻塞，一直等
    // 0: 不等待，直接返回
    // >0: 毫秒数
    inline NNMsg get_rsp(const std::string &url, const NNMsg& req, int timeout = -1)
    {
        yijinjing::nanomsg::socket socket(AF_SP, NN_REQ);
        socket.connect(url);
        socket.setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(int));

        nlohmann::json j_req = req;
        std::string j_str = j_req.dump();
        SPDLOG_TRACE("send {} to {}", j_str, url);
        socket.send(j_str, 0);

        char* buffer = nullptr;
        NNMsg rsp = {};
        rsp.data["error_msg"] = "no response";
        int rc = socket.recv(&buffer, NN_MSG, 0);
        if (rc > 0)
        {
            SPDLOG_TRACE("rcv {} from {}", buffer, url);
            try
            {
                nlohmann::json j = nlohmann::json::parse(std::string(buffer));
                from_json(j, rsp);
            }
            catch (std::exception& e)
            {
                SPDLOG_ERROR("failed to parse data[{}], exception: {}", (char*)buffer, e.what());
            }
            nn_freemsg(buffer);
        }
        else
        {
            SPDLOG_ERROR("no response from {}", url);
        }
        socket.shutdown(0);
        return rsp;
    }
}

#endif //KUNGFU_NANOMSG_UTIL_H
