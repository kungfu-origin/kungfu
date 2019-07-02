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
endif()
if(APPLE)
    SET(KFC_INSTALL_RPATH
        "@executable_path/../Resources/kfc"
        "@loader_path"
        "@loader_path/../../"
    )
    SET(CMAKE_MACOSX_RPATH ON)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
endif()
if(WIN32)
    # install boost using binaries from https://sourceforge.net/projects/boost/files/boost-binaries/1.64.0/
    SET(BOOST_ROOT "C:\\local\\boost_1_64_0")
#    ADD_COMPILE_DEFINITIONS(_WIN32_WINNT=0x0A00) # Target Windows 10 https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
    ADD_COMPILE_DEFINITIONS(BOOST_ALL_NO_LIB)
    ADD_COMPILE_DEFINITIONS(HAVE_SNPRINTF)
#    ADD_COMPILE_DEFINITIONS(BOOST_ALL_DYN_LINK)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /w")
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
endif()

#OPTION(test "Build all tests." ON) # Makes boolean 'test' available.
#ENABLE_TESTING()

############################################################

ADD_COMPILE_DEFINITIONS(SPDLOG_ACTIVE_LEVEL=${SPDLOG_LOG_LEVEL_COMPILE})
ADD_COMPILE_DEFINITIONS(SPDLOG_FMT_EXTERNAL)
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_NAME)
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_ATOMIC_LEVELS)

############################################################

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

############################################################

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/deps/nanomsg-1.1.5/include)
INCLUDE_DIRECTORIES(deps/spdlog-1.3.1/include)
INCLUDE_DIRECTORIES(deps/json-3.5.0/single_include)
INCLUDE_DIRECTORIES(deps/SQLiteCpp-2.3.0/include)
INCLUDE_DIRECTORIES(deps/fmt-5.3.0/include)
INCLUDE_DIRECTORIES(deps/rxcpp-4.1.0/include)
INCLUDE_DIRECTORIES(deps/hffix-b67d404f/include)
INCLUDE_DIRECTORIES(deps/googletest-1.9.0/include)

############################################################

ADD_SUBDIRECTORY(deps)
ADD_SUBDIRECTORY(cpp)

############################################################

SET(KF_EXTENSIONS_BUILD_DIR ${CMAKE_BINARY_DIR}/build_extensions)
ADD_SUBDIRECTORY(extensions)
