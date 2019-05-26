#ifndef MD_GATEWAY_KFEXT_DEMO_H
#define MD_GATEWAY_KFEXT_DEMO_H

#include <string>
#include <map>
#include <kungfu/wingchun/gateway/gateway_impl.h>

namespace kungfu
{
    namespace kfext_demo
    {
        class MdGateway: public kungfu::MdGatewayImpl
        {
        public:
            MdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double);
            virtual void init();
            virtual void start();
            virtual bool subscribe(const std::vector<Instrument>& instruments, bool is_level2 = false);
            virtual bool unsubscribe(const std::vector<Instrument>& instruments);
        };
    }
}

#endif //MD_GATEWAY_KFEXT_DEMO_H
