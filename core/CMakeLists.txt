# Copyright [2019] [taurus.ai]
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(kungfu)

############################################################
SET(CMAKE_CXX_STANDARD 17)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
if(UNIX AND NOT APPLE)
    SET(KFC_INSTALL_RPATH
            "$ORIGIN"
            "$ORIGIN/../../"
            )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
endif()
if(APPLE)
    SET(KFC_INSTALL_RPATH
        "@executable_path/../Resources/kfc"
        "@loader_path"
        "@loader_path/../../"
    )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
    SET(CMAKE_MACOSX_RPATH ON)
endif()
if(WIN32)
    SET(CMAKE_GENERATOR_PLATFORM x64)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /w /permissive- /bigobj")
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
    ADD_COMPILE_DEFINITIONS(HAVE_SNPRINTF)
#    ADD_COMPILE_DEFINITIONS(_WIN32_WINNT=0x0A00) # Target Windows 10 https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
endif()

############################################################

# to corporate clion with cmake-js:
# https://github.com/cmake-js/cmake-js/issues/23

if(CLION)
    function(PARSE_CMAKEJS_PROPERTIES)
        function(GET_VARIABLE INPUT_STRING VARIABLE_TO_SELECT OUTPUT_VARIABLE)
            set(SEARCH_STRING "${VARIABLE_TO_SELECT}=\"")
            string(LENGTH "${SEARCH_STRING}" SEARCH_STRING_LENGTH)
            string(LENGTH "${INPUT_STRING}" INPUT_STRING_LENGTH)

            string(FIND "${INPUT_STRING}" "${VARIABLE_TO_SELECT}=\"" SEARCH_STRING_INDEX)

            math(EXPR SEARCH_STRING_INDEX "${SEARCH_STRING_INDEX}+${SEARCH_STRING_LENGTH}")

            string(SUBSTRING "${INPUT_STRING}" ${SEARCH_STRING_INDEX} ${INPUT_STRING_LENGTH} AFTER_SEARCH_STRING)
            string(FIND "${AFTER_SEARCH_STRING}" "\"" QUOTE_INDEX)
            string(SUBSTRING "${AFTER_SEARCH_STRING}" "0" "${QUOTE_INDEX}" RESULT_STRING)
            set("${OUTPUT_VARIABLE}" "${RESULT_STRING}" PARENT_SCOPE)
        endfunction(GET_VARIABLE)

        string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
        if (CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
            exec_program(./node_modules/.bin/cmake-js ${CMAKE_CURRENT_SOURCE_DIR}
                    ARGS --runtime ${NODE_RUNTIME} --runtime-version ${NODE_RUNTIMEVERSION} --arch ${NODE_ARCH} print-configure --debug
                    OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                    )
        else()
            exec_program(./node_modules/.bin/cmake-js ${CMAKE_CURRENT_SOURCE_DIR}
                    ARGS --runtime ${NODE_RUNTIME} --runtime-version ${NODE_RUNTIMEVERSION} --arch ${NODE_ARCH} print-configure
                    OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                    )
        endif ()

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_JS_INC" CMAKE_JS_INC)
        set(CMAKE_JS_INC "${CMAKE_JS_INC}" PARENT_SCOPE)

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_LIBRARY_OUTPUT_DIRECTORY" CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" PARENT_SCOPE)

        message("-- Clion ready")
    endfunction(PARSE_CMAKEJS_PROPERTIES)

    PARSE_CMAKEJS_PROPERTIES()
endif()

############################################################

ADD_COMPILE_DEFINITIONS(HAVE_USLEEP=1)

ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_SESSION)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_PREUPDATE_HOOK)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_JSON1)

ADD_COMPILE_DEFINITIONS(SQLITE_THREADSAFE=1)
ADD_COMPILE_DEFINITIONS(SQLITE_MAX_MMAP_SIZE=134217728)
ADD_COMPILE_DEFINITIONS(SQLITE_DEFAULT_MMAP_SIZE=1048576)

ADD_COMPILE_DEFINITIONS(SPDLOG_ACTIVE_LEVEL=${SPDLOG_LOG_LEVEL_COMPILE})
ADD_COMPILE_DEFINITIONS(SPDLOG_FMT_EXTERNAL)
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_NAME)
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_ATOMIC_LEVELS)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/deps/nanomsg-1.1.5/include)
INCLUDE_DIRECTORIES(deps/spdlog-1.3.1/include)
INCLUDE_DIRECTORIES(deps/json-3.5.0/single_include)
INCLUDE_DIRECTORIES(deps/SQLiteCpp-2.3.0/include)
INCLUDE_DIRECTORIES(deps/fmt-5.3.0/include)
INCLUDE_DIRECTORIES(deps/sqlite3-3.30.1)
INCLUDE_DIRECTORIES(deps/rxcpp-4.1.0/include)
INCLUDE_DIRECTORIES(deps/hana-1.6.0/include)
INCLUDE_DIRECTORIES(deps/hffix-b67d404f/include)
INCLUDE_DIRECTORIES(deps/googletest-1.9.0/include)

############################################################

ADD_SUBDIRECTORY(deps)
ADD_SUBDIRECTORY(cpp)

############################################################

SET(KF_EXTENSIONS_BUILD_DIR ${CMAKE_BINARY_DIR}/build_extensions)
ADD_SUBDIRECTORY(extensions)
