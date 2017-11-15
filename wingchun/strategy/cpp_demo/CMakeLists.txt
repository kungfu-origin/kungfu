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

SET(STRATEGY_SOURCE_FILE kungfu_strategy_demo.cpp)
ADD_EXECUTABLE(${PROJECT_NAME} ${STRATEGY_SOURCE_FILE})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} wingchunstrategy python journal kflog log4cplus ${Boost_LIBRARIES})