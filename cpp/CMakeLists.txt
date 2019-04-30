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

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(kungfu-cpp)

############################################################

INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/cpp/deps/nanomsg-1.1.5/include")
INCLUDE_DIRECTORIES(deps/spdlog-1.3.1/include)
INCLUDE_DIRECTORIES(deps/json-3.5.0/single_include)
INCLUDE_DIRECTORIES(deps/SQLiteCpp-2.3.0/include)
INCLUDE_DIRECTORIES(deps/fmt-5.3.0/include)
INCLUDE_DIRECTORIES(deps/fast-cpp-csv-parser)
INCLUDE_DIRECTORIES(deps/getopt-1.0.0)
INCLUDE_DIRECTORIES(deps/cxxopts-2.2/include)
INCLUDE_DIRECTORIES(deps/googletest-1.9.0/include)
INCLUDE_DIRECTORIES(deps/cppnanomsg)

############################################################

ADD_SUBDIRECTORY(deps)
ADD_SUBDIRECTORY(yijinjing)
ADD_SUBDIRECTORY(wingchun)