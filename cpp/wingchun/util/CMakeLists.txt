
cmake_minimum_required(VERSION  3.7)
project(util)

ADD_LIBRARY(journal_printer SHARED journal_printer/journal_printer.cpp journal_printer/journal_printer.h)
ADD_LIBRARY(instrument SHARED instrument/instrument.h instrument/instrument.cpp)
TARGET_LINK_LIBRARIES(instrument SQLiteCpp fmt)
target_link_libraries(journal_printer journal)
#add_subdirectory(journal_printer/pybind)

#if (test)
#    add_executable(test_util test/test.cpp)
#    add_executable(test_json test/test_json.cpp)
#    add_executable(test_log test/test_log.cpp)
#    target_link_libraries(test_util instrument)
#    target_link_libraries(test_log journal)
#    add_test(NAME test-util COMMAND test_util)
#endif ()