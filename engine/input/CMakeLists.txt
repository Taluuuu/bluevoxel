cmake_minimum_required(VERSION 3.20)
project(h2o_input)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(INPUT_SOURCES
    src/input_module.cpp)

if(TNT_USE_GLFW)
    list(APPEND INPUT_SOURCES
        src/input_module_glfw.cpp)
endif()

add_library(h2o_input ${INPUT_SOURCES})

target_link_libraries(h2o_input
    PRIVATE
        core
        windowing
        glfw)

target_include_directories(h2o_input
    PUBLIC
        include/

    PRIVATE
        core
        windowing
        glfw)