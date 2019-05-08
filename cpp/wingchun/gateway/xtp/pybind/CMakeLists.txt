PROJECT(pyxtp)
PYBIND11_ADD_MODULE(xtp SHARED "gateway_ext.cpp")
TARGET_LINK_LIBRARIES(xtp PRIVATE xtp_md xtp_td)

