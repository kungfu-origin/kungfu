//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_FEEDHANDLER_H
#define KUNGFU_FEEDHANDLER_H

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>

namespace kungfu
{
    class MarketDataFeedHandler
    {
    public:
        virtual void on_quote(const journal::Quote* quote) {};
        virtual void on_entrust(const journal::Entrust* entrust) {};
        virtual void on_transaction(const journal::Transaction* transaction) {}
    };

    class TraderDataFeedHandler
    {
    public:
        virtual void on_order_input(const journal::OrderInput* input) {}
        virtual void on_order_action(const journal::OrderAction* action) {}
        virtual void on_order(const journal::Order* order) {}
        virtual void on_trade(const journal::Trade* trade) {}
        virtual void on_position(const flying::Position* pos) {}
        virtual void on_position_detail(const flying::Position* pos) {}
        virtual void on_account(const flying::AccountInfo* account) {}
    };
}
#endif //KUNGFU_FEEDHANDLER_H
