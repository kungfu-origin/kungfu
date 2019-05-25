PROJECT(kungfu-cpp)

############################################################

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/cpp/deps/nanomsg-1.1.5/include)
INCLUDE_DIRECTORIES(deps/spdlog-1.3.1/include)
INCLUDE_DIRECTORIES(deps/json-3.5.0/single_include)
INCLUDE_DIRECTORIES(deps/SQLiteCpp-2.3.0/include)
INCLUDE_DIRECTORIES(deps/fmt-5.3.0/include)
INCLUDE_DIRECTORIES(deps/fast-cpp-csv-parser)
INCLUDE_DIRECTORIES(deps/getopt-1.0.0)
INCLUDE_DIRECTORIES(deps/cxxopts-2.2/include)
INCLUDE_DIRECTORIES(deps/googletest-1.9.0/include)

############################################################

ADD_SUBDIRECTORY(deps)

############################################################

ADD_SUBDIRECTORY(yijinjing)
ADD_SUBDIRECTORY(wingchun)
