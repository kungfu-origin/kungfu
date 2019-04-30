
PROJECT(task_scheduler)
SET(TASK_SCHEDULER_SOURCE_FILES task_scheduler.h task_scheduler.cpp)
ADD_LIBRARY(task_scheduler SHARED ${TASK_SCHEDULER_SOURCE_FILES})
#if (test)
#    ADD_EXECUTABLE(test_task_scheduler test/test_task_scheduler.cpp)
#    TARGET_LINK_LIBRARIES(test_task_scheduler task_scheduler gtest_main)
#    ADD_TEST(NAME test-task-scheduler COMMAND test_task_scheduler --gtest_output=xml:testresult.xml)
#endif()