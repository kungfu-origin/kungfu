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

SET(LIB_UTIL_INCLUDE_FILES Timer.h Hash.hpp TypeConvert.hpp)
SET(LIB_UTIL_SOURCE_FILES Timer.cpp)
SET(LIB_INCLUDE_FILES constants.h YJJ_DECLARE.h Frame.hpp FrameHeader.h Journal.h JournalHandler.h
        JournalReader.h JournalWriter.h Page.h PageUtil.h PageHeader.h PageProvider.h IPageProvider.h
        IJournalVisitor.h Log.h PageService.h)
SET(LIB_SOURCE_FILES Journal.cpp JournalHandler.cpp JournalReader.cpp JournalWriter.cpp Page.cpp PageUtil.cpp
        PageProvider.cpp PageService.cpp)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES} ${LIB_INCLUDE_FILES} ${LIB_UTIL_SOURCE_FILES} ${LIB_UTIL_INCLUDE_FILES} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 1.1 VERSION 1.1)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
if(UNIX AND NOT APPLE)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
if (APPLE)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES} iconv)
endif()
