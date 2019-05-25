
PROJECT(geteway_impl)
SET(GATEWAY_IMPL_SOURCE_FILES gateway_impl.h gateway_impl.cpp subscription_storage.hpp)
ADD_LIBRARY(gateway_impl SHARED ${GATEWAY_IMPL_SOURCE_FILES})
TARGET_LINK_LIBRARIES(gateway_impl PRIVATE event_loop uid_generator portfolio storage nn_publisher oms)
