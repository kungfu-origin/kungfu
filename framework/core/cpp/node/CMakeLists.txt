PROJECT(node-addons)

if (WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    ADD_COMPILE_DEFINITIONS(V8_DEPRECATION_WARNINGS=1)
endif ()

if (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -w")
endif ()

############################################################
INCLUDE_DIRECTORIES(${CMAKE_JS_INC})

ADD_COMPILE_DEFINITIONS(NAPI_VERSION=3)
ADD_COMPILE_DEFINITIONS(NAPI_EXPERIMENTAL)

EXECUTE_PROCESS(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
STRING(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
STRING(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
INCLUDE_DIRECTORIES(${NODE_ADDON_API_DIR})

MESSAGE(STATUS "Include node-js headers from [${CMAKE_JS_INC}]")
MESSAGE(STATUS "Include node-addon-api headers from [${NODE_ADDON_API_DIR}]")

############################################################

AUX_SOURCE_DIRECTORY(src SOURCE_FILES_KUNGFU)

if (${NODE_RUNTIME} STREQUAL "node")
    SET(ADDON_NAME "kungfu_node")
    MESSAGE(STATUS "Building node bindings ${ADDON_NAME}")
endif ()

if (${NODE_RUNTIME} STREQUAL "electron")
    SET(ADDON_NAME "kungfu_electron")
    MESSAGE(STATUS "Building node bindings ${ADDON_NAME}")
endif ()

if (ADDON_NAME)
    ADD_LIBRARY(${ADDON_NAME} SHARED ${SOURCE_FILES_KUNGFU} ${CMAKE_JS_SRC})
    SET_TARGET_PROPERTIES(${ADDON_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
    TARGET_LINK_LIBRARIES(${ADDON_NAME} ${CMAKE_JS_LIB} kungfu pybind11::embed)
else ()
    MESSAGE(WARNING "Unknown node runtime ${NODE_RUNTIME}")
endif ()