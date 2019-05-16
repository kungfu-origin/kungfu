
project(commission)

SET(COMMISSION_SOURCE_FILES commission_manager.h commission_manager.cpp commission_storage.h commission_storage.cpp log.cpp log.h)
ADD_LIBRARY(commission SHARED ${COMMISSION_SOURCE_FILES})

if(WIN32)
    TARGET_LINK_LIBRARIES(commission SQLiteCpp sqlite3 fmt ${Boost_LIBRARIES})
endif()
if(UNIX AND NOT APPLE)
    TARGET_LINK_LIBRARIES(commission SQLiteCpp sqlite3 pthread dl fmt ${Boost_LIBRARIES})
endif()
if(APPLE)
    TARGET_LINK_LIBRARIES(commission SQLiteCpp sqlite3 pthread dl fmt ${Boost_LIBRARIES} iconv)
endif()

#if (test)
#    add_executable(test_commission test/test_commission.cpp)
#    target_link_libraries(test_commission commission gtest_main)
#    add_test(NAME test-commission COMMAND test_commission)
#endif()
