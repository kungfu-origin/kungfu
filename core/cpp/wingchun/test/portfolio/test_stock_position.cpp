//
// Created by PolarAir on 2019-04-23.
//

#include "gtest/gtest.h"
#include <kungfu/wingchun/portfolio/position_manager.h>
#include <boost/filesystem.hpp>

using namespace kungfu;

#define ACCOUNT "12345"
#define CLIENT "test"
#define DB_PATH "./test_stock_pos.db"

TEST(StockTest, DB)
{
    if (boost::filesystem::exists(DB_PATH))
    {
        boost::filesystem::remove(DB_PATH);
    }
    auto* pos_manager = new PositionManager(ACCOUNT, DB_PATH);
    Position pos = {0, 0, "", "600000", InstrumentTypeStock, "SSE", ACCOUNT, CLIENT, DirectionLong,
                    1000, 500, 0, 0, 10.0, 0, 9.8, 0, 9.9, 0, 0, 0, 0, 0, 0, 200, "", ""};
    pos_manager->on_positions({pos});
    EXPECT_EQ(pos_manager->get_long_tot("600000", "SSE"), 1000);
    delete(pos_manager);

    pos_manager = new PositionManager(ACCOUNT, DB_PATH);
    EXPECT_EQ(pos_manager->get_long_yd("600000", "SSE"), 500);
}

TEST(StockTest, TradingDay)
{
    if (boost::filesystem::exists(DB_PATH))
    {
        boost::filesystem::remove(DB_PATH);
    }
    PositionManager pos_manager(ACCOUNT, DB_PATH);
    Position pos = {0, 0,"", "600000", InstrumentTypeStock, "SSE", ACCOUNT, CLIENT, DirectionLong,
                    1000, 0, 0, 0, 10.0, 0, 9.8, 0, 9.9, 0, 0, 0, 0, 0, 0, 200, "", ""};
    pos_manager.on_positions({pos});
    EXPECT_EQ(pos_manager.get_long_yd("600000", "SSE"), 0);
    pos_manager.set_current_trading_day("20190423");
    EXPECT_EQ(pos_manager.get_long_yd("600000", "SSE"), 0);
    pos_manager.switch_day("20190424");
    EXPECT_EQ(pos_manager.get_long_yd("600000", "SSE"), 1000);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
