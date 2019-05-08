PROJECT(calendar)

SET(CALENDAR_SOURCE_FILES src/calendar_service.cpp src/calendar_service_daemon.cpp src/calendar.cpp src/calendar_storage.hpp)
ADD_LIBRARY(calendar SHARED ${CALENDAR_SOURCE_FILES})

if (WIN32)
    TARGET_LINK_LIBRARIES(calendar SQLiteCpp sqlite3 nanomsg fmt journal ${BOOST_LIBRARY})
endif()
if (UNIX AND NOT APPLE)
    TARGET_LINK_LIBRARIES(calendar SQLiteCpp sqlite3 pthread dl nanomsg fmt journal ${BOOST_LIBRARY})
endif()
if (APPLE)
    TARGET_LINK_LIBRARIES(calendar SQLiteCpp sqlite3 pthread dl nanomsg fmt journal ${BOOST_LIBRARY} iconv)
endif()

ADD_EXECUTABLE(calendar_service_daemon src/calendar_service_daemon.cpp)
TARGET_LINK_LIBRARIES(calendar_service_daemon calendar)

ADD_SUBDIRECTORY(pybind)
