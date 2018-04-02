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
PROJECT(journal)

IF(USE_SERVICE)
    ADD_DEFINITIONS(-DUSE_PAGED_SERVICE)
ENDIF(USE_SERVICE)

IF(USE_HASH_CODE)
    ADD_DEFINITIONS(-DFRAME_AUTO_SET_HASHCODE)
ENDIF(USE_HASH_CODE)

SET(LIB_UTIL_INCLUDE_FILES ../utils/Timer.h ../utils/Hash.hpp ../utils/TypeConvert.hpp ../utils/json.hpp ../utils/PosHandler.hpp ../utils/FeeHandler.hpp)
SET(LIB_UTIL_SOURCE_FILES ../utils/Timer.cpp)
SET(LIB_INCLUDE_FILES ../utils/constants.h ../utils/YJJ_DECLARE.h Frame.hpp FrameHeader.h Journal.h JournalHandler.h
        JournalReader.h JournalWriter.h Page.h PageUtil.h PageHeader.h PageProvider.h IPageProvider.h
        StrategySocketHandler.h StrategyUtil.h IJournalVisitor.h IStrategyUtil.h JournalFinder.h)
SET(LIB_SOURCE_FILES Journal.cpp JournalHandler.cpp JournalReader.cpp JournalWriter.cpp Page.cpp PageUtil.cpp
        PythonExtend.cpp PageProvider.cpp StrategyUtil.cpp JournalFinder.cpp)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES} ${LIB_INCLUDE_FILES} ${LIB_UTIL_SOURCE_FILES} ${LIB_UTIL_INCLUDE_FILES} )

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 1.1 VERSION 1.1)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} python ${Boost_LIBRARIES})

INSTALL(FILES ${LIB_UTIL_INCLUDE_FILES} ${LIB_INCLUDE_FILES} DESTINATION "include")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/yijinjing")