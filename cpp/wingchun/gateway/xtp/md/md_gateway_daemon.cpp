//
// Created by qlu on 2019/2/27.
//

#include "md_gateway.h"
#include "cxxopts.hpp"
#include <string>

int main(int argc, char* argv[])
{
    cxxopts::Options options("Xtp Md Service");
    options.add_options()
            ("c, client-id", "client id", cxxopts::value<int>()->default_value("1"))
            ("i, ip", "ip", cxxopts::value<std::string>()->default_value("120.27.164.138"))
            ("p, port", "port",  cxxopts::value<int>()->default_value("6002"))
            ("k, software-key", "software key", cxxopts::value<std::string>()->default_value("b8aa7173bba3470e390d787219b2112e"))
            ("u, user-id", "user id",cxxopts::value<std::string>()->default_value("xxxxxxx"))
            ("password", "password", cxxopts::value<std::string>()->default_value("xxxxxxx"))
            ("save-path", "save path", cxxopts::value<std::string>()->default_value("/shared/kungfu/runtime"))
            ("log-level", "log level", cxxopts::value<int>()->default_value("2"));
            ;

    auto result = options.parse(argc, argv);

    int client_id = result["client-id"].as<int>();
    std::string ip = result["ip"].as<std::string>();
    int port = result["port"].as<int>();
    std::string user_id = result["user-id"].as<std::string>();
    std::string password = result["password"].as<std::string>();
    std::string save_path = result["save-path"].as<std::string>();
    int log_level = result["log-level"].as<int>();

    kungfu::xtp::MdGateway gateway(client_id, save_path, ip, port, user_id, password, log_level);
    gateway.init();
    gateway.start();
}
