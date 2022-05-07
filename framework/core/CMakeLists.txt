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

cmake_minimum_required(VERSION 3.15)
project(kungfu)

##########################################################################

include(.cmake/editor.cmake)
include(.cmake/compiler.cmake)
include(.cmake/libnode.cmake)
include(build/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS NO_OUTPUT_DIRS SKIP_STD SKIP_FPIC)

##########################################################################

add_compile_definitions(HAVE_USLEEP=1)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=${SPDLOG_LOG_LEVEL_COMPILE})
add_compile_definitions(SPDLOG_NO_NAME)
add_compile_definitions(SPDLOG_NO_ATOMIC_LEVELS)

add_compile_definitions(SQLITE_ENABLE_COLUMN_METADATA)
add_compile_definitions(SQLITE_ENABLE_JSON1)
add_compile_definitions(SQLITE_ENABLE_SESSION)
add_compile_definitions(SQLITE_ENABLE_PREUPDATE_HOOK)
add_compile_definitions(SQLITE_ENABLE_DBSTAT_VTAB)

add_compile_definitions(SQLITE_MAX_MMAP_SIZE=134217728)
add_compile_definitions(SQLITE_DEFAULT_MMAP_SIZE=1048576)

##########################################################################

option(NNG_TESTS "" OFF)
option(NNG_TOOLS "" OFF)
option(NNG_ENABLE_NNGCAT "" OFF)

##########################################################################

include_directories(src/include)

##########################################################################

# set for libkungfu reference
set(KUNGFU_BUILD_DIR ${CMAKE_BINARY_DIR})

##########################################################################
add_subdirectory(.deps/pybind11-2.9.0)
add_subdirectory(.deps/sqlite3-3.33.0)
add_subdirectory(src/libkungfu)
add_subdirectory(src/bindings/python)
add_subdirectory(src/bindings/node)
