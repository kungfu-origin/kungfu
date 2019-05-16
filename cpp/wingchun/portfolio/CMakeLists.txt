
cmake_minimum_required(VERSION  3.7)
project(portfolio)

SET(PORTFOLIO_SOURCE_FILES include/pnl_def.h include/position_manager.h src/position_manager.cpp src/position_storage.h src/position_storage.cpp src/position_manager.hpp include/account_manager.h src/account_manager.hpp src/account_storage.h src/account_storage.cpp src/account_manager.cpp include/portfolio_manager.h src/portfolio_storage.h src/portfolio_storage.cpp src/portfolio_manager.hpp src/portfolio_manager.cpp src/storage_common.h src/position_manager.hxx src/account_manager.hxx src/portfolio_manager.hxx src/log.cpp)
ADD_LIBRARY(portfolio SHARED ${PORTFOLIO_SOURCE_FILES})
TARGET_LINK_LIBRARIES(portfolio calendar instrument journal commission fmt)

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
