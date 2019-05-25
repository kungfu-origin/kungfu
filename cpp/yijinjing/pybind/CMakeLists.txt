PROJECT(pyyjj)

############################################################

INCLUDE_DIRECTORIES(../../longfist)

INCLUDE_DIRECTORIES(../journal)

PYBIND11_ADD_MODULE(${PROJECT_NAME} "pybind_yjj.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE journal ${Boost_LIBRARIES})
