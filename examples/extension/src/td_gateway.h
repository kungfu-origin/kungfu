#ifndef TD_GATEWAY_KFEXT_DEMO_H
#define TD_GATEWAY_KFEXT_DEMO_H

#include <string>
#include <map>
#include <kungfu/wingchun/gateway/gateway_impl.h>

namespace kungfu
{
    namespace kfext_demo
    {
        class TdGateway: public kungfu::TdGatewayImpl
        {
        public:
            TdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double);
            virtual void init();
            virtual void start();
            virtual const std::string& get_account_id() const { return user_; }
            virtual const AccountType get_account_type() const { return AccountTypeStock; }

            virtual bool insert_order(const OrderInput& input);
            virtual bool cancel_order(const OrderAction& action);
            virtual bool req_position();
            virtual bool req_account();

        private:
            std::string user_;
        };
    }
}
#endif //TD_GATEWAY_KFEXT_DEMO_H
