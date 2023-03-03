cmake_minimum_required(VERSION 3.20)
project(h2o_input)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_input src/input_module.cpp)

target_link_libraries(h2o_input
    PRIVATE
        core
        windowing)

target_include_directories(h2o_input
    PUBLIC
        include/

    PRIVATE
        core
        windowing)