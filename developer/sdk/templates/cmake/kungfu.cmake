macro(kungfu_setup MODULE_NAME)
  include(<%- kfcDir -%>/cmake/compiler.cmake)
  
  add_compile_definitions(FMT_HEADER_ONLY)

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

  execute_process(
    COMMAND
    node -p "require('@kungfu-trader/kungfu-core').executable"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE KFC_EXECUTABLE
  )
  string(REPLACE "\n" "" KFC_EXECUTABLE ${KFC_EXECUTABLE})
  string(REPLACE "\"" "" KFC_EXECUTABLE ${KFC_EXECUTABLE})

  set(PYTHON_EXECUTABLE ${KFC_EXECUTABLE})
  set(ENV{KFC_AS_VARIANT} python)

  add_subdirectory("<%- kfcDir -%>/pybind11" "${PROJECT_BINARY_DIR}/pybind11")

  <%- makeTarget %>(${MODULE_NAME} SHARED ${SOURCES})
  target_link_libraries(${MODULE_NAME} PRIVATE kungfu <%- targetLinks %>)
  set_target_properties(${MODULE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BUILD_OUTPUT_DIR})
  set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUTPUT_DIR})
  set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUTPUT_DIR})
endmacro()