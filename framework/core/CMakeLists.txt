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

CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(kungfu)

############################################################

include(.cmake/compiler.cmake)
include(.cmake/clion.cmake)
include(build/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS NO_OUTPUT_DIRS SKIP_STD SKIP_FPIC)

############################################################

ADD_COMPILE_DEFINITIONS(HAVE_USLEEP=1)

ADD_COMPILE_DEFINITIONS(SPDLOG_ACTIVE_LEVEL=${SPDLOG_LOG_LEVEL_COMPILE})
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_NAME)
ADD_COMPILE_DEFINITIONS(SPDLOG_NO_ATOMIC_LEVELS)

ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_COLUMN_METADATA)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_JSON1)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_SESSION)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_PREUPDATE_HOOK)
ADD_COMPILE_DEFINITIONS(SQLITE_ENABLE_DBSTAT_VTAB)

ADD_COMPILE_DEFINITIONS(SQLITE_MAX_MMAP_SIZE=134217728)
ADD_COMPILE_DEFINITIONS(SQLITE_DEFAULT_MMAP_SIZE=1048576)

############################################################

OPTION(NNG_TESTS "" OFF)
OPTION(NNG_TOOLS "" OFF)
OPTION(NNG_ENABLE_NNGCAT "" OFF)

############################################################

INCLUDE_DIRECTORIES(.deps/hana-1.7.0/include)
INCLUDE_DIRECTORIES(.deps/json-3.9.1/include)
INCLUDE_DIRECTORIES(.deps/sqlite3-3.33.0/include)
INCLUDE_DIRECTORIES(.deps/sqlite_orm-1.6/include)
INCLUDE_DIRECTORIES(.deps/nng-1.5.1/include)
INCLUDE_DIRECTORIES(.deps/tabulate-1.0/include)
INCLUDE_DIRECTORIES(cpp/include)

############################################################

ADD_SUBDIRECTORY(.deps)
ADD_SUBDIRECTORY(cpp)
