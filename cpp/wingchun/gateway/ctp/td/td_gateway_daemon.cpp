//
// Created by qlu on 2019/3/22.
//


#include "td_gateway.h"
#include "cxxopts.hpp"

int main(int argc, char* argv[])
{
    cxxopts::Options options("Ctp Td Service");
    options
            .positional_help("[optional args]")
            .show_positional_help();
    options.add_options()
            ("account-id", "account id", cxxopts::value<std::string>()->default_value("xxxxxx"))
            ("password", "password", cxxopts::value<std::string>()->default_value("xxxxxxxx"))
            ("broker-id", "broker id", cxxopts::value<std::string>()->default_value("9999"))
            ("front-uri", "front uri", cxxopts::value<std::string>()->default_value("tcp://180.168.146.187:10000"))
            ("log-level", "log level", cxxopts::value<int>()->default_value("0"))
            ;

    auto result = options.parse(argc, argv);
    std::string account_id = result["account-id"].as<std::string>();
    std::string password = result["password"].as<std::string>();
    std::string broker_id = result["broker-id"].as<std::string>();
    std::string front_uri = result["front-uri"].as<std::string>();
    int log_level = result["log-level"].as<int>();

    kungfu::ctp::TdGateway gateway(front_uri, broker_id, account_id, password, log_level);
    gateway.init();
    gateway.start();
}


