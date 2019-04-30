
include_directories(../yijinjing/journal)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(task_scheduler)
add_subdirectory(event_loop)
add_subdirectory(uid_generator)
add_subdirectory(util)
add_subdirectory(calendar)
add_subdirectory(oms)
add_subdirectory(commission)
add_subdirectory(portfolio)
add_subdirectory(strategy)
add_subdirectory(gateway)
add_subdirectory(nn_publisher)
add_subdirectory(storage)


