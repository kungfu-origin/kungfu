PROJECT(node_yjj)

if(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif()

############################################################

INCLUDE_DIRECTORIES(${CMAKE_JS_INC})

AUX_SOURCE_DIRECTORY(src/nanomsg SOURCE_FILES_NANOMSG)

ADD_LIBRARY(node_nanomsg SHARED ${SOURCE_FILES_NANOMSG})
SET_TARGET_PROPERTIES(node_nanomsg PROPERTIES PREFIX "" SUFFIX ".node")
TARGET_LINK_LIBRARIES(node_nanomsg nanomsg ${CMAKE_JS_LIB})
