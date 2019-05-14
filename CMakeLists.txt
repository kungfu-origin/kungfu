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

ADD_COMPILE_DEFINITIONS(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

SET(CMAKE_CXX_STANDARD 14)

if(UNIX AND NOT APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    OPTION(BUILD_SHARED_LIBS "turn OFF for .a libs" ON)
endif()
if(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
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
#    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /VERBOSE")
endif()

OPTION(test "Build all tests." ON) # Makes boolean 'test' available.
ENABLE_TESTING()

############################################################

FILE(GLOB_RECURSE boostlib_cmake_path "${CMAKE_CURRENT_SOURCE_DIR}/node_modules" "BoostLib.cmake")
LIST(GET boostlib_cmake_path 0 boostlib_cmake_path)
GET_FILENAME_COMPONENT(boostlib_cmake_path "${boostlib_cmake_path}" DIRECTORY)
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${boostlib_cmake_path}")
INCLUDE(BoostLib)

REQUIRE_BOOST_LIBS("1.64.0" "locale;date_time;filesystem;system;thread;serialization;math_tr1;chrono;regex;program_options")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

############################################################
ADD_SUBDIRECTORY(cpp)

