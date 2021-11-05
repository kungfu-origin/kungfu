add_compile_definitions(NAPI_EXPERIMENTAL)
add_compile_definitions(NAPI_VERSION=8)

execute_process(COMMAND node -p "require('@kungfu-trader/libnode').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE LIBNODE_INCLUDE_DIR
        )
string(REPLACE "\n" "" LIBNODE_INCLUDE_DIR ${LIBNODE_INCLUDE_DIR})
string(REPLACE "\"" "" LIBNODE_INCLUDE_DIR ${LIBNODE_INCLUDE_DIR})

execute_process(COMMAND node -p "require('@kungfu-trader/libnode').libpath"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE LIBNODE_LIB_DIR
        )
string(REPLACE "\n" "" LIBNODE_LIB_DIR ${LIBNODE_LIB_DIR})
string(REPLACE "\"" "" LIBNODE_LIB_DIR ${LIBNODE_LIB_DIR})

message(STATUS "Include libnode headers from ${LIBNODE_INCLUDE_DIR}")

include_directories(${LIBNODE_INCLUDE_DIR})
link_directories(${LIBNODE_LIB_DIR})

if (WIN32)
    set(LIBNODE "libnode")
else()
    set(LIBNODE "node")
endif()