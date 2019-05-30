//
// Created by PolarAir on 2019-02-28.
//

#include "gtest/gtest.h"
#include <kungfu/wingchun/calendar/calendar.h>

using namespace kungfu;

TEST(CalendarTest, NextTradingDay)
{
    Calendar calendar;
    EXPECT_EQ(calendar.get_next_trading_day("20190301", 1), "20190304");
    EXPECT_EQ(calendar.get_next_trading_day("19900105", 1), "19900108");
}

TEST(CalendarTest, SSE)
{
    Calendar calendar;
    long nano=yijinjing::time::parseTime("20190410-10:00:00", "%Y%m%d-%H:%M:%S");
    const char* exchange_1 = "SSE";
    EXPECT_EQ(calendar.is_open(nano, exchange_1),true);
    EXPECT_EQ(yijinjing::time::parseNano(calendar.get_open_time("20190410", exchange_1), "%Y%m%d-%H:%M:%S"),"20190410-09:30:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.get_close_time("20190410", exchange_1), "%Y%m%d-%H:%M:%S"),"20190410-15:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.next_open(nano, exchange_1), "%Y%m%d-%H:%M:%S"),"20190411-09:30:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.next_close(nano, exchange_1), "%Y%m%d-%H:%M:%S"),"20190410-15:00:00");
}

TEST(CalendarTest, SHFE)
{
    Calendar calendar;
    long nano=yijinjing::time::parseTime("20190410-10:00:00", "%Y%m%d-%H:%M:%S");
    const char* exchange_1 = "SHFE";
    EXPECT_EQ(calendar.is_open(nano, exchange_1),true);
    EXPECT_EQ(yijinjing::time::parseNano(calendar.get_open_time("20190410", exchange_1), "%Y%m%d-%H:%M:%S"),"20190409-21:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.get_close_time("20190410", exchange_1), "%Y%m%d-%H:%M:%S"),"20190410-15:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.next_open(nano, exchange_1, EVENING), "%Y%m%d-%H:%M:%S"),"20190410-21:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(calendar.next_close(nano, exchange_1), "%Y%m%d-%H:%M:%S"),"20190410-15:00:00");
}

TEST(CalendarTest, TradingSession)
{
    Calendar calendar;
    std::vector<TradingSession>result = calendar.get_trading_sessions(yijinjing::time::parseTime("20190410-10:00:00", "%Y%m%d-%H:%M:%S"), yijinjing::time::parseTime("20190410-23:00:00", "%Y%m%d-%H:%M:%S"), "SHFE");
    EXPECT_EQ(result.size(), 3);

    EXPECT_EQ(result[0].trading_day, "20190410");
    EXPECT_EQ(yijinjing::time::parseNano(result[0].start_nano, "%Y%m%d-%H:%M:%S"), "20190410-10:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(result[0].end_nano, "%Y%m%d-%H:%M:%S"), "20190410-11:30:00");

    EXPECT_EQ(result[1].trading_day, "20190410");
    EXPECT_EQ(yijinjing::time::parseNano(result[1].start_nano, "%Y%m%d-%H:%M:%S"), "20190410-13:30:00");
    EXPECT_EQ(yijinjing::time::parseNano(result[1].end_nano, "%Y%m%d-%H:%M:%S"), "20190410-15:00:00");

    EXPECT_EQ(result[2].trading_day, "20190411");
    EXPECT_EQ(yijinjing::time::parseNano(result[2].start_nano, "%Y%m%d-%H:%M:%S"), "20190410-21:00:00");
    EXPECT_EQ(yijinjing::time::parseNano(result[2].end_nano, "%Y%m%d-%H:%M:%S"), "20190410-23:00:00");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
