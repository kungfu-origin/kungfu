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
PROJECT(wingchunstrategy)

SET(STRATEGY_SOURCE_FILES WCDataWrapper.cpp WCDataWrapper.h IWCStrategy.h IWCStrategy.cpp WCStrategyUtil.h WCStrategyUtil.cpp PyWCStrategy.cpp PyWCStrategy.h)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${STRATEGY_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} journal kflog python ${Boost_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/wingchun")

INSTALL(FILES ../base/WC_DECLARE.h IWCDataProcessor.h IWCStrategy.h WCDataWrapper.h WCStrategyUtil.h DESTINATION "include")