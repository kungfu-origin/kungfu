PROJECT(kungfu-extensions)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/spdlog-1.3.1/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/json-3.5.0/single_include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/fmt-5.3.0/include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/cpp/deps/nanomsg-1.1.5/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/SQLiteCpp-2.3.0/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/fast-cpp-csv-parser)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/getopt-1.0.0)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/cxxopts-2.2/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/deps/googletest-1.9.0/include)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/yijinjing/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cpp/wingchun/include)

############################################################

ADD_SUBDIRECTORY(xtp)
ADD_SUBDIRECTORY(ctp)
