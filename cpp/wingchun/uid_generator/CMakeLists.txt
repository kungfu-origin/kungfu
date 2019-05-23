
cmake_minimum_required(VERSION  3.7) # first version with add_compile_options()
project(uid_generator)

set(SOURCE_FILES uid_generator.h uid_generator.cpp)
add_library(uid_generator SHARED ${SOURCE_FILES})
target_link_libraries(uid_generator SQLiteCpp)

#if (test)
#    add_executable(test_uid_generator test/test_uid_generator.cpp)
#    target_link_libraries(test_uid_generator gtest gtest_main uid_generator)
#    add_test(NAME test-uid-generator COMMAND test_uid_generator --gtest_output=xml:testresult.xml)
#endif()

