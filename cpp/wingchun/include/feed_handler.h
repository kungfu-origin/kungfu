//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_FEEDHANDLER_H
#define KUNGFU_FEEDHANDLER_H

#include "md_struct.h"
#include "oms_struct.h"

namespace kungfu
{
    class MarketDataFeedHandler
    {
    public:
        virtual void on_quote(const Quote* quote) {};
        virtual void on_entrust(const Entrust* entrust) {};
        virtual void on_transaction(const Transaction* transaction) {}
    };

    class TraderDataFeedHandler
    {
    public:
        virtual void on_order_input(const OrderInput* input) {}
        virtual void on_order_action(const OrderAction* action) {}
        virtual void on_order(const Order* order) {}
        virtual void on_trade(const Trade* trade) {}
        virtual void on_position(const Position* pos) {}
        virtual void on_position_detail(const Position* pos) {}
        virtual void on_account(const AccountInfo* account) {}
    };
}
#endif //KUNGFU_FEEDHANDLER_H
