PROJECT(pywingchun)

############################################################
IF(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
ENDIF()

PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED "pybind_wingchun.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE wingchun)
