cmake_minimum_required(VERSION 3.9)
project(Coordinator)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(Coordinator main.cpp gt_coord.h)