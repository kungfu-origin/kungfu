
cmake_minimum_required(VERSION 3.7)
project(event_loop)

SET(EVENT_LOOP_SOURCE_FILES event_loop.h event_loop.cpp)

add_library(event_loop SHARED ${EVENT_LOOP_SOURCE_FILES})
target_link_libraries(event_loop task_scheduler nanomsg journal fmt)

if (test)
    add_executable(test_loop  test/test.cpp)
    target_link_libraries(test_loop event_loop)
    add_test(NAME test-event-loop COMMAND test_loop)
endif()
