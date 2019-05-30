# Main CMake file for compiling the library itself, examples and tests.
#
# Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)
project(SQLiteCpp)

# Define useful variables to handle OS differences:
if (WIN32)
    set(DEV_NULL                "NUL")
else (WIN32) # UNIX
    set(DEV_NULL                "/dev/null")
endif (WIN32)

# then Compiler/IDE differences:
if (MSVC)
    set(CPPLINT_ARG_OUTPUT      "--output=vs7")
    set(CPPCHECK_ARG_TEMPLATE   "--template=vs")
    # disable Visual Studio warnings for fopen() used in the example
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Flags for linking with multithread static C++ runtime, required by googletest
    if (SQLITECPP_BUILD_TESTS)
        message(STATUS "Linking against multithread static C++ runtime for unit tests with googletest")
        set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    endif (SQLITECPP_BUILD_TESTS)
    # Handle the (partly supported) MSVC versions prior to 2015
    if (MSVC_VERSION LESS 1900) # OR MSVC_TOOLSET_VERSION LESS 140)
      message(WARNING "MSVC < 2015 detected: Visual Studio prior to 2015 is not fully supported. BLOB storage seems to be corrupted.")
    endif (MSVC_VERSION LESS 1900)
else (MSVC)
    set(CPPLINT_ARG_OUTPUT      "--output=eclipse")
    set(CPPCHECK_ARG_TEMPLATE   "--template=gcc")
    # Useful compile flags and extra warnings
    add_compile_options(-fstack-protector -Wall -Wextra -Wpedantic -Wno-long-long -Wswitch-enum -Wshadow -Winline)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++0x-compat") # C++ only
    if (CMAKE_COMPILER_IS_GNUCXX)
        # GCC flags
        if (SQLITECPP_USE_GCOV AND CMAKE_COMPILER_IS_GNUCXX)
            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                message (STATUS "Using GCov instrumentation")
            else ()
                message (WARNING "GCov instrumentation works best in Debug mode")
            endif ()
            add_compile_options (-coverage) # NOTE would be usefull but not working with current google test and gcc 4.8 -fkeep-inline-functions
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage")
        endif ()
    endif (CMAKE_COMPILER_IS_GNUCXX)
endif (MSVC)
# and then common variables
set(CPPLINT_ARG_VERBOSE "--verbose=3")
set(CPPLINT_ARG_LINELENGTH "--linelength=120")

# Print CXX compiler information
message (STATUS "CMAKE_CXX_COMPILER '${CMAKE_CXX_COMPILER}' '${CMAKE_CXX_COMPILER_ID}' '${CMAKE_CXX_COMPILER_VERSION}'")

