macro(kungfu_setup MODULE_NAME)
  include(<%- kfcDir -%>/cmake/compiler.cmake)

  add_compile_definitions(KUNGFU_MODULE_NAME=${MODULE_NAME})
  add_compile_definitions(FMT_HEADER_ONLY)
  add_compile_definitions(SPDLOG_ACTIVE_LEVEL=0)
  add_compile_definitions(SPDLOG_NO_NAME)
  add_compile_definitions(SPDLOG_NO_ATOMIC_LEVELS)
  add_compile_definitions(SPDLOG_FMT_EXTERNAL)

  message(STATUS "CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}")

  if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    set(CMAKE_CXX_FLAGS_RELEASE "-O")
  endif ()

  include_directories("<%- kfcDir -%>/include")
  <%_ includes.forEach(dir => { _%>
  include_directories("<%= dir %>")
  <%_ }); _%>

  link_directories("<%- kfcDir -%>")
  <%_ links.forEach((dir, i) => { _%>
  link_directories("<%= dir %>")
  <%_ }); _%>

  <%_ sources.forEach((dir, i) => { _%>
  aux_source_directory("<%- dir %>" SOURCE_<%- i %>)
  <%_ }); _%>
  set(SOURCES <%= extraSource %> <%= sources.map((dir, i) => '${SOURCE_' + i + '}').join(' ') %>)

  set(BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/target")

  SET(KFC_EXECUTABLE "<%- kfcExec %>")

  string(REPLACE "\n" "" KFC_EXECUTABLE ${KFC_EXECUTABLE})
  string(REPLACE "\"" "" KFC_EXECUTABLE ${KFC_EXECUTABLE})

  set(PYTHON_EXECUTABLE ${KFC_EXECUTABLE})
  set(ENV{KFC_AS_VARIANT} python)

  add_subdirectory("<%- kfcDir -%>/pybind11" "${PROJECT_BINARY_DIR}/pybind11")

  <%- makeTarget %>(${MODULE_NAME} <%- makeTargetLinkType %> ${SOURCES})
  target_link_libraries(${MODULE_NAME} PRIVATE kungfu <%- targetLinks %>)
  set_target_properties(${MODULE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BUILD_OUTPUT_DIR})
  set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUTPUT_DIR})
  set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUTPUT_DIR})
endmacro()