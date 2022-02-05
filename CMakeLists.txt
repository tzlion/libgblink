cmake_minimum_required(VERSION 3.21)
project(gblink)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DGBLINK_EXPORTS)

add_library(gblink SHARED libgblink.cpp)
