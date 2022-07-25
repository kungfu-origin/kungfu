cmake_minimum_required(VERSION 3.15)
project(xtp)
include(build/kungfu.cmake)

include(node_modules/@kungfu-trader/barrich-linux-deps/dist/include)
kungfu_setup(xtp)