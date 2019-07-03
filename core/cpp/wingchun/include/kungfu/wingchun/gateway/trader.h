//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_TRADER_H
#define WINGCHUN_TRADER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            class Trader : public practice::apprentice
            {
            public:
                explicit Trader(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source, const std::string &account_id);

                virtual ~Trader() = default;

                const std::string &get_account_id() const
                { return account_id_; }

                const std::string &get_source() const
                { return source_; }

                const std::string current_trading_day() const
                {
                    //TODO
                    return "20190702";
                }

                virtual const AccountType get_account_type() const = 0;

                virtual bool insert_order(const yijinjing::event_ptr& event) = 0;

                virtual bool cancel_order(const yijinjing::event_ptr& event) = 0;

                virtual bool req_position() = 0;

                virtual bool req_account() = 0;

            protected:
                void react(const rx::observable<yijinjing::event_ptr> &events) override ;

                void on_start(const rx::observable<yijinjing::event_ptr> &events) override ;

            private:
                std::string source_;
                std::string account_id_;
                Calendar_ptr calendar_;
            };
        }
    }
}

#endif //WINGCHUN_TRADER_H
