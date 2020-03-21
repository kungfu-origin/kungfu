# to corporate clion with cmake-js:
# https://github.com/cmake-js/cmake-js/issues/23
if(CLION)
    function(PARSE_CMAKEJS_PROPERTIES)
        function(GET_VARIABLE INPUT_STRING VARIABLE_TO_SELECT OUTPUT_VARIABLE)
            set(SEARCH_STRING "${VARIABLE_TO_SELECT}=\"")
            string(LENGTH "${SEARCH_STRING}" SEARCH_STRING_LENGTH)
            string(LENGTH "${INPUT_STRING}" INPUT_STRING_LENGTH)

            string(FIND "${INPUT_STRING}" "${VARIABLE_TO_SELECT}=\"" SEARCH_STRING_INDEX)

            math(EXPR SEARCH_STRING_INDEX "${SEARCH_STRING_INDEX}+${SEARCH_STRING_LENGTH}")

            string(SUBSTRING "${INPUT_STRING}" ${SEARCH_STRING_INDEX} ${INPUT_STRING_LENGTH} AFTER_SEARCH_STRING)
            string(FIND "${AFTER_SEARCH_STRING}" "\"" QUOTE_INDEX)
            string(SUBSTRING "${AFTER_SEARCH_STRING}" "0" "${QUOTE_INDEX}" RESULT_STRING)
            set("${OUTPUT_VARIABLE}" "${RESULT_STRING}" PARENT_SCOPE)
        endfunction(GET_VARIABLE)

        string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
        if (CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
            exec_program(./node_modules/.bin/cmake-js ${CMAKE_CURRENT_SOURCE_DIR}
                    ARGS --runtime ${NODE_RUNTIME} --runtime-version ${NODE_RUNTIMEVERSION} --arch ${NODE_ARCH} print-configure --debug
                    OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                    )
        else()
            exec_program(./node_modules/.bin/cmake-js ${CMAKE_CURRENT_SOURCE_DIR}
                    ARGS --runtime ${NODE_RUNTIME} --runtime-version ${NODE_RUNTIMEVERSION} --arch ${NODE_ARCH} print-configure
                    OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                    )
        endif ()

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_JS_INC" CMAKE_JS_INC)
        set(CMAKE_JS_INC "${CMAKE_JS_INC}" PARENT_SCOPE)

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_LIBRARY_OUTPUT_DIRECTORY" CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" PARENT_SCOPE)

        message("-- Clion ready")
    endfunction(PARSE_CMAKEJS_PROPERTIES)

    PARSE_CMAKEJS_PROPERTIES()
endif()