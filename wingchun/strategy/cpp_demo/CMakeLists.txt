# Copyright [2017] [taurus.ai]
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(kungfu_strategy_demo)

############################################################
SET(TOOLCHAIN_DIR "/opt/kungfu/toolchain")

SET(BOOST_INCLUDEDIR "${TOOLCHAIN_DIR}/boost-1.62.0/include")
SET(BOOST_LIBRARYDIR "${TOOLCHAIN_DIR}/boost-1.62.0/lib")

SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_STATIC_RUNTIME OFF)

FIND_PACKAGE(Boost 1.62.0 COMPONENTS date_time filesystem system thread serialization math_tr1 python chrono regex program_options unit_test_framework  locale REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDEDIR})

FIND_PACKAGE(PythonInterp)
FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
OPTION(BUILD_SHARED_LIBS "turn OFF for .a libs" ON)

############################################################
INCLUDE_DIRECTORIES(/opt/kungfu/master/include)

LINK_DIRECTORIES(/opt/kungfu/master/lib/yijinjing)
LINK_DIRECTORIES(/opt/kungfu/master/lib/wingchun)

############################################################
SET(STRATEGY_SOURCE_FILE kungfu_strategy_demo.cpp)
ADD_EXECUTABLE(${PROJECT_NAME} ${STRATEGY_SOURCE_FILE})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} wingchunstrategy python journal kflog log4cplus ${Boost_LIBRARIES})