PROJECT(kungfu-extensions)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/include)

ADD_SUBDIRECTORY(sim)
ADD_SUBDIRECTORY(xtp)
IF (NOT APPLE)
    ADD_SUBDIRECTORY(ctp)
ENDIF ()
