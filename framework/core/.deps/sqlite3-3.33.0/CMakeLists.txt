project(sqlite3k)
add_library(${PROJECT_NAME} sqlite3.c shell.c)
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KUNGFU_BUILD_SQLITE_DIR})
