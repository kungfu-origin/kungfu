
PROJECT(nn_publisher)
SET(NN_PUBLISHER_SOURCE_FILES nn_publisher.cpp nn_publisher.h)
ADD_LIBRARY(nn_publisher SHARED ${NN_PUBLISHER_SOURCE_FILES})
TARGET_LINK_LIBRARIES(nn_publisher nanomsg fmt)

ADD_EXECUTABLE(test_receive  test/test_receive.cpp)
TARGET_LINK_LIBRARIES(test_receive nanomsg)
