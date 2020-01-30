//
// Created by qlu on 2019-08-20.
//

#ifndef KUNGFU_XTP_EXT_COMMON_H
#define KUNGFU_XTP_EXT_COMMON_H

#include <nlohmann/json.hpp>

namespace kungfu::wingchun::xtp
{
    struct TDConfiguration
    {
        int client_id;
        std::string user_id;
        std::string password;
        std::string software_key;
        std::string td_ip;
        int td_port;
    };

    inline void from_json(const nlohmann::json &j, TDConfiguration &c)
    {
        j.at("client_id").get_to(c.client_id);
        j.at("user_id").get_to(c.user_id);
        j.at("password").get_to(c.password);
        j.at("software_key").get_to(c.software_key);
        j.at("td_ip").get_to(c.td_ip);
        j.at("td_port").get_to(c.td_port);
    }

    struct MDConfiguration
    {
        int client_id;
        std::string user_id;
        std::string password;
        std::string md_ip;
        int md_port;
        std::string protocol;
        int buffer_size;
    };

    inline void from_json(const nlohmann::json &j, MDConfiguration &c)
    {
        j.at("client_id").get_to(c.client_id);
        j.at("user_id").get_to(c.user_id);
        j.at("password").get_to(c.password);
        j.at("md_ip").get_to(c.md_ip);
        j.at("md_port").get_to(c.md_port);
        c.protocol = j.value("protocol", "tcp");
        if (c.protocol != "udp")
        {
            c.protocol = "tcp";
        }
        c.buffer_size = j.value("buffer_size", 64);
    }
}
#endif //KUNGFU_XTP_EXT_COMMON_H
