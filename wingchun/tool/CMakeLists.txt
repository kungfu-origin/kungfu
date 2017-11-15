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
PROJECT(wingchuntool)

ADD_LIBRARY(${PROJECT_NAME} SHARED DataConsumer.cpp DataConsumer.h StrategyDataConsumer.cpp StrategyDataConsumer.h)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} journal kflog python ${Boost_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/wingchun")

ADD_EXECUTABLE(wc_printer wc_strategy_data_printer.cpp)
TARGET_LINK_LIBRARIES(wc_printer ${PROJECT_NAME} journal python ${Boost_LIBRARIES})
INSTALL(TARGETS wc_printer DESTINATION "bin")

ADD_EXECUTABLE(td_pos_tool td_pos_tool.cpp ../base/TDUserStruct.cpp ../base/TDUserStruct.h)
TARGET_LINK_LIBRARIES(td_pos_tool journal python ${Boost_LIBRARIES})
INSTALL(TARGETS td_pos_tool DESTINATION "bin")
