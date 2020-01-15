PROJECT(node_addons)

if(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    ADD_COMPILE_DEFINITIONS(V8_DEPRECATION_WARNINGS=1)
endif()

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -w")
endif()

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

MESSAGE("-- Include node-js headers from ${CMAKE_JS_INC}")
MESSAGE("-- Include node-addon-api headers from ${NODE_ADDON_API_DIR}")

############################################################

AUX_SOURCE_DIRECTORY(src/yjj SOURCE_FILES_NANOMSG)
ADD_LIBRARY(node_yjj SHARED ${SOURCE_FILES_NANOMSG})
SET_TARGET_PROPERTIES(node_yjj PROPERTIES PREFIX "" SUFFIX ".node")
TARGET_LINK_LIBRARIES(node_yjj yijinjing ${CMAKE_JS_LIB})

############################################################

AUX_SOURCE_DIRECTORY(src/nanomsg SOURCE_FILES_NANOMSG)
ADD_LIBRARY(node_nanomsg SHARED ${SOURCE_FILES_NANOMSG})
SET_TARGET_PROPERTIES(node_nanomsg PROPERTIES PREFIX "" SUFFIX ".node")
TARGET_LINK_LIBRARIES(node_nanomsg nanomsg ${CMAKE_JS_LIB})

############################################################

AUX_SOURCE_DIRECTORY(src/sqlite3 SOURCE_FILES_SQLITE3)
ADD_LIBRARY(node_sqlite3 SHARED ${SOURCE_FILES_SQLITE3})
SET_TARGET_PROPERTIES(node_sqlite3 PROPERTIES PREFIX "" SUFFIX ".node")
TARGET_LINK_LIBRARIES(node_sqlite3 sqlite3 ${CMAKE_JS_LIB})

############################################################

if(WIN32)
    AUX_SOURCE_DIRECTORY(src/windows-kill SOURCE_FILES_WINDOWS_KILL)
    ADD_LIBRARY(node_windows_kill SHARED ${SOURCE_FILES_WINDOWS_KILL})
    SET_TARGET_PROPERTIES(node_windows_kill PROPERTIES PREFIX "" SUFFIX ".node")
    TARGET_LINK_LIBRARIES(node_windows_kill Dbghelp ${CMAKE_JS_LIB})
endif()