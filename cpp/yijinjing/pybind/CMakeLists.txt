PROJECT(pyyjj)

if(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif()

############################################################

INCLUDE_DIRECTORIES(../../longfist)

INCLUDE_DIRECTORIES(../journal)

PYBIND11_ADD_MODULE(${PROJECT_NAME} SHARED "pybind_yjj.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE yijinjing)
