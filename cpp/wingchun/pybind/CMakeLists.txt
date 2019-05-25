PROJECT(pywingchun)

############################################################
IF(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
ENDIF()

PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED "PyWingchun.cpp")
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ../event_loop)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE event_loop calendar strategy ${Boost_LIBRARIES})
