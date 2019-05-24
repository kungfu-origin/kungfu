CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(event_loop)

SET(EVENT_LOOP_SOURCE_FILES event_loop.h event_loop.cpp)

ADD_LIBRARY(event_loop SHARED ${EVENT_LOOP_SOURCE_FILES})
TARGET_LINK_LIBRARIES(event_loop PRIVATE task_scheduler nanomsg journal fmt)

if (test)
    add_executable(test_loop  test/test.cpp)
    target_link_libraries(test_loop PRIVATE event_loop nanomsg journal fmt task_scheduler)
    add_test(NAME test-event-loop COMMAND test_loop)
endif()
