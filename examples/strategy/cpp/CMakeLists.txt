# Copyright [www.kungfu-trader.com]
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(demo)

############################################################

IF(APPLE)
    SET(KF_SDK_HOME "/Applications/Kungfu.app/Contents/Resources/kfc")
ENDIF()
IF(UNIX AND NOT APPLE)
    SET(KF_SDK_HOME "/opt/Kungfu/resources/kfc")
ENDIF()
IF (WIN32)
    SET(KF_SDK_HOME "C:\\Program Files\\Kungfu\\resources\\kfc")
ENDIF()
MESSAGE("KF_SDK_HOME set to ${KF_SDK_HOME}")

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
            "${KF_SDK_HOME}"
            )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
endif()
if(APPLE)
    SET(KFC_INSTALL_RPATH
            "@executable_path/../Resources/kfc"
            "@loader_path"
            "@loader_path/../../"
            "${KF_SDK_HOME}"
            )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
    SET(CMAKE_MACOSX_RPATH ON)
endif()

if(WIN32)
    SET(CMAKE_GENERATOR_PLATFORM x64)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /w /permissive-")
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
    ADD_COMPILE_DEFINITIONS(HAVE_SNPRINTF)
    ADD_COMPILE_DEFINITIONS(FMT_HEADER_ONLY)
endif()
############################################################

INCLUDE_DIRECTORIES("${KF_SDK_HOME}/include")
LINK_DIRECTORIES("${KF_SDK_HOME}")

#INCLUDE("${KF_SDK_HOME}/cmake/sdk.cmake")
#ADD_SUBDIRECTORY("${KF_SDK_HOME}/pybind11" "${PROJECT_BINARY_DIR}/pybind11")

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} kungfu)
