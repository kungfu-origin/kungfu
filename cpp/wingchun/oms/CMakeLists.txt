
PROJECT(oms)
set(SOURCE_FILES include/def.h impl/order_manager_impl.cpp impl/simple_order_impl.cpp impl/algo_order_impl.cpp)
add_library(oms SHARED ${SOURCE_FILES})
target_link_libraries(oms fmt)

