#include "td_gateway.h"

namespace kungfu
{
    namespace kfext_demo
    {
        TdGateway::TdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double):
            kungfu::TdGatewayImpl("kfext_demo_td", "kfext_demo_td")
        {
            SPDLOG_INFO("Connecting kfext demo TD");
        }

        void TdGateway::init()
        {
            SPDLOG_INFO("Init kfext demo TD");
            TdGatewayImpl::init();
        }

        void TdGateway::start()
        {
            SPDLOG_INFO("Start kfext demo TD");
            GatewayImpl::start();
        }

        bool TdGateway::insert_order(const OrderInput &input)
        {
            return false;
        }

        bool TdGateway::cancel_order(const OrderAction &action)
        {
            return false;
        }

        bool TdGateway::req_position()
        {
            return false;
        }

        bool TdGateway::req_account()
        {
            return false;
        }
    }
}
