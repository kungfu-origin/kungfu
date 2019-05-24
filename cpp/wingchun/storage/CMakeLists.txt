CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(storage)

SET(STORAGE_SOURCE_FILES order_storage.cpp order_storage.h trade_storage.cpp trade_storage.h snapshot_storage.cpp snapshot_storage.h account_list_storage.cpp account_list_storage.h source_list_storage.cpp source_list_storage.h log.h log.cpp)
ADD_LIBRARY(storage SHARED ${STORAGE_SOURCE_FILES})
TARGET_LINK_LIBRARIES(storage PRIVATE SQLiteCpp fmt)

#if (test)
#    add_executable(test_order_storage  test/test_order_storage.cpp)
#    target_link_libraries(test_order_storage storage)
#    add_test(NAME test-order-storage COMMAND test_order_storage)
#endif()
