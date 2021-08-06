aux_source_directory(src/yijinjing/cache SOURCE_FILES_YJJ_CACHE)
aux_source_directory(src/yijinjing/journal SOURCE_FILES_YJJ_JOURNAL)
aux_source_directory(src/yijinjing/index SOURCE_FILES_YJJ_INDEX)
aux_source_directory(src/yijinjing/io SOURCE_FILES_YJJ_IO)
aux_source_directory(src/yijinjing/util SOURCE_FILES_YJJ_UTIL)
aux_source_directory(src/yijinjing/practice SOURCE_FILES_YJJ_PRACTICE)

aux_source_directory(src/wingchun/broker SOURCE_FILES_WC_BROKER)
aux_source_directory(src/wingchun/service SOURCE_FILES_WC_SERVICE)
aux_source_directory(src/wingchun/book SOURCE_FILES_WC_BOOK)
aux_source_directory(src/wingchun/strategy SOURCE_FILES_WC_STRATEGY)
aux_source_directory(src/wingchun/util SOURCE_FILES_WC_UTIL)

add_library(${PROJECT_NAME} SHARED
        ${SOURCE_FILES_YJJ_CACHE}
        ${SOURCE_FILES_YJJ_JOURNAL}
        ${SOURCE_FILES_YJJ_INDEX}
        ${SOURCE_FILES_YJJ_IO}
        ${SOURCE_FILES_YJJ_UTIL}
        ${SOURCE_FILES_YJJ_PRACTICE}
        ${SOURCE_FILES_WC_BROKER}
        ${SOURCE_FILES_WC_SERVICE}
        ${SOURCE_FILES_WC_BOOK}
        ${SOURCE_FILES_WC_STRATEGY}
        ${SOURCE_FILES_WC_UTIL})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} sqlite3 nng)
