
project(strategy_demo)
add_executable(strategy_demo main.cpp simple_strategy.h simple_strategy.cpp)
target_link_libraries(strategy_demo strategy)

