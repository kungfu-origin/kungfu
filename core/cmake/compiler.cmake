SET(CMAKE_CXX_STANDARD 17)

if (UNIX)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O0") # set -fPIC for nng
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_CXX_FLAGS_DEBUG "-g")
    SET(CMAKE_CXX_FLAGS_RELEASE "")

    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif ()
if (UNIX AND NOT APPLE)
    SET(KFC_INSTALL_RPATH
            "$ORIGIN"
            "$ORIGIN/../../"
            )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
endif ()
if (APPLE)
    SET(KFC_INSTALL_RPATH
            "@loader_path"
            "@loader_path/../../"
            "@executable_path/../../../../Resources/kfc"
            #@executable_path = "Kungfu.app/Contents/Frameworks/Kungfu Helper.app/Contents/MacOS"
            )
    SET(CMAKE_INSTALL_RPATH "${KFC_INSTALL_RPATH}")
    SET(CMAKE_MACOSX_RPATH ON)
endif ()
if (WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /permissive- /bigobj /MP /W0")
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4199")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4199")
    ADD_COMPILE_DEFINITIONS(HAVE_SNPRINTF)
    ADD_COMPILE_DEFINITIONS(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0 -Wno-address-of-packed-member -Wno-deprecated")
endif ()