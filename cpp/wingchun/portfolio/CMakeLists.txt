
cmake_minimum_required(VERSION  3.7)
project(portfolio)

SET(PORTFOLIO_SOURCE_FILES include/pnl_def.h include/position_manager.h include/account_manager.h include/portfolio_manager.h src/position_manager_impl.h src/position_manager_impl.cpp src/account_manager_impl.h src/account_manager_impl.cpp src/log.cpp src/portfolio_manager_impl.h src/portfolio_manager_impl.cpp src/storage.cpp src/storage.h)
ADD_LIBRARY(portfolio SHARED ${PORTFOLIO_SOURCE_FILES})
TARGET_LINK_LIBRARIES(portfolio SQLiteCpp calendar instrument journal commission fmt)

#if (test)
#    ADD_EXECUTABLE(test_stock_position test/test_stock_position.cpp)
#    TARGET_LINK_LIBRARIES(test_stock_position portfolio gtest gtest_main)
#    ADD_TEST(NAME test-stock-position COMMAND test_stock_position)

#    ADD_EXECUTABLE(test_stock_account test/test_stock_account.cpp)
#    TARGET_LINK_LIBRARIES(test_stock_account portfolio gtest gtest_main)
#    ADD_TEST(NAME test-stock-account COMMAND test_stock_account)

#    ADD_EXECUTABLE(test_stock_portfolio test/test_stock_portfolio.cpp)
#    TARGET_LINK_LIBRARIES(test_stock_portfolio portfolio gtest gtest_main)
#    ADD_TEST(NAME test-stock-portfolio COMMAND test_stock_portfolio)
#endif()
