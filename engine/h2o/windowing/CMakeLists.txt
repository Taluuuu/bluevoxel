cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_windowing STATIC
    src/window_glfw.cpp
    src/windowing_module.cpp
)

target_link_libraries(h2o_windowing
    PRIVATE
        h2o_core

        glfw
)

target_include_directories(h2o_windowing
    PUBLIC
        include/

    PRIVATE
        h2o_core

        glfw
)