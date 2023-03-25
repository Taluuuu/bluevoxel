cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(INPUT_SOURCES
    src/input_module.cpp)

add_library(h2o_input ${INPUT_SOURCES})

target_link_libraries(h2o_input
    PRIVATE
        h2o_core
        h2o_windowing)

target_include_directories(h2o_input
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_windowing)