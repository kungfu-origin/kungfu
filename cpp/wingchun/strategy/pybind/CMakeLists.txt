PROJECT(pystrategy)
PYBIND11_ADD_MODULE(pystrategy SHARED strategy_util_ext.cpp)
TARGET_LINK_LIBRARIES(pystrategy PRIVATE strategy_util)
