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
PROJECT(pywingchun)

############################################################
IF(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
ENDIF()

PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED "PyWingchun.cpp")
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ../event_loop)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE event_loop calendar strategy ${Boost_LIBRARIES})
