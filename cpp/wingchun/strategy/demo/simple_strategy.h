//
// Created by qlu on 2019/5/9.
//

#ifndef PROJECT_SIMPLE_STRATEGY_H
#define PROJECT_SIMPLE_STRATEGY_H

#include "strategy/include/strategy.h"

using namespace kungfu;

class SimpleStrategy: public Strategy
{
public:
    SimpleStrategy(const std::string& name);

    virtual void init();

    virtual void on_quote(const Quote& quote);
    virtual void on_order(const Order& order);
    virtual void on_trade(const Trade& trade);

private:
    uint64_t order_id_;
};

#endif //PROJECT_SIMPLE_STRATEGY_H
