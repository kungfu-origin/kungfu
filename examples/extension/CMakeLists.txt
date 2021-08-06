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
PROJECT(kfextdemo)

############################################################

# Setup KungFu SDK
if (KF_SDK_HOME)
    MESSAGE("KF_SDK_HOME set to ${KF_SDK_HOME}")
else (KF_SDK_HOME)
    GET_FILENAME_COMPONENT(KF_SDK_HOME "../../core/build/kfc" ABSOLUTE)
    MESSAGE("KF_SDK_HOME set to ${KF_SDK_HOME}")
endif (KF_SDK_HOME)

INCLUDE("${KF_SDK_HOME}/cmake/sdk.cmake")

############################################################

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE kungfu)

CONFIGURE_FILE(__init__.py "${PROJECT_BINARY_DIR}/dist/__init__.py")
