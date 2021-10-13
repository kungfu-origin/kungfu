project(kungfu-node-bind)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif ()

############################################################

execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

message(STATUS "Include node-addon-api headers from [${NODE_ADDON_API_DIR}]")
message(STATUS "Include node-js headers from [${CMAKE_JS_INC}]")

include_directories(${NODE_ADDON_API_DIR})
include_directories(${CMAKE_JS_INC})

############################################################

aux_source_directory(binding SOURCE_FILES_KUNGFU)

if (${NODE_RUNTIME} STREQUAL "node")
    set(KF_NODE_BINDING_NAME "kungfu_node")
    message(STATUS "Building node bindings ${KF_NODE_BINDING_NAME}")
endif ()

if (${NODE_RUNTIME} STREQUAL "electron")
    set(KF_NODE_BINDING_NAME "kungfu_electron")
    message(STATUS "Building node bindings ${KF_NODE_BINDING_NAME}")
endif ()

if (KF_NODE_BINDING_NAME)
    add_library(${KF_NODE_BINDING_NAME} SHARED ${SOURCE_FILES_KUNGFU})
    set_target_properties(${KF_NODE_BINDING_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
    target_link_libraries(${KF_NODE_BINDING_NAME} ${CMAKE_JS_LIB} kungfu)
else ()
    message(WARNING "Unknown node runtime ${NODE_RUNTIME}")
endif ()
