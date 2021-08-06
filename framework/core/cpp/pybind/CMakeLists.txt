project(pykungfu)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif ()

aux_source_directory(src SOURCE_FILES)

pybind11_add_module(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE kungfu)

if (${NODE_RUNTIME} STREQUAL "node")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})
endif ()
