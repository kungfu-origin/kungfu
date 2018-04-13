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
PROJECT(wingchun)

############################################################

INCLUDE_DIRECTORIES(../longfist)
INCLUDE_DIRECTORIES(../longfist/api/ctp/${CTP_VERSION}/include)
INCLUDE_DIRECTORIES(../longfist/api/xtp/${XTP_VERSION}/include)

INCLUDE_DIRECTORIES(../yijinjing/journal)
INCLUDE_DIRECTORIES(../yijinjing/log)
INCLUDE_DIRECTORIES(../yijinjing/paged)
INCLUDE_DIRECTORIES(../yijinjing/utils)

INCLUDE_DIRECTORIES(base)

LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../longfist/api/ctp/${CTP_VERSION}/lib)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../longfist/api/xtp/${XTP_VERSION}/lib)
LINK_DIRECTORIES(../yijinjing)

ADD_SUBDIRECTORY(md)
ADD_SUBDIRECTORY(td)
ADD_SUBDIRECTORY(strategy)
ADD_SUBDIRECTORY(tool)
