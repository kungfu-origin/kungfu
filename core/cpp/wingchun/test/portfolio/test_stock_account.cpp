//
// Created by PolarAir on 2019-04-23.
//

#include "gtest/gtest.h"
#include <kungfu/wingchun/portfolio/account_manager.h>
#include <boost/filesystem.hpp>

using namespace kungfu;

#define ACCOUNT "12345"
#define CLIENT "test"
#define DB_PATH "./test_stock_acc.db"

uint64_t get_order_id()
{
    static uint64_t id = 0;
    return id++;
}

TEST(StockTest, Buy)
{
    if (boost::filesystem::exists(DB_PATH))
    {
        boost::filesystem::remove(DB_PATH);
    }
    AccountManager acc_manager(ACCOUNT, AccountTypeStock, DB_PATH);
    AccountInfo acc = {0, 0, "", ACCOUNT, AccountTypeStock, "", "xtp", 100000, 100000, 100000, 0, 0, 0, 0, 100000, 0,
                       0, 0, 0, 0, 0, 0, 0, 0};
    acc_manager.on_account(acc);
    EXPECT_EQ(acc.avail, 100000);

    auto order_id = get_order_id();
    OrderInput input = {order_id, "600000", "SSE", ACCOUNT, CLIENT, InstrumentTypeStock, 10.0, 10.0, 100, SideBuy,
                        OffsetOpen, PriceTypeLimit, VolumeConditionAny, TimeConditionIOC, 0};
    acc_manager.insert_order(&input);
    EXPECT_EQ(acc_manager.get_avail(), 100000 - 1000 - 5);
    EXPECT_EQ(acc_manager.get_frozen_cash(), 1005);

    Trade trade = {0, 0, order_id, 0, 0, "600000", "SSE", ACCOUNT, CLIENT, InstrumentTypeStock, SideBuy, OffsetOpen,
                   10.0, 100, 0, 0};
    auto tax = acc_manager.calc_tax(&trade);
    auto commission = acc_manager.calc_commission(&trade);
    trade.tax = tax;
    trade.commission = commission;
    acc_manager.on_trade(&trade);
    Order order = {0, order_id, 0, 0, "", "600000", "SSE", ACCOUNT, CLIENT, InstrumentTypeStock, 10.0, 10.0, 100, 100,
                   0, tax, commission, OrderStatusFilled, 0, "", 0, SideBuy, OffsetOpen, PriceTypeLimit,
                   VolumeConditionAny, TimeConditionIOC};
    acc_manager.on_order(&order);
    EXPECT_EQ(acc_manager.get_long_tot("600000", "SSE"), 100);
    EXPECT_EQ(acc_manager.get_long_yd("600000", "SSE"), 0);
    EXPECT_EQ(acc_manager.get_avail(), 100000 - 1000 - tax - commission);
    EXPECT_EQ(acc_manager.get_frozen_cash(), 0);
    EXPECT_EQ(acc_manager.get_market_value(), 1000);
    EXPECT_EQ(acc_manager.get_intraday_fee(), tax + commission);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
