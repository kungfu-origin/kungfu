#ifndef KUNGFU_SIM_EXT_COMMON_H
#define KUNGFU_SIM_EXT_COMMON_H

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            struct Configuration
            {
                std::string account_id;
                std::string password;
                std::string md_uri;
                std::string ord_uri;
                std::string rpt_uri;
                std::string qry_uri;
                std::string hd_serial;
                bool is_level2;
            };

            inline void from_json(const nlohmann::json &j, Configuration &c)
            {
                j.at("account_id").get_to(c.account_id);
                j.at("password").get_to(c.password);
                j.at("md_uri").get_to(c.md_uri);
                j.at("ord_uri").get_to(c.ord_uri);
                j.at("rpt_uri").get_to(c.rpt_uri);
                j.at("qry_uri").get_to(c.qry_uri);
                j.at("hd_serial").get_to(c.hd_serial);
                j.at("is_level2").get_to(c.is_level2);
            }
        }
    }
}

#endif //KUNGFU_SIM_EXT_COMMON_H