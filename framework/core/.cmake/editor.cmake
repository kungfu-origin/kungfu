# to corporate clion with cmake-js:
# https://github.com/cmake-js/cmake-js/issues/23
if (CLION)
    function(parse_cmakejs_properties)
        function(get_variable INPUT_STRING VARIABLE_TO_SELECT OUTPUT_VARIABLE)
            set(SEARCH_STRING "${VARIABLE_TO_SELECT}=")
            string(LENGTH "${SEARCH_STRING}" SEARCH_STRING_LENGTH)
            string(LENGTH "${INPUT_STRING}" INPUT_STRING_LENGTH)

            string(FIND "${INPUT_STRING}" "${VARIABLE_TO_SELECT}=" SEARCH_STRING_INDEX)

            math(EXPR SEARCH_STRING_INDEX "${SEARCH_STRING_INDEX}+${SEARCH_STRING_LENGTH}")

            string(SUBSTRING "${INPUT_STRING}" ${SEARCH_STRING_INDEX} ${INPUT_STRING_LENGTH} AFTER_SEARCH_STRING)
            string(FIND "${AFTER_SEARCH_STRING}" "'" QUOTE_INDEX)
            string(SUBSTRING "${AFTER_SEARCH_STRING}" "0" "${QUOTE_INDEX}" RESULT_STRING)
            set("${OUTPUT_VARIABLE}" "${RESULT_STRING}" PARENT_SCOPE)
        endfunction(get_variable)

        string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
        if (CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
            set(DEBUG_OPTION "--debug")
        else ()
            set(DEBUG_OPTION "")
        endif ()

        execute_process(
                COMMAND yarn cmake-js --runtime ${NODE_RUNTIME} --runtime-version ${NODE_VERSION} --arch ${NODE_ARCH} print-configure ${DEBUG_OPTION}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE CMAKE_JS_OUTPUT
                )

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_JS_INC" CMAKE_JS_INC)
        set(CMAKE_JS_INC "${CMAKE_JS_INC}" PARENT_SCOPE)

        get_variable("${CMAKE_JS_OUTPUT}" "CMAKE_LIBRARY_OUTPUT_DIRECTORY" CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" PARENT_SCOPE)

        if (CMAKE_JS_INC STREQUAL "")
            message(FATAL_ERROR "Failed to get CMAKE_JS_INC")
        endif ()

        message(STATUS "Clion ready")
    endfunction(parse_cmakejs_properties)

    parse_cmakejs_properties()
endif ()
