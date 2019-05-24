
PROJECT(oms)
SET(SOURCE_FILES include/def.h impl/order_manager_impl.cpp impl/simple_order_impl.cpp impl/algo_order_impl.cpp)
ADD_LIBRARY(oms SHARED ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(oms PRIVATE fmt)

