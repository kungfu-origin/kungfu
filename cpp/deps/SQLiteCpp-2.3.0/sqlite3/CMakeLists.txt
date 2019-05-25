# CMake file for compiling the sqlite3 static library under Windows (for ease of use)
#
# Copyright (c) 2012-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT) 

if(WIN32)
    # add sources of the "sqlite3" static library
    # because windows needs extern int (e.g. SQLite::OPEN_READWRITE) to be exported to dll
    add_library(sqlite3 sqlite3.c sqlite3.h)
else()
    # add sources of the "sqlite3" shared library
    # because python will copy system sqlite3.so to kfc folder with will cause mutex SIGSEGV issue
    add_library(sqlite3 SHARED sqlite3.c sqlite3.h)
    set_target_properties(sqlite3 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqlite3)
    set_target_properties(sqlite3 PROPERTIES SOVERSION 0)
endif(WIN32)

if (UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
    set_target_properties(SQLiteCpp PROPERTIES COMPILE_FLAGS "-fPIC")
endif (UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
