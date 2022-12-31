cmake_minimum_required(VERSION 3.20)
project(windowing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(windowing 
    src/window_glfw.cpp
    src/windowing_module.cpp
)

target_link_libraries(windowing
    PRIVATE
        core

        glfw
)

target_include_directories(windowing
    PUBLIC
        include/

    PRIVATE
        core

        glfw
)