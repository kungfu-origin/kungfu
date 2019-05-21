
project(strategy)

SET(UTIL_SOURCE_FILES src/strategy_util.cpp src/strategy_util.h)
ADD_LIBRARY(strategy_util SHARED ${UTIL_SOURCE_FILES})
TARGET_LINK_LIBRARIES(strategy_util journal uid_generator nanomsg nn_publisher fmt oms portfolio calendar storage ${Boost_LIBRARIES})

SET(STRATEGY_FILES include/strategy.h src/strategy.cpp)
ADD_LIBRARY(strategy SHARED ${STRATEGY_FILES})
TARGET_LINK_LIBRARIES(strategy strategy_util event_loop ${Boost_LIBRARIES})

add_subdirectory(demo)

