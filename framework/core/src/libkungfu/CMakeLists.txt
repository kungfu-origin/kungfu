project(libkungfu)
file(GLOB_RECURSE SOURCE_FILES PROJECT_SOURCE_DIR *.cpp)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # enable msvc to generate kungfu.lib
set(LIB_NAME kungfu)
add_library(${LIB_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${LIB_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_DIR})
target_link_libraries(${LIB_NAME} sqlite3k ${CONAN_LIBS})
