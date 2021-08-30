project(kungfu-python-bind)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif ()

############################################################

add_compile_definitions(NAPI_EXPERIMENTAL)
add_compile_definitions(NAPI_VERSION=7)

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

message(STATUS "LIBNODE_INCLUDE_DIR set to ${LIBNODE_INCLUDE_DIR}")
message(STATUS "LIBNODE_LIB_DIR set to ${LIBNODE_LIB_DIR}")

include_directories(${LIBNODE_INCLUDE_DIR})
link_directories(${LIBNODE_LIB_DIR})

############################################################

aux_source_directory(binding SOURCE_FILES)

set(KF_PYTHON_BINDING_NAME pykungfu)
pybind11_add_module(${KF_PYTHON_BINDING_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${KF_PYTHON_BINDING_NAME} PRIVATE kungfu node)
