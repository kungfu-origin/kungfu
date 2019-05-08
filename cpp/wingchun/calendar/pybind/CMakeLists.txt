PROJECT(pycalendar)
PYBIND11_ADD_MODULE(pycalendar SHARED "service_ext.cpp")
TARGET_LINK_LIBRARIES(pycalendar PRIVATE calendar)