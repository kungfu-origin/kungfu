project(${LIBKUNGFU_NAME})

file(GLOB_RECURSE WINGCHUN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/wingchun/*.cpp)
file(GLOB_RECURSE YIJINJING_CACHE_SOURCE_FILES ${PROJECT_SOURCE_DIR}/yijinjing/cache/*.cpp)
file(GLOB_RECURSE YIJINJING_OTHER_SOURCE_FILES ${PROJECT_SOURCE_DIR}/yijinjing/*.cpp)
list(REMOVE_ITEM YIJINJING_OTHER_SOURCE_FILES ${YIJINJING_CACHE_SOURCE_FILES})

## set the target compile options variable to replace some of options set by .cmake/compiler.cmake
if (UNIX)
    set(COMPILE_DEBUG_OPTIONS "-g;-O0")
    set(COMPILE_RELEASE_OPTIONS "-O3")
    set(COMPILE_RELEASE_BYPASS_OPTIONS "-O0")
endif ()

if (MSVC)
    set(COMPILE_DEBUG_OPTIONS "/O0")
    set(COMPILE_RELEASE_OPTIONS "/O2")
    set(COMPILE_RELEASE_BYPASS_OPTIONS "/O2")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # enable msvc to generate kungfu.lib
endif ()

add_library(yijinjing_cache OBJECT ${YIJINJING_CACHE_SOURCE_FILES})
set_target_properties(yijinjing_cache PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(yijinjing_cache  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_DIR})
target_compile_options(yijinjing_cache PRIVATE "$<$<CONFIG:Debug>:${COMPILE_DEBUG_OPTIONS}>")
target_compile_options(yijinjing_cache PRIVATE "$<$<CONFIG:Release>:${COMPILE_RELEASE_BYPASS_OPTIONS}>")

add_library(yijinjing_other OBJECT ${YIJINJING_OTHER_SOURCE_FILES})
set_target_properties(yijinjing_other PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(yijinjing_other PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_DIR})
target_compile_options(yijinjing_other PRIVATE "$<$<CONFIG:Debug>:${COMPILE_DEBUG_OPTIONS}>")
target_compile_options(yijinjing_other PRIVATE "$<$<CONFIG:Release>:${COMPILE_RELEASE_OPTIONS}>")

add_library(wingchun OBJECT ${WINGCHUN_SOURCE_FILES})
set_target_properties(wingchun PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(wingchun PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_DIR})
target_compile_options(wingchun PRIVATE "$<$<CONFIG:Debug>:${COMPILE_DEBUG_OPTIONS}>")
target_compile_options(wingchun PRIVATE "$<$<CONFIG:Release>:${COMPILE_RELEASE_OPTIONS}>")

add_library(${LIBKUNGFU_NAME} SHARED $<TARGET_OBJECTS:wingchun> $<TARGET_OBJECTS:yijinjing_cache> $<TARGET_OBJECTS:yijinjing_other>)
set_target_properties(${LIBKUNGFU_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${LIBKUNGFU_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_DIR})
target_link_libraries(${LIBKUNGFU_NAME} ${CONAN_LIBS})
target_compile_options(${LIBKUNGFU_NAME} PRIVATE "$<$<CONFIG:Debug>:${COMPILE_DEBUG_OPTIONS}>")
target_compile_options(${LIBKUNGFU_NAME} PRIVATE "$<$<CONFIG:Release>:${COMPILE_RELEASE_OPTIONS}>")
