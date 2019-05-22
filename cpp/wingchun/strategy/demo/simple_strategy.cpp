//
// Created by qlu on 2019/5/9.
//

#include "simple_strategy.h"
#include "constants.h"

SimpleStrategy::SimpleStrategy(const std::string &name): Strategy(name), order_id_(0) {}

void SimpleStrategy::init()
{
    add_md(SOURCE_XTP);
    add_account(SOURCE_XTP, "15040900", 10000.0);
    subscribe(SOURCE_XTP, {"600000"}, EXCHANGE_SSE);
}

void SimpleStrategy::on_quote(const journal::Quote &quote)
{
    if (order_id_ == 0)
    {
        order_id_ = insert_limit_order("600000", EXCHANGE_SSE, "15040900", quote.ask_price[0], 500, SideBuy, OffsetOpen);
        register_nanotime_callback(get_nano() + 5 * 1e9, std::bind(&Strategy::stop, this));
    }
}

void SimpleStrategy::on_order(const journal::Order &order)
{
}

void SimpleStrategy::on_trade(const journal::Trade &trade)
{
}


