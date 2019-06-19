PROJECT(kungfu-extensions)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/yijinjing/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/wingchun/include)

############################################################

ADD_SUBDIRECTORY(xtp)
ADD_SUBDIRECTORY(ctp)
ADD_SUBDIRECTORY(tora)