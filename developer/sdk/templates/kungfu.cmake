macro(kungfu_setup MODULE_NAME)
    set(CMAKE_CXX_STANDARD 17)

    if(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
        set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
        set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    endif()
    if(UNIX AND NOT APPLE)
        set(KFC_INSTALL_RPATH
                "$ORIGIN"
                "$ORIGIN/../../"
                )
        set(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
        set(KFC_DIR "/opt/Kungfu/resources/kfc")
    endif()
    if(APPLE)
        set(KFC_INSTALL_RPATH
                "@executable_path/../Resources/kfc"
                "@loader_path"
                "@loader_path/../../"
                )
        set(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
        set(CMAKE_MACOSX_RPATH ON)
        set(KFC_DIR "/Applications/Kungfu.app/Contents/Resources/kfc")
    endif()
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /w /permissive-")
        set(CMAKE_GENERATOR_PLATFORM x64)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
        set(KFC_DIR "C:\\Program Files\\Kungfu\\resources\\kfc")
        add_compile_definitions(HAVE_SNPRINTF)
    endif()

    add_compile_definitions(FMT_HEADER_ONLY)

    include_directories("${KFC_DIR}/include")
    <%_ includes.forEach(dir => { _%>
    include_directories("<%= dir %>")
    <%_ }); _%>

    link_directories("${KFC_DIR}")
    <%_ links.forEach((dir, i) => { _%>
    link_directories("<%= dir %>")
    <%_ }); _%>

    <%_ sources.forEach((dir, i) => { _%>
    aux_source_directory("<%- dir %>" SOURCE_<%- i %>)
    <%_ }); _%>
    set(SOURCES <%= extraSource %> <%= sources.map((dir, i) => '${SOURCE_' + i + '}').join(' ') %>)

    set(BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/target")

    add_subdirectory("${KFC_DIR}/pybind11" "${PROJECT_BINARY_DIR}/pybind11")

    <%- makeTarget %>(${MODULE_NAME} SHARED ${SOURCES})
    target_link_libraries(${MODULE_NAME} PRIVATE kungfu <%- targetLinks %>)
    set_target_properties(${MODULE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BUILD_OUTPUT_DIR})
    set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUTPUT_DIR})
    set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUTPUT_DIR})
endmacro()