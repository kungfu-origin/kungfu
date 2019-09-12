//
// Created by qlu on 2019-08-20.
//

#ifndef KUNGFU_XTP_EXT_COMMON_H
#define KUNGFU_XTP_EXT_COMMON_H

#include <nlohmann/json.hpp>

namespace kungfu
{
    namespace wingchun
    {
        namespace xtp
        {
            struct Configuration
            {
                int client_id;
                std::string user_id;
                std::string password;
                std::string software_key;
                std::string md_ip;
                int md_port;
                std::string td_ip;
                int td_port;
                std::string protocol;
                int buffer_size;
            };

            inline void from_json(const nlohmann::json& j, Configuration& c)
            {
                j.at("client_id").get_to(c.client_id);
                j.at("user_id").get_to(c.user_id);
                j.at("password").get_to(c.password);
                j.at("software_key").get_to(c.software_key);
                j.at("md_ip").get_to(c.md_ip);
                j.at("md_port").get_to(c.md_port);
                j.at("td_ip").get_to(c.td_ip);
                j.at("td_port").get_to(c.td_port);
                c.protocol =  j.value("protocol", "tcp");
                if (c.protocol != "udp")
                {
                    c.protocol = "tcp";
                }
                c.buffer_size = j.value("buffer_size", 64);
            }
        }
    }
}
#endif //KUNGFU_XTP_EXT_COMMON_H
