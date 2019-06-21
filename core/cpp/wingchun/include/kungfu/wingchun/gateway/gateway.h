//
// Created by qlu on 2019/2/14.
//

#ifndef PROJECT_SERVICE_H
#define PROJECT_SERVICE_H

#include <string>
#include <vector>

#include <kungfu/yijinjing/io.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/state.h>

namespace kungfu
{
    namespace wingchun
    {
        class Gateway
        {
        public:
            virtual const std::string &get_name() const = 0;

            virtual const std::string &get_source() const = 0;

            virtual GatewayState get_state() const = 0;

            virtual void set_state(const enum GatewayState &state, const std::string &message = "") = 0;

            virtual void init() = 0;

            virtual void on_login(const std::string &source, const std::string &name, const std::string &client_id) = 0;

            virtual void on_started() = 0;
        };

        class MdGateway : virtual public Gateway
        {
        public:
            virtual bool subscribe_market_data(const std::vector<msg::data::Instrument> &instruments, bool is_level2) = 0;

            virtual bool unsubscribe_market_data(const std::vector<msg::data::Instrument> &instruments) = 0;

            virtual void on_login(const std::string &source, const std::string &name, const std::string &client_id) = 0;

            virtual void on_started() = 0;
        };

        class TdGateway : virtual public Gateway
        {
        public:
            virtual const std::string &get_account_id() const = 0;

            virtual const AccountType get_account_type() const = 0;

            virtual bool insert_order(const msg::data::OrderInput &input) = 0;

            virtual bool cancel_order(const msg::data::OrderAction &action) = 0;

            virtual bool req_position_detail() = 0;

            virtual bool req_position() = 0;

            virtual bool req_account() = 0;

            virtual void on_login(const std::string &source, const std::string &name, const std::string &client_id) = 0;

            virtual void on_started() = 0;

        };
    }
}

#endif //PROJECT_SERVICE_H
