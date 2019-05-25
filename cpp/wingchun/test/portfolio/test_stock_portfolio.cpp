//
// Created by PolarAir on 2019-04-23.
//

#include "gtest/gtest.h"
#include <kungfu/wingchun/portfolio/portfolio_manager.h>
#include <boost/filesystem.hpp>

using namespace kungfu;

#define ACCOUNT "12345"
#define ACCOUNT2 "54321"
#define CLIENT "test"
#define DB_PATH "./test_stock_pnl.db"

TEST(StockTest, TradingDay)
{
    if (boost::filesystem::exists(DB_PATH))
    {
        boost::filesystem::remove(DB_PATH);
    }
    PortfolioManager pnl_manager(DB_PATH);
    AccountInfo acc = {0, 0, "", ACCOUNT, AccountTypeStock, "", "xtp", 0, 0, 0, 0, 0, 0, 0, 100000, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pnl_manager.on_account(acc);
    Position pos = {0, 0, "20190426", "600000", InstrumentTypeStock, "SSE", ACCOUNT, CLIENT, DirectionLong,
                    1000, 0, 0, 0, 10.0, 0, 9.8, 0, 9.9, 0, 0, 0, 0, 0, 0, 200, "", ""};
    pnl_manager.on_positions({pos});
    EXPECT_EQ(pnl_manager.get_long_yd("600000", "SSE"), 0);
    pnl_manager.set_current_trading_day("20190423");
    EXPECT_EQ(pnl_manager.get_long_yd("600000", "SSE"), 0);
    pnl_manager.switch_day("20190424");
    EXPECT_EQ(pnl_manager.get_long_yd("600000", "SSE"), 1000);
}

TEST(StockTest, MultiAccount)
{
    if (boost::filesystem::exists(DB_PATH))
    {
        boost::filesystem::remove(DB_PATH);
    }
    PortfolioManager pnl_manager(DB_PATH);
    AccountInfo acc = {0, 0, "", ACCOUNT, AccountTypeStock, "", "xtp", 0, 0, 0, 0, 0, 0, 0, 100000, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    AccountInfo acc2 = {0, 0, "", ACCOUNT2, AccountTypeStock, "", "xtp", 0, 0, 0, 0, 0, 0, 0, 100000, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pnl_manager.on_account(acc);
    pnl_manager.on_account(acc2);
    Position pos = {0, 0,"20190426" ,"600000", InstrumentTypeStock, "SSE", ACCOUNT, CLIENT, DirectionLong,
                    1000, 0, 0, 0, 10.0, 0, 9.8, 0, 9.9, 0, 0, 0, 0, 0, 0, 200, "", ""};
    Position pos2 = {0, 0, "20190426", "600000", InstrumentTypeStock, "SSE", ACCOUNT2, CLIENT, DirectionLong,
                    1000, 0, 0, 0, 10.0, 0, 9.8, 0, 9.9, 0, 0, 0, 0, 0, 0, 200, "", ""};
    pnl_manager.on_positions({pos, pos2});
    EXPECT_EQ(pnl_manager.get_long_tot("600000", "SSE"), 2000);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
