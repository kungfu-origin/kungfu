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
PROJECT(kfext_bar)

IF(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
ENDIF()

SET(KF_EXTENSION_BUILD_DIR ${KF_EXTENSIONS_BUILD_DIR}/bar)

CONFIGURE_FILE(__init__.py "${KF_EXTENSION_BUILD_DIR}/__init__.py")
CONFIGURE_FILE(package.json "${KF_EXTENSION_BUILD_DIR}/package.json")

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED ${SOURCE_FILES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${KF_EXTENSION_BUILD_DIR})
IF(WIN32)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KF_EXTENSION_BUILD_DIR})
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${KF_EXTENSION_BUILD_DIR})
ENDIF()
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE wingchun yijinjing)

ADD_CUSTOM_COMMAND(OUTPUT package_json
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/package.json ${KF_EXTENSION_BUILD_DIR})
ADD_CUSTOM_TARGET(kfext_bar_package_json ALL DEPENDS package_json)
