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
PROJECT(kfextdemo)

############################################################
SET(CMAKE_CXX_STANDARD 14)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif(UNIX)
if(APPLE)
    SET(CMAKE_MACOSX_RPATH ON)
endif(APPLE)
if(WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /w")
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
endif(WIN32)

############################################################

FILE(GLOB_RECURSE boostlib_cmake_path "${CMAKE_CURRENT_SOURCE_DIR}/node_modules" "BoostLib.cmake")
LIST(GET boostlib_cmake_path 0 boostlib_cmake_path)
GET_FILENAME_COMPONENT(boostlib_cmake_path "${boostlib_cmake_path}" DIRECTORY)
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${boostlib_cmake_path}")
INCLUDE(BoostLib)

REQUIRE_BOOST_LIBS("1.64.0" "locale;date_time;filesystem;system;thread;serialization;math_tr1;chrono;regex;program_options")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

############################################################

if (KF_SDK_HOME)
    MESSAGE("KF_SDK_HOME set to ${KF_SDK_HOME}")
else()
    SET(KF_SDK_HOME "../../build/kfc")
    MESSAGE("KF_SDK_HOME set to ${KF_SDK_HOME}")
endif (KF_SDK_HOME)

INCLUDE_DIRECTORIES(${KF_SDK_HOME}/include)
LINK_DIRECTORIES(${KF_SDK_HOME})

ADD_SUBDIRECTORY(../../cpp/deps/pybind11-2.2.4 build/pybind11)

############################################################

SET(KF_EXTENSION_BUILD_DIR ${CMAKE_BINARY_DIR}/dist)

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED ${SOURCE_FILES})

#TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE yijinjing wingchun)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${KF_EXTENSION_BUILD_DIR})

CONFIGURE_FILE(__init__.py "${KF_EXTENSION_BUILD_DIR}/__init__.py")
