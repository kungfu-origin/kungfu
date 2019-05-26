#include "md_gateway.h"

namespace kungfu
{
    namespace kfext_demo
    {
        MdGateway::MdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double): kungfu::MdGatewayImpl("kfext_demo_md")
        {
            SPDLOG_INFO("Connecting kfext demo MD");
        }

        void MdGateway::init()
        {
            SPDLOG_INFO("Init kfext demo MD");
            MdGatewayImpl::init();
        }

        void MdGateway::start()
        {
            SPDLOG_INFO("Start kfext demo MD");
            GatewayImpl::start();
        }

        bool MdGateway::subscribe(const std::vector<Instrument>& instruments, bool is_level2)
        {
            return false;
        }

        bool MdGateway::unsubscribe(const std::vector<Instrument>& instruments)
        {
            return false;
        }
    }
}
