PROJECT(kungfu-extensions)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/yijinjing/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/wingchun/include)

############################################################

ADD_SUBDIRECTORY(passive)
ADD_SUBDIRECTORY(xtp)
ADD_SUBDIRECTORY(bar_handle)

IF (NOT APPLE)
    ADD_SUBDIRECTORY(ctp)
ENDIF ()