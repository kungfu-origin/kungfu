project(kungfu-node-addon)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif ()

############################################################

add_compile_definitions(NAPI_EXPERIMENTAL)
add_compile_definitions(NAPI_VERSION=7)

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

aux_source_directory(src SOURCE_FILES_KUNGFU)

if (${NODE_RUNTIME} STREQUAL "node")
    set(ADDON_NAME "kungfu_node")
    message(STATUS "Building node bindings ${ADDON_NAME}")
endif ()

if (${NODE_RUNTIME} STREQUAL "electron")
    set(ADDON_NAME "kungfu_electron")
    message(STATUS "Building node bindings ${ADDON_NAME}")
endif ()

if (ADDON_NAME)
    add_library(${ADDON_NAME} SHARED ${SOURCE_FILES_KUNGFU} ${CMAKE_JS_SRC})
    set_target_properties(${ADDON_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
    target_link_libraries(${ADDON_NAME} ${CMAKE_JS_LIB} kungfu pybind11::embed)
else ()
    message(WARNING "Unknown node runtime ${NODE_RUNTIME}")
endif ()
