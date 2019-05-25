//
// Created by qlu on 2019/2/14.
//

#ifndef PROJECT_SERVICE_H
#define PROJECT_SERVICE_H

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <string>
#include <vector>
#include <kungfu/wingchun/gateway/state.h>

namespace kungfu
{
    class Gateway
    {
    public:
        virtual const std::string& get_name() const = 0;
        virtual const std::string& get_source() const = 0;

        virtual GatewayState get_state() const = 0;
        virtual void set_state(const GatewayState& state, const std::string& message = "") = 0;

        virtual void init() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void on_login(const std::string& recipient, const std::string& client_id) = 0;
        virtual void on_started() = 0;
    };

    class MdGateway: virtual public Gateway
    {
    public:
        virtual bool subscribe(const std::vector<journal::Instrument>& instruments, bool is_level2) = 0;
        virtual bool unsubscribe(const std::vector<journal::Instrument>& instruments) = 0;

        virtual void on_login(const std::string& recipient, const std::string& client_id) = 0;
        virtual void on_started() = 0;
    };

    class TdGateway: virtual public Gateway
    {
    public:
        virtual const std::string& get_account_id() const  = 0;
        virtual const AccountType get_account_type() const = 0;

        virtual bool insert_order(const journal::OrderInput& input) = 0;
        virtual bool cancel_order(const journal::OrderAction& action) = 0;

        virtual bool req_position_detail() = 0;
        virtual bool req_position() = 0;
        virtual bool req_account() = 0;

        virtual void on_login(const std::string& recipient, const std::string& client_id) = 0;
        virtual void on_started() = 0;

    };
}

#endif //PROJECT_SERVICE_H
