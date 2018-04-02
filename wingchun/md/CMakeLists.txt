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
PROJECT(wingchunmd)

SET(CTP_SOURCE_FILES MDEngineCTP.cpp MDEngineCTP.h IMDEngine.cpp IMDEngine.h ../base/IEngine.cpp ../base/IEngine.h)
ADD_LIBRARY(ctpmd SHARED ${CTP_SOURCE_FILES})
TARGET_LINK_LIBRARIES(ctpmd journal kflog thostmduserapi ${Boost_LIBRARIES})
INSTALL(TARGETS ctpmd DESTINATION "lib/wingchun")

SET(XTP_SOURCE_FILES MDEngineXTP.cpp MDEngineXTP.h IMDEngine.cpp IMDEngine.h ../base/IEngine.cpp ../base/IEngine.h)
ADD_LIBRARY(xtpmd SHARED ${XTP_SOURCE_FILES})
TARGET_LINK_LIBRARIES(xtpmd journal kflog xtpquoteapi ${Boost_LIBRARIES})
INSTALL(TARGETS xtpmd DESTINATION "lib/wingchun")