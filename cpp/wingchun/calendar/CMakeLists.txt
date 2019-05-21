PROJECT(calendar)

SET(CALENDAR_SOURCE_FILES src/calendar_service.cpp src/calendar_storage.hpp src/calendar.cpp)
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
