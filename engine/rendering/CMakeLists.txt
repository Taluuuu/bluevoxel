cmake_minimum_required(VERSION 3.18)
project(rendering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(rendering
    src/renderer.cpp
)

target_link_libraries(rendering
    PRIVATE

)

target_include_directories(rendering
    PUBLIC
        include/
)