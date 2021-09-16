project(kungfu-python-bind)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
endif ()

############################################################

aux_source_directory(binding SOURCE_FILES)

set(KF_PYTHON_BINDING_NAME pykungfu)
pybind11_add_module(${KF_PYTHON_BINDING_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${KF_PYTHON_BINDING_NAME} PRIVATE kungfu node)