# Print CXX FLAGS
message (STATUS "CMAKE_CXX_FLAGS                '${CMAKE_CXX_FLAGS}'")
if (MSVC)
    message (STATUS "CMAKE_CXX_FLAGS_DEBUG          '${CMAKE_CXX_FLAGS_DEBUG}'")
    message (STATUS "CMAKE_CXX_FLAGS_RELEASE        '${CMAKE_CXX_FLAGS_RELEASE}'")
    message (STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO '${CMAKE_CXX_FLAGS_RELWITHDEBINFO}'")
    message (STATUS "CMAKE_CXX_FLAGS_MINSIZEREL     '${CMAKE_CXX_FLAGS_MINSIZEREL}'")
else (NOT MSVC)
    if     (CMAKE_BUILD_TYPE STREQUAL Debug)
        message (STATUS "CMAKE_CXX_FLAGS_DEBUG          '${CMAKE_CXX_FLAGS_DEBUG}'")
    elseif (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
        message (STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO '${CMAKE_CXX_FLAGS_RELWITHDEBINFO}'")
    elseif (CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
        message (STATUS "CMAKE_CXX_FLAGS_MINSIZEREL     '${CMAKE_CXX_FLAGS_MINSIZEREL}'")
    else ()
        message (STATUS "CMAKE_CXX_FLAGS_RELEASE        '${CMAKE_CXX_FLAGS_RELEASE}'")
    endif ()
endif ()

# Options relative to SQLite and SQLiteC++ functions

option(SQLITE_ENABLE_COLUMN_METADATA "Enable Column::getColumnOriginName(). Require support from sqlite3 library." ON)
if (SQLITE_ENABLE_COLUMN_METADATA)
    # Enable the use of SQLite column metadata and Column::getColumnOriginName() method,
    # Require that the sqlite3 library is also compiled with this flag (default under Debian/Ubuntu, but not on Mac OS X).
    add_definitions(-DSQLITE_ENABLE_COLUMN_METADATA)
endif (SQLITE_ENABLE_COLUMN_METADATA)

option(SQLITE_ENABLE_ASSERT_HANDLER "Enable the user defintion of a assertion_failed() handler." OFF)
if (SQLITE_ENABLE_ASSERT_HANDLER)
    # Enable the user defintion of a assertion_failed() handler (default to false, easier to handler for begginers).
    add_definitions(-DSQLITECPP_ENABLE_ASSERT_HANDLER)
endif (SQLITE_ENABLE_ASSERT_HANDLER)

option(SQLITE_USE_LEGACY_STRUCT "Fallback to forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)" OFF)
if (SQLITE_USE_LEGACY_STRUCT)
    # Force forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)
    add_definitions(-DSQLITE_USE_LEGACY_STRUCT)
endif (SQLITE_USE_LEGACY_STRUCT)


## Build the C++ Wrapper ##

# adding a new file require explicittly modifing the CMakeLists.txt
# so that CMake knows that it should rebuild the project (it is best practice)

# list of sources files of the library
set(SQLITECPP_SRC
 ${PROJECT_SOURCE_DIR}/src/Backup.cpp
 ${PROJECT_SOURCE_DIR}/src/Column.cpp
 ${PROJECT_SOURCE_DIR}/src/Database.cpp
 ${PROJECT_SOURCE_DIR}/src/Exception.cpp
 ${PROJECT_SOURCE_DIR}/src/Statement.cpp
 ${PROJECT_SOURCE_DIR}/src/Transaction.cpp
)
source_group(src FILES ${SQLITECPP_SRC})

# list of header files of the library
set(SQLITECPP_INC
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/SQLiteCpp.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Assertion.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Backup.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Column.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Database.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Exception.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Statement.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Transaction.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/Utils.h
 ${PROJECT_SOURCE_DIR}/include/SQLiteCpp/VariadicBind.h
)
source_group(include FILES ${SQLITECPP_INC})

# list of test files of the library
set(SQLITECPP_TESTS
 tests/Column_test.cpp
 tests/Database_test.cpp
 tests/Statement_test.cpp
 tests/Backup_test.cpp
 tests/Transaction_test.cpp
 tests/VariadicBind_test.cpp
 tests/Exception_test.cpp
)
source_group(tests FILES ${SQLITECPP_TESTS})

# list of example files of the library
set(SQLITECPP_EXAMPLES
 examples/example1/main.cpp
)
source_group(example1 FILES ${SQLITECPP_EXAMPLES})

# list of doc files of the library
set(SQLITECPP_DOC
 README.md
 LICENSE.txt
 CHANGELOG.md
 TODO.txt
)
source_group(doc FILES ${SQLITECPP_DOC})

# list of script files of the library
set(SQLITECPP_SCRIPT
 .travis.yml
 appveyor.yml
 build.bat
 build.sh
 cpplint.py
 Doxyfile
 FindSQLiteCpp.cmake
)
source_group(scripts FILES ${SQLITECPP_SCRIPT})

# All includes are relative to the "include" directory
include_directories("${PROJECT_SOURCE_DIR}/include")

# add sources of the wrapper as a "SQLiteCpp" static library
if(UNIX)
    add_library(SQLiteCpp SHARED ${SQLITECPP_SRC} ${SQLITECPP_INC} ${SQLITECPP_DOC} ${SQLITECPP_SCRIPT})
endif()
if(WIN32)
    add_library(SQLiteCpp ${SQLITECPP_SRC} ${SQLITECPP_INC} ${SQLITECPP_DOC} ${SQLITECPP_SCRIPT})
endif()

# make the sqlite3 library part of the interface of the SQLiteCpp wrapper itself (the client app does not need to link to sqlite3)
# PR https://github.com/SRombauts/SQLiteCpp/pull/111 "linked SQLiteCpp to sqlite3" commented out since it breaks install step from PR #118
target_link_libraries(SQLiteCpp PUBLIC sqlite3)

if (UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
    set_target_properties(SQLiteCpp PROPERTIES COMPILE_FLAGS "-fPIC")
endif (UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))

## Build provided copy of SQLite3 C library ##

# TODO 
#find_package(sqlite3)
#if(sqlite3_VERSION VERSION_LESS "3.19")
#    set_target_properties(SQLiteCpp PROPERTIES COMPILE_FLAGS "-DSQLITECPP_HAS_MEM_STRUCT")
#endif()

option(SQLITECPP_INTERNAL_SQLITE "Add the internal SQLite3 source to the project." ON)
if (SQLITECPP_INTERNAL_SQLITE)
    message("-- SQLITECPP_INTERNAL_SQLITE ON")
    # build the SQLite3 C library (for ease of use/compatibility) versus Linux sqlite3-dev package
    add_subdirectory(sqlite3)
    target_include_directories(sqlite3 PUBLIC "${PROJECT_SOURCE_DIR}/sqlite3")
    target_include_directories(SQLiteCpp PRIVATE "${PROJECT_SOURCE_DIR}/sqlite3")
else()
    message("-- SQLITECPP_INTERNAL_SQLITE OFF")
endif (SQLITECPP_INTERNAL_SQLITE)

# Optional additional targets:

option(SQLITECPP_RUN_CPPLINT "Run cpplint.py tool for Google C++ StyleGuide." OFF)
if (SQLITECPP_RUN_CPPLINT)
    find_package(PythonInterp)
    if (PYTHONINTERP_FOUND)
        # add a cpplint target to the "all" target
        add_custom_target(SQLiteCpp_cpplint
         ALL
         COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/cpplint.py ${CPPLINT_ARG_OUTPUT} ${CPPLINT_ARG_VERBOSE} ${CPPLINT_ARG_LINELENGTH} ${SQLITECPP_SRC} ${SQLITECPP_INC}
        )
    endif (PYTHONINTERP_FOUND)
else (SQLITECPP_RUN_CPPLINT)
    message(STATUS "SQLITECPP_RUN_CPPLINT OFF")
endif (SQLITECPP_RUN_CPPLINT)

option(SQLITECPP_RUN_CPPCHECK "Run cppcheck C++ static analysis tool." ON)
if (SQLITECPP_RUN_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
    if (CPPCHECK_EXECUTABLE)
        # add a cppcheck target to the "all" target
        add_custom_target(SQLiteCpp_cppcheck
         ALL
         COMMAND ${CPPCHECK_EXECUTABLE} -j 8 cppcheck --enable=style --quiet ${CPPCHECK_ARG_TEMPLATE} ${PROJECT_SOURCE_DIR}/src
        )
        execute_process(COMMAND "${CPPCHECK_EXECUTABLE}" --version OUTPUT_VARIABLE CPPCHECK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Found Cppcheck: ${CPPCHECK_EXECUTABLE} ${CPPCHECK_VERSION}")
    else (CPPCHECK_EXECUTABLE)
        message(STATUS "Could NOT find cppcheck")
    endif (CPPCHECK_EXECUTABLE)
else (SQLITECPP_RUN_CPPCHECK)
    message(STATUS "SQLITECPP_RUN_CPPCHECK OFF")
endif (SQLITECPP_RUN_CPPCHECK)

option(SQLITECPP_RUN_DOXYGEN "Run Doxygen C++ documentation tool." OFF)
if (SQLITECPP_RUN_DOXYGEN)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # add a Doxygen target to the "all" target
        add_custom_target(SQLiteCpp_doxygen
         ALL
         COMMAND doxygen Doxyfile > ${DEV_NULL}
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    endif (DOXYGEN_FOUND)
else (SQLITECPP_RUN_DOXYGEN)
    message(STATUS "SQLITECPP_RUN_DOXYGEN OFF")
endif (SQLITECPP_RUN_DOXYGEN)

option(SQLITECPP_BUILD_EXAMPLES "Build examples." OFF)
if (SQLITECPP_BUILD_EXAMPLES)
    # add the basic example executable
    add_executable(SQLiteCpp_example1 ${SQLITECPP_EXAMPLES})
    target_link_libraries(SQLiteCpp_example1 SQLiteCpp sqlite3)
    # Link target with pthread and dl for linux
    if (UNIX)
        target_link_libraries(SQLiteCpp_example1 pthread)
        if (NOT APPLE)
            target_link_libraries(SQLiteCpp_example1 dl)
        endif ()
    elseif (MSYS OR MINGW)
        target_link_libraries(SQLiteCpp_example1 ssp)
    endif ()
else (SQLITECPP_BUILD_EXAMPLES)
    message(STATUS "SQLITECPP_BUILD_EXAMPLES OFF")
endif (SQLITECPP_BUILD_EXAMPLES)

option(SQLITECPP_BUILD_TESTS "Build and run tests." OFF)
if (SQLITECPP_BUILD_TESTS)
    # deactivate some warnings for compiling the gtest library
    if (NOT MSVC)
        add_compile_options(-Wno-variadic-macros -Wno-long-long -Wno-switch-enum -Wno-float-equal -Wno-conversion-null -Wno-switch-default -Wno-pedantic)
    endif (NOT MSVC)

    # add the subdirectory containing the CMakeLists.txt for the gtest library
    # TODO: under Linux, uses libgtest-dev if found
    if (NOT EXISTS "${PROJECT_SOURCE_DIR}/googletest/CMakeLists.txt")
        message(FATAL_ERROR "Missing 'googletest' submodule! Either use 'git submodule init' and 'git submodule update' to get googletest according to the README, or deactivate unit tests with -DSQLITECPP_BUILD_TESTS=OFF")
    endif ()
    add_subdirectory(googletest)
    include_directories("${PROJECT_SOURCE_DIR}/googletest/googletest/include")

    # Add definitions to keep googletest from making the compilation fail
    if (MSVC)
        if (MSVC AND MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS_EQUAL 1919) # OR MSVC_TOOLSET_VERSION EQUAL 141)
            target_compile_definitions(gtest PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
            target_compile_definitions(gtest_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
            target_compile_definitions(gmock PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
            target_compile_definitions(gmock_main PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
        endif (MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS_EQUAL 1919)
    endif (MSVC)

    # add the unit test executable
    add_executable(SQLiteCpp_tests ${SQLITECPP_TESTS})
    target_link_libraries(SQLiteCpp_tests gtest_main SQLiteCpp sqlite3)
    # Link target with dl for linux
    if (UNIX AND NOT APPLE)
        target_link_libraries(SQLiteCpp_tests dl)
    endif ()

    # add a "test" target:
    enable_testing()

    # does the tests pass?
    add_test(UnitTests SQLiteCpp_tests)

    if (SQLITECPP_BUILD_EXAMPLES)
        # does the example1 runs successfully?
        add_test(Example1Run SQLiteCpp_example1)
    endif (SQLITECPP_BUILD_EXAMPLES)
else (SQLITECPP_BUILD_TESTS)
    message(STATUS "SQLITECPP_BUILD_TESTS OFF")
endif (SQLITECPP_BUILD_TESTS)

