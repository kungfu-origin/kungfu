PROJECT(kfext_btdata)

ADD_CUSTOM_COMMAND(OUTPUT kfext_btdata
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/btdata ${KF_EXTENSIONS_BUILD_DIR}/btdata)
ADD_CUSTOM_TARGET(btdata ALL DEPENDS kfext_btdata)
