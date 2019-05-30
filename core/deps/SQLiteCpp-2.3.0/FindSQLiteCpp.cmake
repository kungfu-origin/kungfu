# @file    CMakeLists.txt
# @ingroup SQLiteCpp
# @brief   SQLiteCpp CMake module.
#
# Copyright (c) 2010-2014 Kartik Kumar (me@kartikkumar.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

macro(_sqlitecpp_check_version)
  file(READ "${SQLITECPP_INCLUDE_DIR}/SQLiteCpp.h" _sqlitecpp_header)

  string(REGEX MATCH "define[ \t]+SQLITECPP_VERSION_NUMBER[ \t]+([0-9]+)" 
         _sqlitecpp_version_match "${_sqlitecpp_header}")
  set(SQLITECPP_VERSION "${CMAKE_MATCH_1}")
  if(${SQLITECPP_VERSION} VERSION_LESS ${SQLiteCpp_FIND_VERSION})
    set(SQLITECPP_VERSION_OK FALSE)
  else(${SQLITECPP_VERSION} VERSION_LESS ${SQLiteCpp_FIND_VERSION})
    set(SQLITECPP_VERSION_OK TRUE)
  endif(${SQLITECPP_VERSION} VERSION_LESS ${SQLiteCpp_FIND_VERSION})

  if(NOT SQLITECPP_VERSION_OK)
    message(STATUS "SQLiteCpp version ${SQLITECPP_VERSION} found in ${SQLITECPP_INCLUDE_DIR}, "
                   "but at least version ${SQLiteCpp_FIND_VERSION} is required!")
  endif(NOT SQLITECPP_VERSION_OK)

  set(SQLITECPP_LIBRARY "SQLiteCpp")
  link_directories(${SQLITECPP_LIBRARY_DIR})
endmacro(_sqlitecpp_check_version)

if(SQLITECPP_INCLUDE_DIR)
  # Check if SQLiteCpp is already present in cache.
  _sqlitecpp_check_version()
  set(SQLITECPP_FOUND ${SQLITECPP_VERSION_OK})

else (SQLITECPP_INCLUDE_DIR)
  find_path(SQLITECPP_BASE_PATH NAMES SQLiteCpp.h
      PATHS
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/..
      ${PROJECT_SOURCE_DIR}/../..
      ${PROJECT_SOURCE_DIR}/../../..
      ${PROJECT_SOURCE_DIR}/../../../..      
      PATH_SUFFIXES SQLiteCpp/include/SQLiteCpp
    )
  set(SQLITECPP_INCLUDE_DIR ${SQLITECPP_BASE_PATH})
  set(SQLITECPP_LIBRARY_DIR ${SQLITECPP_BASE_PATH}/../../build)

  if(SQLITECPP_INCLUDE_DIR)
    _sqlitecpp_check_version()
  endif(SQLITECPP_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLITECPP DEFAULT_MSG SQLITECPP_INCLUDE_DIR SQLITECPP_VERSION_OK)

  mark_as_advanced(SQLITECPP_INCLUDE_DIR)

endif(SQLITECPP_INCLUDE_DIR)
